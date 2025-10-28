#include "Graphics/Camera/Camera.h"
#include "Math/MathUtils.h"

using namespace Math;

namespace Graphics
{
	Camera::Camera()
	{
		mViewMatrix = MatrixIdentity();
		mProjectionMatrix = MatrixIdentity();
	}

	void Camera::MoveForward(float distance)
	{
		MoveAlongDirection(GetForwardVector(), distance);
	}

	void Camera::MoveRight(float distance)
	{
		MoveAlongDirection(GetRightVector(), distance);
	}

	void Camera::MoveUp(float distance)
	{
		MoveAlongDirection(mUpVector, distance);
	}

	void Camera::RotateYaw(float angle)
	{
		Vector3 forward = Subtract(mTarget, mPosition);
		float distance = Length(forward);

		// Up 벡터를 회전축으로 사용하여 수평 회전 구현
		Matrix4x4 rotationMatrix = MatrixRotationAxis(mUpVector, angle);
		Vector3 rotatedForward = Vector3TransformNormal(forward, rotationMatrix);

		Vector3 newForward = Normalize(rotatedForward);
		newForward = Multiply(newForward, distance);

		mTarget = Add(mPosition, newForward);
		mViewDirty = true;
	}

	void Camera::RotatePitch(float angle)
	{
		Vector3 right = GetRightVector();
		Vector3 forward = Subtract(mTarget, mPosition);
		float distance = Length(forward);

		Matrix4x4 rotationMatrix = MatrixRotationAxis(right, angle);
		Vector3 rotatedForward = Vector3TransformNormal(forward, rotationMatrix);

		Vector3 newForward = Normalize(rotatedForward);
		newForward = Multiply(newForward, distance);

		mTarget = Add(mPosition, newForward);

		// Pitch 회전 시 Up 벡터도 함께 회전시켜 카메라 롤 방지
		mUpVector = Vector3TransformNormal(mUpVector, rotationMatrix);

		mViewDirty = true;
	}

	void Camera::UpdateViewMatrix()
	{
		if (mViewDirty)
		{
			mViewMatrix = MatrixLookAtLH(mPosition, mTarget, mUpVector);
			mViewDirty = false;
		}
	}

	void Camera::SetPosition(const Vector3& position)
	{
		mPosition = position;
		mViewDirty = true;
	}

	void Camera::SetTarget(const Vector3& target)
	{
		mTarget = target;
		mViewDirty = true;
	}

	void Camera::SetUpVector(const Vector3& up)
	{
		mUpVector = up;
		mViewDirty = true;
	}

	void Camera::SetLookAt(const Vector3& position, const Vector3& target, const Vector3& up)
	{
		mPosition = position;
		mTarget = target;
		mUpVector = up;
		mViewDirty = true;
	}

	void Camera::SetLookTo(const Vector3& position, const Vector3& direction, const Vector3& up)
	{
		mPosition = position;
		// direction 벡터로부터 절대 타겟 위치 계산
		mTarget = Add(position, direction);
		mUpVector = up;
		mViewDirty = true;
	}

	Vector3 Camera::GetForwardVector() const
	{
		Vector3 forward = Subtract(mTarget, mPosition);
		return Normalize(forward);
	}

	Vector3 Camera::GetRightVector() const
	{
		Vector3 forward = GetForwardVector();
		Vector3 right = Cross(forward, mUpVector);
		return Normalize(right);
	}

	Matrix4x4 Camera::GetViewProjectionMatrix() const
	{
		return MatrixMultiply(mViewMatrix, mProjectionMatrix);
	}

	void Camera::MoveAlongDirection(const Math::Vector3& direction, float distance)
	{
		Vector3 offset = Multiply(direction, distance);
		mPosition = Add(mPosition, offset);
		mTarget = Add(mTarget, offset);
		mViewDirty = true;
	}

} // namespace Graphics