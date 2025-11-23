#include "pch.h"
#include "ECS/Systems/CameraSystem.h"

#include "ECS/Archetype.h"
#include "ECS/Registry.h"
#include "ECS/RegistryView.h"

#include "Math/MathUtils.h"
#include "Core/Assert.h"
#include "Core/Logging/LogMacros.h"

using namespace Math;

namespace ECS
{
	void CameraSystem::UpdateViewMatrix(
		const TransformComponent& transform,
		CameraComponent& camera
	)
	{
		if (!camera.viewDirty)
		{
			return;
		}

		// Rotation Matrix (Quaternion → Matrix)
		Math::Matrix4x4 rotationMatrix = Math::MatrixRotationQuaternion(transform.rotation);

		// Rotation의 역행렬 = Transpose (직교 행렬이므로)
		Math::Matrix4x4 invRotation = Math::MatrixTranspose(rotationMatrix);

		// Translation의 역행렬 = -Position
		Math::Vector3 pos = transform.position;
		Math::Matrix4x4 invTranslation = Math::MatrixTranslation(-pos.x, -pos.y, -pos.z);

		// View = InvTranslation * InvRotation
		// (World = Rotation * Translation 이므로)
		camera.viewMatrix = Math::MatrixMultiply(invTranslation, invRotation);

		camera.viewDirty = false;

		//// Transform의 rotation에서 forward/up 벡터 추출
		//Vector3 position = transform.position;
		//Quaternion rotation = transform.rotation;

		//// Quaternion에서 방향 벡터 계산
		//Vector3 forward = QuaternionGetForward(rotation);
		//Vector3 target = Add(position, forward);
		//Vector3 up = Vector3(0.0f, 1.0f, 0.0f);

		//// View 행렬 계산 (Left-Handed)
		//camera.viewMatrix = MatrixLookAtLH(position, target, up);
		//camera.viewDirty = false;
	}

	void CameraSystem::UpdateProjectionMatrix(CameraComponent& camera)
	{
		if (!camera.projectionDirty)
		{
			return; // 이미 최신 상태
		}

		if (camera.projectionType == ProjectionType::Perspective)
		{
			// 원근 투영
			CORE_ASSERT(camera.fovY > 0.0f && camera.fovY < PI, "Invalid FOV");
			CORE_ASSERT(camera.aspectRatio > EPSILON, "Invalid aspect ratio");
			CORE_ASSERT(
				camera.nearPlane > 0.0f && camera.nearPlane < camera.farPlane,
				"Invalid clip planes"
			);

			camera.projectionMatrix = MatrixPerspectiveFovLH(
				camera.fovY,
				camera.aspectRatio,
				camera.nearPlane,
				camera.farPlane
			);
		}
		else if (camera.projectionType == ProjectionType::Orthographic)
		{
			// 직교 투영
			CORE_ASSERT(camera.orthoWidth > 0.0f, "Invalid ortho width");
			CORE_ASSERT(camera.orthoHeight > 0.0f, "Invalid ortho height");
			CORE_ASSERT(
				camera.nearPlane < camera.farPlane,
				"Invalid clip planes"
			);

			camera.projectionMatrix = MatrixOrthographicLH(
				camera.orthoWidth,
				camera.orthoHeight,
				camera.nearPlane,
				camera.farPlane
			);
		}

		camera.projectionDirty = false;
	}

	void CameraSystem::UpdateAllCameras(Registry& registry)
	{
		// Transform + Camera를 가진 Entity만 순회
		auto view = CameraArchetype::CreateView(registry);
		// auto view = registry.CreateView<TransformComponent, CameraComponent>();

		for (Entity entity : view)
		{
			auto* transform = registry.GetComponent<TransformComponent>(entity);
			auto* camera = registry.GetComponent<CameraComponent>(entity);

			UpdateViewMatrix(*transform, *camera);
			UpdateProjectionMatrix(*camera);
		}
	}

