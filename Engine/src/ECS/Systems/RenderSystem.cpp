#include "pch.h"
#include "ECS/Systems/RenderSystem.h"

// ECS
#include "ECS/Components/MaterialComponent.h"
#include "ECS/Components/MeshComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Registry.h"
#include "ECS/RegistryView.h"
#include "ECS/Systems/TransformSystem.h"

// Framework
#include "Framework/Resources/ResourceManager.h"

// Graphics
#include "Graphics/Camera/PerspectiveCamera.h"
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"


namespace ECS
{
	RenderSystem::RenderSystem(
		Framework::ResourceManager* resourceManager,
		Graphics::PerspectiveCamera* camera
	)
		: mResourceManager(resourceManager)
		, mCamera(camera)
	{
		CORE_ASSERT(resourceManager != nullptr, "ResourceManager cannot be null");
		CORE_ASSERT(camera != nullptr, "Camera cannot be null");
	}

	void RenderSystem::OnInitialize(Registry& registry)
	{
		LOG_INFO("[RenderSystem] Initialized");
	}

	void RenderSystem::OnUpdate(Registry& registry, Core::float32 deltaTime)
	{
		// FrameData 초기화
		mFrameData.Clear();  // opaqueItems, transparentItems 모두 클리어

		// 카메라 행렬 업데이트
		mCamera->UpdateViewMatrix();
		mCamera->UpdateProjectionMatrix();
		mFrameData.viewMatrix = mCamera->GetViewMatrix();
		mFrameData.projectionMatrix = mCamera->GetProjectionMatrix();

		// Transform + Mesh + Material을 가진 Entity 찾기
		auto view = registry.CreateView<
			TransformComponent,
			MeshComponent,
			MaterialComponent
		>();

		// 렌더링 데이터 수집
		for (Entity entity : view)
		{
			auto* transform = registry.GetComponent<TransformComponent>(entity);
			auto* meshComp = registry.GetComponent<MeshComponent>(entity);
			auto* materialComp = registry.GetComponent<MaterialComponent>(entity);

			// World Matrix 계산
			Math::Matrix4x4 worldMatrix = TransformSystem::GetWorldMatrix(*transform);

			// Mesh 리소스 가져오기
			Graphics::Mesh* mesh = mResourceManager->GetMesh(meshComp->meshId);
			if (mesh == nullptr)
			{
				LOG_WARN("[RenderSystem] Mesh not found for entity (ID: %u)", entity.id);
				continue;
			}

			// Material 리소스 가져오기
			Graphics::Material* material = mResourceManager->GetMaterial(materialComp->materialId);
			if (material == nullptr)
			{
				LOG_WARN("[RenderSystem] Material not found for entity (ID: %u)", entity.id);
				continue;
			}

			// RenderItem 추가 (RenderObject → RenderItem)
			Graphics::RenderItem renderItem;
			renderItem.mesh = mesh;  // const 포인터로 저장
			renderItem.material = material;
			renderItem.worldMatrix = worldMatrix;

			// MVP 미리 계산 (선택적, 셰이더에서 할 수도 있음)
			Math::Matrix4x4 viewProj = Math::MatrixMultiply(
				mFrameData.viewMatrix,
				mFrameData.projectionMatrix
			);
			renderItem.mvpMatrix = Math::MatrixMultiply(worldMatrix, viewProj);
			renderItem.mvpMatrix = Math::MatrixTranspose(renderItem.mvpMatrix);

			// Phase 3.2에서는 모두 불투명으로 처리
			mFrameData.opaqueItems.push_back(renderItem);
		}

		// 통계 로깅 (디버그)
#ifdef _DEBUG
		if (!mFrameData.opaqueItems.empty())
		{
			static Core::float32 logTimer = 0.0f;
			logTimer += deltaTime;

			if (logTimer >= 1.0f)
			{
				LOG_DEBUG(
					"[RenderSystem] Collected %zu opaque items",
					mFrameData.opaqueItems.size()
				);
				logTimer = 0.0f;
			}
		}
#endif
	}

	void RenderSystem::OnShutdown(Registry& registry)
	{
		mFrameData.Clear();
		LOG_INFO("[RenderSystem] Shutdown");
	}

} // namespace ECS
