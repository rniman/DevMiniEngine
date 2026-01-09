#include "pch.h"
#include "ECS/Systems/CameraSystem.h"

#include "ECS/Archetype.h"
#include "ECS/Registry.h"
#include "ECS/RegistryView.h"

#include "Math/MathUtils.h"
#include "Core/Assert.h"
#include "Core/Logging/LogMacros.h"

namespace ECS
{
	//=============================================================================
	// 생성자
	//=============================================================================

	CameraSystem::CameraSystem(Registry& registry)
		: ISystem(registry)
	{
	}

	//=============================================================================
	// ISystem 인터페이스 구현
	//=============================================================================

	void CameraSystem::Initialize()
	{
		LOG_INFO("[CameraSystem] Initialized");
	}

	void CameraSystem::Update(Core::float32 deltaTime)
	{
		(void)deltaTime;
		UpdateAllCameras(*GetRegistry());
	}

	void CameraSystem::Shutdown()
	{
		LOG_INFO("[CameraSystem] Shutdown");
	}

	//=============================================================================
	// 고수준 API (Entity 기반)
	//=============================================================================

	Entity CameraSystem::FindMainCamera()
	{
		return FindMainCamera(*GetRegistry());
	}

	bool CameraSystem::SetMainCamera(Entity entity)
	{
		auto* targetCamera = GetRegistry()->GetComponent<CameraComponent>(entity);
		if (!targetCamera)
		{
			LOG_WARN("[CameraSystem] Entity %u has no CameraComponent", entity.id);
			return false;
		}

		// 기존 Main Camera 해제
		auto view = CameraOnlyArchetype::CreateView(*GetRegistry());
		for (Entity e : view)
		{
			auto* camera = GetRegistry()->GetComponent<CameraComponent>(e);
			if (camera)
			{
				camera->isMainCamera = false;
			}
		}

		targetCamera->isMainCamera = true;
		LOG_INFO("[CameraSystem] Main camera set to Entity (ID: %u)", entity.id);
		return true;
	}

	bool CameraSystem::SetFovYDegrees(Entity entity, Core::float32 degrees)
	{
		auto* camera = GetRegistry()->GetComponent<CameraComponent>(entity);
		if (!camera)
		{
			return false;
		}

		SetFovYDegreesInternal(*camera, degrees);
		return true;
	}

	bool CameraSystem::SetFovYRadians(Entity entity, Core::float32 radians)
	{
		auto* camera = GetRegistry()->GetComponent<CameraComponent>(entity);
		if (!camera)
		{
			return false;
		}

		SetFovYRadiansInternal(*camera, radians);
		return true;
	}

	bool CameraSystem::SetAspectRatio(Entity entity, Core::float32 aspectRatio)
	{
		auto* camera = GetRegistry()->GetComponent<CameraComponent>(entity);
		if (!camera)
		{
			return false;
		}

		SetAspectRatioInternal(*camera, aspectRatio);
		return true;
	}

	bool CameraSystem::SetAspectRatio(Entity entity, Core::float32 width, Core::float32 height)
	{
		auto* camera = GetRegistry()->GetComponent<CameraComponent>(entity);
		if (!camera)
		{
			return false;
		}

		CORE_ASSERT(width > 0.0f && height > 0.0f, "Width and height must be positive");
		SetAspectRatioInternal(*camera, width / height);
		return true;
	}

	bool CameraSystem::SetClipPlanes(Entity entity, Core::float32 nearPlane, Core::float32 farPlane)
	{
		auto* camera = GetRegistry()->GetComponent<CameraComponent>(entity);
		if (!camera)
		{
			return false;
		}

		SetClipPlanesInternal(*camera, nearPlane, farPlane);
		return true;
	}

	bool CameraSystem::SetUpMode(Entity entity, CameraUpMode mode)
	{
		auto* camera = GetRegistry()->GetComponent<CameraComponent>(entity);
		if (!camera)
		{
			return false;
		}

		SetUpModeInternal(*camera, mode);
		return true;
	}

	bool CameraSystem::SetWorldUpReference(Entity entity, const Math::Vector3& worldUp)
	{
		auto* camera = GetRegistry()->GetComponent<CameraComponent>(entity);
		if (!camera)
		{
			return false;
		}

		SetWorldUpReferenceInternal(*camera, worldUp);
		return true;
	}

	bool CameraSystem::SetLocalUp(Entity entity, const Math::Vector3& localUp)
	{
		auto* camera = GetRegistry()->GetComponent<CameraComponent>(entity);
		if (!camera)
		{
			return false;
		}

		SetLocalUpInternal(*camera, localUp);
		return true;
	}

	bool CameraSystem::SetLookAt(
		Entity entity,
		const Math::Vector3& position,
		const Math::Vector3& target,
		const Math::Vector3& worldUp
	)
	{
		auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		auto* camera = GetRegistry()->GetComponent<CameraComponent>(entity);

		if (!transform || !camera)
		{
			LOG_WARN("[CameraSystem] Entity %u missing required components", entity.id);
			return false;
		}

		SetLookAtInternal(*transform, *camera, position, target, worldUp);
		return true;
	}