	Entity CameraSystem::FindMainCamera(Registry& registry)
	{
		// CameraComponent를 가진 Entity만 순회
		auto view = CameraOnlyArchetype::CreateView(registry);
		// auto view = registry.CreateView<CameraComponent>();

		for (Entity entity : view)
		{
			auto* camera = registry.GetComponent<CameraComponent>(entity);
			if (camera->isMainCamera)
			{
				return entity;
			}
		}

		return Entity::Invalid();  // Invalid Entity
	}

	void CameraSystem::SetFovYDegrees(CameraComponent& camera, Core::float32 degrees)
	{
		CORE_ASSERT(
			degrees > 0.0f && degrees < 180.0f,
			"FOV must be between 0 and 180 degrees"
		);

		camera.fovY = DegToRad(degrees);
		camera.projectionDirty = true;
	}

	void CameraSystem::SetFovYRadians(CameraComponent& camera, Core::float32 radians)
	{
		CORE_ASSERT(
			radians > 0.0f && radians < PI,
			"FOV must be between 0 and π radians"
		);

		camera.fovY = radians;
		camera.projectionDirty = true;
	}

	void CameraSystem::SetAspectRatio(CameraComponent& camera, Core::float32 aspectRatio)
	{
		CORE_ASSERT(aspectRatio > 0.0f, "Aspect ratio must be positive");

		camera.aspectRatio = aspectRatio;
		camera.projectionDirty = true;
	}

	void CameraSystem::SetAspectRatio(
		CameraComponent& camera,
		Core::float32 width,
		Core::float32 height
	)
	{
		CORE_ASSERT(width > 0.0f && height > 0.0f, "Width and height must be positive");

		camera.aspectRatio = width / height;
		camera.projectionDirty = true;
	}

	void CameraSystem::SetClipPlanes(
		CameraComponent& camera,
		Core::float32 nearPlane,
		Core::float32 farPlane
	)
	{
		CORE_ASSERT(
			nearPlane > 0.0f && nearPlane < farPlane,
			"Invalid clip planes"
		);

		camera.nearPlane = nearPlane;
		camera.farPlane = farPlane;
		camera.projectionDirty = true;
	}

	void CameraSystem::SetLookAt(
		TransformComponent& transform,
		const Vector3& position,
		const Vector3& target,
		const Vector3& up
	)
	{
		// 카메라 위치 설정
		transform.position = position;

		// View Matrix 생성 (DirectXMath가 알아서 처리)
		Matrix4x4 viewMatrix = MatrixLookAtLH(position, target, up);

		// View Matrix의 역행렬 = World Transform
		Matrix4x4 worldMatrix = MatrixInverse(viewMatrix);

		// World Transform에서 Rotation만 추출
		transform.rotation = QuaternionFromRotationMatrix(worldMatrix);

		// View Dirty 플래그는 Transform 변경으로 자동 설정됨
		// (향후 Transform에 Dirty Flag 추가 시)
	}

	void CameraSystem::SetMainCamera(Registry& registry, Entity entity)
	{
		// CameraComponent를 가진 Entity만 효율적으로 순회
		auto view = CameraOnlyArchetype::CreateView(registry);
		// auto view = registry.CreateView<CameraComponent>();

		for (Entity e : view)
		{
			auto* camera = registry.GetComponent<CameraComponent>(e);
			camera->isMainCamera = false;
		}

		// 지정된 Entity를 Main Camera로 설정
		auto* targetCamera = registry.GetComponent<CameraComponent>(entity);
		if (targetCamera)
		{
			targetCamera->isMainCamera = true;
			LOG_INFO("[CameraSystem] Main Camera set to Entity (ID: %u)", entity.id);
		}
		else
		{
			LOG_WARN("[CameraSystem] Entity (ID: %u) has no CameraComponent", entity.id);
		}
	}

} // namespace ECS
