#include "pch.h"
#include "ECS/Systems/RenderSystem.h"

// ECS
#include "ECS/Archetype.h"
#include "ECS/Entity.h"
#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/MaterialComponent.h"
#include "ECS/Components/MeshComponent.h"
#include "ECS/Components/LightComponents.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Registry.h"
#include "ECS/RegistryView.h"
#include "ECS/Systems/CameraSystem.h"
#include "ECS/Systems/LightingSystem.h"
#include "ECS/Systems/TransformSystem.h"

// Core
#include "Core/Assert.h"
#include "Core/Types.h"
#include "Core/Logging/LogMacros.h"

// Framework
#include "Framework/Resources/ResourceManager.h"

// Graphics
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"

// Math
#include "Math/MathUtils.h"  // MatrixTranspose

namespace ECS
{
	//=============================================================================
	// 생성자
	//=============================================================================

	RenderSystem::RenderSystem(Registry& registry, Framework::ResourceManager* resourceManager)
		: ISystem(registry)
		, mResourceManager(resourceManager)
	{
		CORE_ASSERT(resourceManager != nullptr, "ResourceManager cannot be null");
	}

	//=============================================================================
	// ISystem 인터페이스 구현
	//=============================================================================

	void RenderSystem::Initialize()
	{
		LOG_INFO("[RenderSystem] Initialized");
	}

	void RenderSystem::Update(Core::float32 deltaTime)
	{
		// 이전 프레임 데이터 정리
		mFrameData.Clear();

		// Main Camera 찾기 (정적 함수 사용)
		Entity mainCameraEntity = CameraSystem::FindMainCamera(*GetRegistry());
		if (!mainCameraEntity.IsValid())
		{
			LOG_WARN("[RenderSystem] No main camera found!");
			return;
		}

		// Camera Component 가져오기
		auto* cameraComp = GetRegistry()->GetComponent<CameraComponent>(mainCameraEntity);
		auto* cameraTransform = GetRegistry()->GetComponent<TransformComponent>(mainCameraEntity);

		if (!cameraComp || !cameraTransform)
		{
			LOG_ERROR("[RenderSystem] Main camera missing required components!");
			return;
		}

		// 카메라 데이터 설정
		mFrameData.viewMatrix = cameraComp->viewMatrix;
		mFrameData.projectionMatrix = cameraComp->projectionMatrix;
		mFrameData.cameraPosition = cameraTransform->position;

		// VP 미리 계산
		Math::Matrix4x4 viewProj = mFrameData.viewMatrix * mFrameData.projectionMatrix;

		// 조명 데이터 수집 (정적 함수 사용)
		LightingSystem::CollectDirectionalLights(*GetRegistry(), mFrameData.directionalLights);
		LightingSystem::CollectPointLights(*GetRegistry(), mFrameData.pointLights);

		// Transform + Mesh + Material을 가진 Entity 찾기
		auto view = RenderableArchetype::CreateView(*GetRegistry());

		// 렌더링 데이터 수집
		for (Entity entity : view)
		{
			auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
			auto* meshComp = GetRegistry()->GetComponent<MeshComponent>(entity);
			auto* materialComp = GetRegistry()->GetComponent<MaterialComponent>(entity);

			// World Matrix 계산 (정적 함수 사용)
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

			// RenderItem 추가
			Graphics::RenderItem renderItem;
			renderItem.mesh = mesh;
			renderItem.material = material;
			renderItem.worldMatrix = worldMatrix;

			// MVP 미리 계산
			renderItem.mvpMatrix = Math::MatrixTranspose(worldMatrix * viewProj);

			mFrameData.opaqueItems.push_back(renderItem);
		}

		// 통계 로깅 (디버그)
#ifdef _DEBUG
		static Core::float32 logTimer = 0.0f;
		logTimer += deltaTime;

		if (logTimer >= 1.0f)
		{
			if (!mFrameData.opaqueItems.empty() ||
				!mFrameData.directionalLights.empty() ||
				!mFrameData.pointLights.empty())
			{
				LOG_DEBUG(
					"[RenderSystem] Collected: %zu items, %zu dir lights, %zu point lights",
					mFrameData.opaqueItems.size(),
					mFrameData.directionalLights.size(),
					mFrameData.pointLights.size()
				);
			}
			logTimer = 0.0f;
		}
#endif
	}

	void RenderSystem::Shutdown()
	{
		mFrameData.Clear();
		LOG_INFO("[RenderSystem] Shutdown");
	}

} // namespace ECS