	//=============================================================================
	// 저수준 API (public static)
	//=============================================================================

	Entity CameraSystem::FindMainCamera(Registry& registry)
	{
		auto view = CameraOnlyArchetype::CreateView(registry);

		for (Entity entity : view)
		{
			auto* camera = registry.GetComponent<CameraComponent>(entity);
			if (camera && camera->isMainCamera)
			{
				return entity;
			}
		}

		return Entity::Invalid();
	}

	void CameraSystem::UpdateViewMatrix(const TransformComponent& transform, CameraComponent& camera)
	{
		// 카메라는 씬에 1-2개이므로 매 프레임 항상 계산
		Math::Vector3 forward = transform.rotation.RotateVector(camera.forward);
		Math::Vector3 up;

		if (camera.upMode == CameraUpMode::WorldUp)
		{
			// FPS 스타일: 월드 up 고정
			up = camera.worldUpReference;
		}
		else
		{
			// 비행 스타일: 로컬 up을 회전
			up = transform.rotation.RotateVector(camera.localUp);
		}

		Math::Vector3 target = transform.position + forward;
		camera.viewMatrix = Math::MatrixLookAtLH(transform.position, target, up);
	}

	void CameraSystem::UpdateProjectionMatrix(CameraComponent& camera)
	{
		// 카메라는 씬에 1-2개이므로 매 프레임 항상 계산
		if (camera.projectionType == ProjectionType::Perspective)
		{
			CORE_ASSERT(camera.fovY > 0.0f && camera.fovY < Math::PI, "Invalid FOV");
			CORE_ASSERT(camera.aspectRatio > Math::EPSILON, "Invalid aspect ratio");
			CORE_ASSERT(camera.nearPlane > 0.0f && camera.nearPlane < camera.farPlane, "Invalid clip planes");

			camera.projectionMatrix = Math::MatrixPerspectiveFovLH(
				camera.fovY,
				camera.aspectRatio,
				camera.nearPlane,
				camera.farPlane
			);
		}
		else
		{
			CORE_ASSERT(camera.orthoWidth > 0.0f, "Invalid ortho width");
			CORE_ASSERT(camera.orthoHeight > 0.0f, "Invalid ortho height");

			camera.projectionMatrix = Math::MatrixOrthographicLH(
				camera.orthoWidth,
				camera.orthoHeight,
				camera.nearPlane,
				camera.farPlane
			);
		}
	}

	void CameraSystem::UpdateAllCameras(Registry& registry)
	{
		auto view = CameraArchetype::CreateView(registry);

		for (Entity entity : view)
		{
			auto* transform = registry.GetComponent<TransformComponent>(entity);
			auto* camera = registry.GetComponent<CameraComponent>(entity);

			if (transform && camera)
			{
				UpdateViewMatrix(*transform, *camera);
				UpdateProjectionMatrix(*camera);
			}
		}
	}

	//=============================================================================
	// 내부 헬퍼 함수 (private static)
	//=============================================================================

	void CameraSystem::SetFovYDegreesInternal(CameraComponent& camera, Core::float32 degrees)
	{
		CORE_ASSERT(degrees > 0.0f && degrees < 180.0f, "FOV must be between 0 and 180 degrees");
		camera.fovY = Math::DegToRad(degrees);
	}

	void CameraSystem::SetFovYRadiansInternal(CameraComponent& camera, Core::float32 radians)
	{
		CORE_ASSERT(radians > 0.0f && radians < Math::PI, "FOV must be between 0 and PI radians");
		camera.fovY = radians;
	}

	void CameraSystem::SetAspectRatioInternal(CameraComponent& camera, Core::float32 aspectRatio)
	{
		CORE_ASSERT(aspectRatio > 0.0f, "Aspect ratio must be positive");
		camera.aspectRatio = aspectRatio;
	}

	void CameraSystem::SetClipPlanesInternal(CameraComponent& camera, Core::float32 nearPlane, Core::float32 farPlane)
	{
		CORE_ASSERT(nearPlane > 0.0f, "Near plane must be positive");
		CORE_ASSERT(farPlane > nearPlane, "Far plane must be greater than near plane");
		camera.nearPlane = nearPlane;
		camera.farPlane = farPlane;
	}

	void CameraSystem::SetUpModeInternal(CameraComponent& camera, CameraUpMode mode)
	{
		camera.upMode = mode;
	}

	void CameraSystem::SetWorldUpReferenceInternal(CameraComponent& camera, const Math::Vector3& worldUp)
	{
		camera.worldUpReference = worldUp.Normalized();
	}

	void CameraSystem::SetLocalUpInternal(CameraComponent& camera, const Math::Vector3& localUp)
	{
		camera.localUp = localUp.Normalized();
	}

	void CameraSystem::SetLookAtInternal(
		TransformComponent& transform,
		CameraComponent& camera,
		const Math::Vector3& position,
		const Math::Vector3& target,
		const Math::Vector3& worldUp
	)
	{
		transform.position = position;

		Math::Vector3 forward = (target - position).Normalized();

		// 카메라 방향 저장
		camera.forward = forward;
		camera.worldUpReference = worldUp;
	}

} // namespace ECS
