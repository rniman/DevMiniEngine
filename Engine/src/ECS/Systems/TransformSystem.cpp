#include "pch.h"
#include "ECS/Systems/TransformSystem.h"
#include "ECS/Registry.h"
#include "Core/Logging/LogMacros.h"
#include "Math/MathUtils.h"
#include "Math/MathTypes.h"

namespace ECS
{
	//=============================================================================
	// 생성자
	//=============================================================================

	TransformSystem::TransformSystem(Registry& registry)
		: ISystem(registry)
	{
	}

	//=============================================================================
	// ISystem 인터페이스 구현
	//=============================================================================

	void TransformSystem::Initialize()
	{
		LOG_INFO("[TransformSystem] Initialized");
	}

	void TransformSystem::Update(Core::float32 deltaTime)
	{
		// Phase 3.5에서 계층 구조 업데이트, Dirty Flag 처리 구현 예정
	}

	void TransformSystem::Shutdown()
	{
		LOG_INFO("[TransformSystem] Shutdown");
	}

	//=============================================================================
	// 고수준 API (Entity 기반)
	//=============================================================================

	bool TransformSystem::SetPosition(Entity entity, const Math::Vector3& position)
	{
		auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		transform->position = position;
		return true;
	}

	bool TransformSystem::GetPosition(Entity entity, Math::Vector3& outPosition)
	{
		auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		outPosition = transform->position;
		return true;
	}

	bool TransformSystem::SetRotationEuler(Entity entity, const Math::Vector3& eulerAngles)
	{
		auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		SetRotationEuler(*transform, eulerAngles);
		return true;
	}

	bool TransformSystem::SetRotation(Entity entity, const Math::Quaternion& rotation)
	{
		auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		transform->rotation = rotation.Normalized();
		return true;
	}

	bool TransformSystem::GetRotationEuler(Entity entity, Math::Vector3& outEuler)
	{
		auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		outEuler = GetRotationEuler(*transform);
		return true;
	}

	bool TransformSystem::SetScale(Entity entity, const Math::Vector3& scale)
	{
		auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		transform->scale = scale;
		return true;
	}

	bool TransformSystem::SetScale(Entity entity, Core::float32 uniformScale)
	{
		return SetScale(entity, Math::Vector3(uniformScale));
	}

	bool TransformSystem::Rotate(Entity entity, const Math::Vector3& eulerDelta)
	{
		auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		Rotate(*transform, eulerDelta);
		return true;
	}

	bool TransformSystem::RotateAround(Entity entity, const Math::Vector3& axis, Core::float32 angle)
	{
		auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		RotateAround(*transform, axis, angle);
		return true;
	}

	bool TransformSystem::Translate(Entity entity, const Math::Vector3& delta)
	{
		auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		transform->position += delta;
		return true;
	}

	bool TransformSystem::GetWorldMatrix(Entity entity, Math::Matrix4x4& outMatrix)
	{
		auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		outMatrix = GetWorldMatrix(*transform);
		return true;
	}

	bool TransformSystem::GetWorldInvTranspose(Entity entity, Math::Matrix4x4& outMatrix)
	{
		auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		outMatrix = GetWorldInvTranspose(*transform);
		return true;
	}

	bool TransformSystem::LookAt(Entity entity, const Math::Vector3& target, const Math::Vector3& up)
	{
		auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		Math::Vector3 forward = (target - transform->position).Normalized();
		Math::Vector3 right = up.Cross(forward).Normalized();
		Math::Vector3 adjustedUp = forward.Cross(right);

		Math::Matrix4x4 rotMatrix = Math::Matrix4x4::Identity();
		rotMatrix.m[0][0] = right.x;
		rotMatrix.m[0][1] = right.y;
		rotMatrix.m[0][2] = right.z;
		rotMatrix.m[1][0] = adjustedUp.x;
		rotMatrix.m[1][1] = adjustedUp.y;
		rotMatrix.m[1][2] = adjustedUp.z;
		rotMatrix.m[2][0] = forward.x;
		rotMatrix.m[2][1] = forward.y;
		rotMatrix.m[2][2] = forward.z;

		transform->rotation = Math::QuaternionFromRotationMatrix(rotMatrix);
		return true;
	}

	//=============================================================================
	// 저수준 API (Component 직접) - 정적
	//=============================================================================

	void TransformSystem::SetRotationEuler(TransformComponent& transform, const Math::Vector3& eulerAngles)
	{
		transform.rotation = Math::QuaternionFromEuler(eulerAngles);
	}

	void TransformSystem::SetRotationEuler(
		TransformComponent& transform,
		Core::float32 pitch,
		Core::float32 yaw,
		Core::float32 roll
	)
	{
		transform.rotation = Math::QuaternionFromEuler(pitch, yaw, roll);
	}

	Math::Vector3 TransformSystem::GetRotationEuler(const TransformComponent& transform)
	{
		return transform.rotation.ToEuler();
	}

	void TransformSystem::Rotate(TransformComponent& transform, const Math::Vector3& eulerDelta)
	{
		Math::Quaternion delta = Math::QuaternionFromEuler(eulerDelta);
		transform.rotation = (transform.rotation * delta).Normalized();
	}

	void TransformSystem::RotateAround(TransformComponent& transform, const Math::Vector3& axis, Core::float32 angle)
	{
		Math::Quaternion delta = Math::QuaternionFromAxisAngle(axis, angle);
		transform.rotation = (transform.rotation * delta).Normalized();
	}

	Math::Matrix4x4 TransformSystem::GetLocalMatrix(const TransformComponent& transform)
	{
		Math::Matrix4x4 scaleMatrix = Math::MatrixScaling(transform.scale);
		Math::Matrix4x4 rotationMatrix = Math::MatrixRotationQuaternion(transform.rotation);
		Math::Matrix4x4 translationMatrix = Math::MatrixTranslation(transform.position);

		return scaleMatrix * rotationMatrix * translationMatrix;
	}

	Math::Matrix4x4 TransformSystem::GetWorldMatrix(const TransformComponent& transform)
	{
		return GetLocalMatrix(transform);
	}

	Math::Matrix4x4 TransformSystem::GetWorldInvTranspose(const TransformComponent& transform)
	{
		if (IsUniformScale(transform.scale))
		{
			// 균등 스케일: 역전치 불필요
			return GetWorldMatrix(transform);
		}

		// 비균등 스케일: 역전치 계산
		return Math::MatrixTranspose(Math::MatrixInverse(GetWorldMatrix(transform)));
	}

	Math::Vector3 TransformSystem::GetForward(const TransformComponent& transform)
	{
		return transform.rotation.GetForward();
	}

	Math::Vector3 TransformSystem::GetRight(const TransformComponent& transform)
	{
		return transform.rotation.GetRight();
	}

	Math::Vector3 TransformSystem::GetUp(const TransformComponent& transform)
	{
		return transform.rotation.GetUp();
	}

} // namespace ECS
