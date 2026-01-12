/**
 * @file HierarchyBuilder.cpp
 * @brief HierarchyBuilder 구현
 *
 * @note Phase 4.5: ModelViewerApp에서 분리
 */
#include "pch.h"
#include "Framework/Assets/HierarchyBuilder.h"

 // Framework
#include "Framework/Assets/MeshAsset.h"
#include "Framework/Assets/ModelLoader.h"
#include "Framework/Resources/ResourceManager.h"

// Core
#include "Core/Logging/LogMacros.h"

// ECS
#include "ECS/Components/HierarchyComponent.h"
#include "ECS/Components/MaterialComponent.h"
#include "ECS/Components/MeshComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Registry.h"
#include "ECS/SystemManager.h"
#include "ECS/Systems/TransformSystem.h"

namespace Framework
{
	HierarchyBuildResult HierarchyBuilder::Build(
		const LoadedModelData& modelData,
		const std::vector<ResourceId>& materialIds,
		ECS::Registry& registry,
		ECS::SystemManager& systemManager,
		ResourceManager& resourceManager,
		const HierarchyBuildOptions& options
	)
	{
		HierarchyBuildResult result;
		result.Clear();

		if (modelData.nodes.empty())
		{
			LOG_ERROR("[HierarchyBuilder] Model has no nodes");
			return result;
		}

		LOG_INFO("[HierarchyBuilder] Building hierarchy from %zu nodes", modelData.nodes.size());

		// TransformSystem 가져오기
		auto* transformSystem = systemManager.GetSystem<ECS::TransformSystem>();
		if (!transformSystem)
		{
			LOG_ERROR("[HierarchyBuilder] TransformSystem not found");
			return result;
		}

		// 1단계: 노드 인덱스 → Entity 매핑 테이블
		std::vector<ECS::Entity> nodeToEntity(modelData.nodes.size());

		// 2단계: 모든 노드에 대해 Entity 생성
		for (Core::size_t i = 0; i < modelData.nodes.size(); ++i)
		{
			const auto& node = modelData.nodes[i];

			// Entity 생성
			ECS::Entity entity = registry.CreateEntity();
			nodeToEntity[i] = entity;
			result.allEntities.push_back(entity);

			// Transform 분해
			DecomposedTransform trs = ModelLoader::DecomposeMatrix(node.localTransform);

			// 비균등 스케일 경고
			if (trs.hasNonUniformScale)
			{
				LOG_WARN(
					"[HierarchyBuilder] Node '%s' has non-uniform scale (%.2f, %.2f, %.2f)",
					node.name.c_str(), trs.scale.x, trs.scale.y, trs.scale.z
				);
			}

			// TransformComponent 추가
			ECS::TransformComponent transform;
			transform.position = trs.position;
			transform.rotation = trs.rotation;
			transform.scale = trs.scale;
			transform.eulerHint = trs.rotation.ToEuler();
			registry.AddComponent(entity, transform);

			// HierarchyComponent 추가
			ECS::HierarchyComponent hierarchy;
			registry.AddComponent(entity, hierarchy);

			LOG_DEBUG(
				"[HierarchyBuilder] Node[%zu] '%s' -> Entity %u (meshes: %zu)",
				i, node.name.c_str(), entity.id, node.meshIndices.size()
			);
		}

		// 3단계: 부모-자식 관계 설정
		ECS::Entity rootEntity = ECS::Entity::Invalid();

		for (Core::size_t i = 0; i < modelData.nodes.size(); ++i)
		{
			const auto& node = modelData.nodes[i];
			ECS::Entity entity = nodeToEntity[i];

			if (node.parentIndex >= 0 &&
				node.parentIndex < static_cast<Core::int32>(nodeToEntity.size()))
			{
				// 부모가 있는 경우
				ECS::Entity parentEntity = nodeToEntity[node.parentIndex];
				transformSystem->SetParent(entity, parentEntity);
			}
			else
			{
				// 루트 노드 (parentIndex == -1)
				transformSystem->SetParent(entity, ECS::Entity::Invalid());

				// 첫 번째 루트를 메인 루트로 설정
				if (!rootEntity.IsValid())
				{
					rootEntity = entity;
				}
			}
		}

		result.rootEntity = rootEntity;

		// 4단계: 루트 위치/스케일 오프셋 적용
		if (rootEntity.IsValid())
		{
			if (options.rootPosition != Math::Vector3::Zero())
			{
				transformSystem->SetPosition(rootEntity, options.rootPosition);
			}

			Math::Vector3 defaultScale = { 1.0f, 1.0f, 1.0f };
			if (options.rootScale.x != defaultScale.x ||
				options.rootScale.y != defaultScale.y ||
				options.rootScale.z != defaultScale.z)
			{
				transformSystem->SetScale(rootEntity, options.rootScale);
			}
		}

		// 5단계: 메시/머티리얼 연결
		for (Core::size_t i = 0; i < modelData.nodes.size(); ++i)
		{
			const auto& node = modelData.nodes[i];
			ECS::Entity entity = nodeToEntity[i];

			if (node.meshIndices.empty())
			{
				// 피벗 노드 - 메시 없음
				continue;
			}

			// 메시 생성 (소유권은 AssetManager로 이전)
			std::string meshName = modelData.name + "_" + node.name + "_Mesh";

			ResourceId meshId = CreateMeshFromNodeIndices(
				node.meshIndices,
				modelData,
				meshName,
				resourceManager,
				options.releaseSourceDataAfterUpload
			);

			if (!meshId.IsValid())
			{
				LOG_WARN("[HierarchyBuilder] Failed to create mesh for node '%s'", node.name.c_str());
				continue;
			}

			// MeshComponent 추가
			ECS::MeshComponent meshComp;
			meshComp.meshId = meshId;
			registry.AddComponent(entity, meshComp);

			// MaterialComponent 추가
			ECS::MaterialComponent matComp;
			for (Core::size_t j = 0;
				j < node.meshIndices.size() && j < ECS::MaterialComponent::MAX_MATERIALS;
				++j)
			{
				Core::uint32 meshIdx = node.meshIndices[j];
				if (meshIdx < modelData.meshes.size())
				{
					Core::int32 matIdx = modelData.meshes[meshIdx].materialIndex;
					if (matIdx >= 0 && matIdx < static_cast<Core::int32>(materialIds.size()))
					{
						ECS::MaterialHelpers::SetMaterial(
							matComp,
							static_cast<Core::uint32>(j),
							materialIds[matIdx]
						);
					}
				}
			}
			registry.AddComponent(entity, matComp);

			// 렌더링 대상으로 등록
			result.renderableEntities.push_back(entity);
		}

		LOG_INFO(
			"[HierarchyBuilder] Created %zu entities (%zu renderable) from '%s'",
			result.GetTotalCount(),
			result.GetRenderableCount(),
			modelData.name.c_str()
		);

		return result;
	}

