#include "pch.h"
#include "ECS/Systems/TransformSystem.h"
#include "Math/MathUtils.h"
#include "Math/MathTypes.h"

namespace ECS
{
	void TransformSystem::SetRotationEuler(
		TransformComponent& transform,
		const Math::Vector3& eulerAngles
	)
	{
		transform.rotation = Math::QuaternionFromEuler(
			eulerAngles.x,
			eulerAngles.y,
			eulerAngles.z
		);
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
		return Math::QuaternionToEuler(transform.rotation);
	}

	void TransformSystem::Rotate(TransformComponent& transform, const Math::Vector3& eulerDelta)
	{
		Math::Quaternion delta = Math::QuaternionFromEuler(
			eulerDelta.x,
			eulerDelta.y,
			eulerDelta.z
		);

		transform.rotation = Math::QuaternionMultiply(transform.rotation, delta);
	}

	void TransformSystem::RotateAround(
		TransformComponent& transform,
		const Math::Vector3& axis,
		Core::float32 angle
	)
	{
		Math::Quaternion delta = Math::QuaternionFromAxisAngle(axis, angle);
		transform.rotation = Math::QuaternionMultiply(transform.rotation, delta);
	}

	Math::Matrix4x4 TransformSystem::GetLocalMatrix(const TransformComponent& transform)
	{
		// S * R * T 순서로 변환 행렬 생성
		Math::Matrix4x4 scaleMatrix = Math::MatrixScaling(
			transform.scale.x,
			transform.scale.y,
			transform.scale.z
		);

		Math::Matrix4x4 rotationMatrix = Math::MatrixRotationQuaternion(
			transform.rotation
		);

		Math::Matrix4x4 translationMatrix = Math::MatrixTranslation(
			transform.position.x,
			transform.position.y,
			transform.position.z
		);

		// 행렬 곱: S * R * T
		Math::Matrix4x4 worldMatrix = Math::MatrixMultiply(scaleMatrix, rotationMatrix);
		worldMatrix = Math::MatrixMultiply(worldMatrix, translationMatrix);

		return worldMatrix;
	}

} // namespace ECS
