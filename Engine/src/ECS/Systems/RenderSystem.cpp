// Engine/src/ECS/Systems/RenderSystem.cpp
#include "pch.h"
#include "ECS/Systems/RenderSystem.h"
#include "ECS/Archetype.h"
#include "ECS/Entity.h"
#include "ECS/Registry.h"
#include "ECS/RegistryView.h"
#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/MaterialComponent.h"
#include "ECS/Components/MeshComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Systems/CameraSystem.h"
#include "ECS/Systems/LightingSystem.h"
#include "ECS/Systems/TransformSystem.h"
#include "Core/Assert.h"
#include "Core/Logging/LogMacros.h"
#include "Framework/Resources/ResourceManager.h"
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"
#include "Math/MathUtils.h"

namespace ECS
{
	RenderSystem::RenderSystem(Registry& registry, Framework::ResourceManager* resourceManager)
		: ISystem(registry)
		, mResourceManager(resourceManager)
	{
		CORE_ASSERT(resourceManager != nullptr, "ResourceManager cannot be null");
	}

	//=========================================================================
	// ISystem 인터페이스
	//=========================================================================

	void RenderSystem::Initialize()
	{
		LOG_INFO("[RenderSystem] Initialized");
	}

	void RenderSystem::Update(Core::float32 deltaTime)
	{
		mFrameData.Clear();

		// Main Camera 찾기
		Entity mainCameraEntity = CameraSystem::FindMainCamera(*GetRegistry());
		if (!mainCameraEntity.IsValid())
		{
			LOG_WARN("[RenderSystem] No main camera found!");
			return;
		}

		auto* cameraComp = GetRegistry()->GetComponent<CameraComponent>(mainCameraEntity);
		auto* cameraTransform = GetRegistry()->GetComponent<TransformComponent>(mainCameraEntity);
		if (!cameraComp || !cameraTransform)
		{
			LOG_ERROR("[RenderSystem] Main camera missing required components!");
			return;
		}

		// 카메라 데이터
		mFrameData.viewMatrix = cameraComp->viewMatrix;
		mFrameData.projectionMatrix = cameraComp->projectionMatrix;
		mFrameData.cameraPosition = cameraTransform->position;

		Math::Matrix4x4 viewProj = mFrameData.viewMatrix * mFrameData.projectionMatrix;

		// 조명 데이터 수집
		LightingSystem::CollectDirectionalLights(*GetRegistry(), mFrameData.directionalLights);
		LightingSystem::CollectPointLights(*GetRegistry(), mFrameData.pointLights);

		// Debug Entity 수집
		LightingSystem::CollectDirectionalLightEntities(*GetRegistry(), mFrameData.debug.directionalLightEntities);
		LightingSystem::CollectPointLightEntities(*GetRegistry(), mFrameData.debug.pointLightEntities);

		// Renderable Entity 순회
		auto view = RenderableArchetype::CreateView(*GetRegistry());

		for (Entity entity : view)
		{
			auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
			auto* meshComp = GetRegistry()->GetComponent<MeshComponent>(entity);
			auto* materialComp = GetRegistry()->GetComponent<MaterialComponent>(entity);

			Math::Matrix4x4 worldMatrix = TransformSystem::GetWorldMatrix(*transform);

			Graphics::Mesh* mesh = mResourceManager->GetMesh(meshComp->meshId);
			if (!mesh)
			{
				LOG_WARN("[RenderSystem] Mesh not found for entity %u", entity.id);
				continue;
			}

			Graphics::Material* material = mResourceManager->GetMaterial(materialComp->materialId);
			if (!material)
			{
				LOG_WARN("[RenderSystem] Material not found for entity %u", entity.id);
				continue;
			}

			Graphics::RenderItem renderItem;
			renderItem.mesh = mesh;
			renderItem.material = material;
			renderItem.worldMatrix = worldMatrix;
			renderItem.mvpMatrix = Math::MatrixTranspose(worldMatrix * viewProj);

			mFrameData.opaqueItems.push_back(renderItem);
		}
	}

	void RenderSystem::Shutdown()
	{
		mFrameData.Clear();
		LOG_INFO("[RenderSystem] Shutdown");
	}

} // namespace ECS