	ResourceId HierarchyBuilder::CreateMeshFromNodeIndices(
		const std::vector<Core::uint32>& meshIndices,
		const LoadedModelData& modelData,
		const std::string& meshName,
		ResourceManager& resourceManager,
		bool releaseAfterUpload
	)
	{
		if (meshIndices.empty())
		{
			return ResourceId::Invalid();
		}

		// 단일 메시 최적화
		if (meshIndices.size() == 1)
		{
			Core::uint32 meshIdx = meshIndices[0];
			if (meshIdx >= modelData.meshes.size())
			{
				LOG_ERROR("[HierarchyBuilder] Mesh index %u out of range", meshIdx);
				return ResourceId::Invalid();
			}

			const auto& meshData = modelData.meshes[meshIdx];

			auto meshAsset = std::make_unique<MeshAsset>();

			// 데이터 복사 (const이므로 move 불가)
			std::vector<Graphics::StandardVertex> vertices = meshData.vertices;
			std::vector<Core::uint32> indices = meshData.indices;
			std::vector<Graphics::SubmeshInfo> submeshes = meshData.submeshes;

			meshAsset->SetVertices(std::move(vertices));
			meshAsset->SetIndices(std::move(indices));
			meshAsset->SetSubmeshes(std::move(submeshes));
			meshAsset->SetAABB(meshData.aabbMin, meshData.aabbMax);

			if (releaseAfterUpload)
			{
				meshAsset->SetDataPolicy(MeshDataPolicy::ReleaseAfterUpload);
			}
			else
			{
				meshAsset->SetDataPolicy(MeshDataPolicy::KeepSourceData);
			}

			return resourceManager.CreateMeshFromAsset(meshName, std::move(meshAsset));
		}

		// 멀티 메시 병합
		std::vector<const LoadedMeshData*> meshPtrs;
		meshPtrs.reserve(meshIndices.size());

		for (Core::uint32 meshIdx : meshIndices)
		{
			if (meshIdx < modelData.meshes.size())
			{
				meshPtrs.push_back(&modelData.meshes[meshIdx]);
			}
			else
			{
				LOG_WARN("[HierarchyBuilder] Mesh index %u out of range, skipping", meshIdx);
			}
		}

		if (meshPtrs.empty())
		{
			return ResourceId::Invalid();
		}

		// MeshAsset::MergeFromMeshData 사용
		auto meshAsset = MeshAsset::MergeFromMeshData(meshPtrs);
		if (!meshAsset)
		{
			LOG_ERROR("[HierarchyBuilder] Failed to merge meshes for '%s'", meshName.c_str());
			return ResourceId::Invalid();
		}

		if (releaseAfterUpload)
		{
			meshAsset->SetDataPolicy(MeshDataPolicy::ReleaseAfterUpload);
		}
		else
		{
			meshAsset->SetDataPolicy(MeshDataPolicy::KeepSourceData);
		}

		return resourceManager.CreateMeshFromAsset(meshName, std::move(meshAsset));
	}

} // namespace Framework
