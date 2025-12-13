#pragma once
#include "Core/Types.h"
#include "Math/MathTypes.h"
#include <cstdlib>
#include <cmath>
#include <DirectXMath.h>

namespace Math
{
	//=============================================================================
	// 부동소수점 비교 상수
	// 기본 EPSILON은 MathTypes.h에 정의됨
	//=============================================================================

	/// @brief 더 엄격한 비교가 필요할 때
	constexpr Core::float32 EPSILON_STRICT = 1e-8f;

	/// @brief 덜 엄격한 비교 (렌더링 등)
	constexpr Core::float32 EPSILON_LOOSE = 1e-4f;

	//=============================================================================
	// 부동소수점 비교 함수
	//=============================================================================

	/**
	 * @brief 부동소수점 값이 0에 가까운지 검사
	 * @param value 검사할 값
	 * @return 0에 가까우면 true
	 */
	inline bool IsZero(Core::float32 value)
	{
		return std::abs(value) < EPSILON;
	}

	/**
	 * @brief 부동소수점 값이 0에 가까운지 검사 (커스텀 epsilon)
	 * @param value 검사할 값
	 * @param epsilon 비교 임계값
	 * @return 0에 가까우면 true
	 */
	inline bool IsZero(Core::float32 value, Core::float32 epsilon)
	{
		return std::abs(value) < epsilon;
	}

	/**
	 * @brief 두 부동소수점 값이 거의 같은지 검사
	 * @param a 첫 번째 값
	 * @param b 두 번째 값
	 * @return 거의 같으면 true
	 */
	inline bool IsEqual(Core::float32 a, Core::float32 b)
	{
		return IsZero(a - b);
	}

	/**
	 * @brief 두 부동소수점 값이 거의 같은지 검사 (커스텀 epsilon)
	 * @param a 첫 번째 값
	 * @param b 두 번째 값
	 * @param epsilon 비교 임계값
	 * @return 거의 같으면 true
	 */
	inline bool IsEqual(Core::float32 a, Core::float32 b, Core::float32 epsilon)
	{
		return IsZero(a - b, epsilon);
	}

	//=============================================================================
	// Vector2 유틸리티 함수
	//=============================================================================

	inline Vector2 Add(const Vector2& a, const Vector2& b) { return a + b; }
	inline Vector2 Subtract(const Vector2& a, const Vector2& b) { return a - b; }
	inline Vector2 Multiply(const Vector2& v, Core::float32 scalar) { return v * scalar; }
	inline Core::float32 Dot(const Vector2& a, const Vector2& b) { return a.Dot(b); }
	inline Core::float32 Length(const Vector2& v) { return v.Length(); }
	inline Core::float32 LengthSquared(const Vector2& v) { return v.LengthSquared(); }
	inline Vector2 Normalize(const Vector2& v) { return v.Normalized(); }

	inline Core::float32 Distance(const Vector2& a, const Vector2& b)
	{
		return (b - a).Length();
	}

	inline Vector2 Lerp(const Vector2& a, const Vector2& b, Core::float32 t)
	{
		return Vector2::FromSIMD(DirectX::XMVectorLerp(a.ToSIMD(), b.ToSIMD(), t));
	}

	/// @brief 2D 벡터의 수직 벡터 반환 (반시계 방향 90도 회전)
	inline Vector2 Perpendicular(const Vector2& v)
	{
		return Vector2(-v.y, v.x);
	}

	/// @brief 2D 외적 (z 성분 반환, 면적 계산 등에 사용)
	inline Core::float32 Cross2D(const Vector2& a, const Vector2& b)
	{
		return a.x * b.y - a.y * b.x;
	}

	//=============================================================================
	// Vector3 유틸리티 함수
	//=============================================================================

	inline Vector3 Add(const Vector3& a, const Vector3& b) { return a + b; }
	inline Vector3 Subtract(const Vector3& a, const Vector3& b) { return a - b; }
	inline Vector3 Multiply(const Vector3& v, Core::float32 scalar) { return v * scalar; }
	inline Core::float32 Dot(const Vector3& a, const Vector3& b) { return a.Dot(b); }
	inline Vector3 Cross(const Vector3& a, const Vector3& b) { return a.Cross(b); }
	inline Core::float32 Length(const Vector3& v) { return v.Length(); }
	inline Core::float32 LengthSquared(const Vector3& v) { return v.LengthSquared(); }
	inline Vector3 Normalize(const Vector3& v) { return v.Normalized(); }

	inline Core::float32 Distance(const Vector3& a, const Vector3& b)
	{
		return (b - a).Length();
	}

	inline Vector3 Lerp(const Vector3& a, const Vector3& b, Core::float32 t)
	{
		return Vector3::FromSIMD(DirectX::XMVectorLerp(a.ToSIMD(), b.ToSIMD(), t));
	}

	/// @brief 벡터 반사 (입사 벡터와 법선으로 반사 벡터 계산)
	inline Vector3 Reflect(const Vector3& incident, const Vector3& normal)
	{
		return Vector3::FromSIMD(DirectX::XMVector3Reflect(incident.ToSIMD(), normal.ToSIMD()));
	}

	/// @brief 벡터를 평면에 투영
	inline Vector3 ProjectOnPlane(const Vector3& v, const Vector3& planeNormal)
	{
		return v - planeNormal * v.Dot(planeNormal);
	}

	/// @brief 벡터를 다른 벡터에 투영
	inline Vector3 ProjectOnVector(const Vector3& v, const Vector3& target)
	{
		Core::float32 dot = v.Dot(target);
		Core::float32 lenSq = target.LengthSquared();
		if (lenSq > EPSILON)
		{
			return target * (dot / lenSq);
		}
		return Vector3::Zero();
	}

	/// @brief 두 벡터 사이의 각도 (라디안)
	inline Core::float32 AngleBetween(const Vector3& a, const Vector3& b)
	{
		VectorSIMD angle = DirectX::XMVector3AngleBetweenVectors(a.ToSIMD(), b.ToSIMD());
		return DirectX::XMVectorGetX(angle);
	}

	inline bool IsUniformScale(const Math::Vector3& scale)
	{
		// 1. 가장 흔한 케이스 (1,1,1) 빠른 탈출
		if (scale.x == 1.0f && scale.y == 1.0f && scale.z == 1.0f)
		{
			return true;
		}

		// 일반적인 균등 스케일 체크
		float deltaXY = std::abs(scale.x - scale.y);
		float deltaXZ = std::abs(scale.x - scale.z);

		return (deltaXY < Math::EPSILON) && (deltaXZ < Math::EPSILON);
	}

	//=============================================================================
	// Vector4 유틸리티 함수
	//=============================================================================

	inline Vector4 Add(const Vector4& a, const Vector4& b) { return a + b; }
	inline Vector4 Subtract(const Vector4& a, const Vector4& b) { return a - b; }
	inline Vector4 Multiply(const Vector4& v, Core::float32 scalar) { return v * scalar; }
	inline Core::float32 Dot(const Vector4& a, const Vector4& b) { return a.Dot(b); }
	inline Core::float32 Length(const Vector4& v) { return v.Length(); }
	inline Core::float32 LengthSquared(const Vector4& v) { return v.LengthSquared(); }
	inline Vector4 Normalize(const Vector4& v) { return v.Normalized(); }

	inline Vector4 Lerp(const Vector4& a, const Vector4& b, Core::float32 t)
	{
		return Vector4::FromSIMD(DirectX::XMVectorLerp(a.ToSIMD(), b.ToSIMD(), t));
	}

	//=============================================================================
	// 행렬 연산
	//=============================================================================

	inline Matrix4x4 MatrixMultiply(const Matrix4x4& a, const Matrix4x4& b)
	{
		return a * b;
	}

	inline Matrix4x4 MatrixInverse(const Matrix4x4& m)
	{
		VectorSIMD det;
		MatrixSIMD result = DirectX::XMMatrixInverse(&det, m.ToSIMD());
		return Matrix4x4::FromSIMD(result);
	}

	inline Matrix4x4 MatrixTranspose(const Matrix4x4& m)
	{
		return Matrix4x4::FromSIMD(DirectX::XMMatrixTranspose(m.ToSIMD()));
	}

	inline Matrix4x4 MatrixIdentity()
	{
		return Matrix4x4::Identity();
	}

	/// @brief 역행렬 계산과 함께 행렬식 반환
	inline Matrix4x4 MatrixInverseWithDeterminant(const Matrix4x4& m, Core::float32& outDeterminant)
	{
		VectorSIMD det;
		MatrixSIMD result = DirectX::XMMatrixInverse(&det, m.ToSIMD());
		outDeterminant = DirectX::XMVectorGetX(det);
		return Matrix4x4::FromSIMD(result);
	}

	/// @brief 행렬식 계산
	inline Core::float32 MatrixDeterminant(const Matrix4x4& m)
	{
		VectorSIMD det = DirectX::XMMatrixDeterminant(m.ToSIMD());
		return DirectX::XMVectorGetX(det);
	}

	//=============================================================================
	// 변환 행렬
	//=============================================================================

	inline Matrix4x4 MatrixTranslation(Core::float32 x, Core::float32 y, Core::float32 z)
	{
		return Matrix4x4::FromSIMD(DirectX::XMMatrixTranslation(x, y, z));
	}

	inline Matrix4x4 MatrixTranslation(const Vector3& v)
	{
		return MatrixTranslation(v.x, v.y, v.z);
	}

	inline Matrix4x4 MatrixRotationX(Core::float32 angle)
	{
		return Matrix4x4::FromSIMD(DirectX::XMMatrixRotationX(angle));
	}

	inline Matrix4x4 MatrixRotationY(Core::float32 angle)
	{
		return Matrix4x4::FromSIMD(DirectX::XMMatrixRotationY(angle));
	}

	inline Matrix4x4 MatrixRotationZ(Core::float32 angle)
	{
		return Matrix4x4::FromSIMD(DirectX::XMMatrixRotationZ(angle));
	}

	/**
	 * @brief Pitch, Yaw, Roll 복합 회전 행렬 생성 (라디안)
	 *
	 * 회전 적용 순서: Roll(Z) -> Pitch(X) -> Yaw(Y)
	 */
	inline Matrix4x4 MatrixRotationRollPitchYaw(Core::float32 pitch, Core::float32 yaw, Core::float32 roll)
	{
		return Matrix4x4::FromSIMD(DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll));
	}

	inline Matrix4x4 MatrixRotationRollPitchYaw(const Vector3& angles)
	{
		return MatrixRotationRollPitchYaw(angles.x, angles.y, angles.z);
	}

	inline Matrix4x4 MatrixScaling(Core::float32 x, Core::float32 y, Core::float32 z)
	{
		return Matrix4x4::FromSIMD(DirectX::XMMatrixScaling(x, y, z));
	}

	inline Matrix4x4 MatrixScaling(const Vector3& v)
	{
		return MatrixScaling(v.x, v.y, v.z);
	}

	inline Matrix4x4 MatrixScaling(Core::float32 uniformScale)
	{
		return MatrixScaling(uniformScale, uniformScale, uniformScale);
	}

	//=============================================================================
	// 쿼터니언 연산
	//=============================================================================

	inline Quaternion QuaternionIdentity()
	{
		return Quaternion::Identity();
	}

	inline Quaternion QuaternionFromEuler(Core::float32 pitch, Core::float32 yaw, Core::float32 roll)
	{
		VectorSIMD q = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
		return Quaternion::FromSIMD(q);
	}

	inline Quaternion QuaternionFromEuler(const Vector3& eulerAngles)
	{
		return QuaternionFromEuler(eulerAngles.x, eulerAngles.y, eulerAngles.z);
	}

	inline Quaternion QuaternionFromAxisAngle(const Vector3& axis, Core::float32 angle)
	{
		VectorSIMD q = DirectX::XMQuaternionRotationAxis(axis.ToSIMD(), angle);
		return Quaternion::FromSIMD(q);
	}

	inline Quaternion QuaternionMultiply(const Quaternion& a, const Quaternion& b)
	{
		return a * b;
	}

	inline Quaternion QuaternionNormalize(const Quaternion& q)
	{
		return q.Normalized();
	}

	inline Quaternion QuaternionConjugate(const Quaternion& q)
	{
		return q.Conjugate();
	}

	inline Quaternion QuaternionInverse(const Quaternion& q)
	{
		return q.Inverse();
	}

	inline Quaternion QuaternionSlerp(const Quaternion& a, const Quaternion& b, Core::float32 t)
	{
		VectorSIMD result = DirectX::XMQuaternionSlerp(a.ToSIMD(), b.ToSIMD(), t);
		return Quaternion::FromSIMD(result);
	}

	inline Matrix4x4 MatrixRotationQuaternion(const Quaternion& q)
	{
		return Matrix4x4::FromSIMD(DirectX::XMMatrixRotationQuaternion(q.ToSIMD()));
	}

	inline Vector3 Vector3RotateByQuaternion(const Vector3& v, const Quaternion& q)
	{
		VectorSIMD result = DirectX::XMVector3Rotate(v.ToSIMD(), q.ToSIMD());
		return Vector3::FromSIMD(result);
	}

	/**
	 * @brief Quaternion에서 Euler 각도 추출
	 * @return Vector3(pitch, yaw, roll) 라디안 단위
	 */
	inline Vector3 Vector3EulerFromQuaternion(const Quaternion& q)
	{
		Matrix4x4 mat = MatrixRotationQuaternion(q);

		Core::float32 pitch, yaw, roll;
		Core::float32 sinPitch = -mat.m[1][2];

		// Gimbal Lock 체크
		if (std::abs(sinPitch) >= 0.9999f)
		{
			pitch = std::copysign(HALF_PI, sinPitch);
			yaw = std::atan2(-mat.m[0][1], mat.m[0][0]);
			roll = 0.0f;
		}
		else
		{
			pitch = std::asin(sinPitch);
			yaw = std::atan2(mat.m[0][2], mat.m[2][2]);
			roll = std::atan2(mat.m[1][0], mat.m[1][1]);
		}

		return Vector3(pitch, yaw, roll);
	}

	/// @brief Quaternion에서 Forward 방향 벡터 추출
	inline Vector3 Vector3ForwardFromQuaternion(const Quaternion& q)
	{
		return Vector3RotateByQuaternion(Vector3::Forward(), q);
	}

	/// @brief Quaternion에서 Up 방향 벡터 추출
	inline Vector3 Vector3UpFromQuaternion(const Quaternion& q)
	{
		return Vector3RotateByQuaternion(Vector3::Up(), q);
	}

	/// @brief Quaternion에서 Right 방향 벡터 추출
	inline Vector3 Vector3RightFromQuaternion(const Quaternion& q)
	{
		return Vector3RotateByQuaternion(Vector3::Right(), q);
	}

	/// @brief 회전 행렬에서 Quaternion 생성
	inline Quaternion QuaternionFromRotationMatrix(const Matrix4x4& m)
	{
		VectorSIMD q = DirectX::XMQuaternionRotationMatrix(m.ToSIMD());
		return Quaternion::FromSIMD(q);
	}

	/// @brief 두 방향 벡터 사이의 회전을 나타내는 Quaternion 생성
	inline Quaternion QuaternionFromToRotation(const Vector3& from, const Vector3& to)
	{
		Vector3 fromNorm = from.Normalized();
		Vector3 toNorm = to.Normalized();

		Core::float32 dot = fromNorm.Dot(toNorm);

		// 거의 같은 방향
		if (dot > 0.9999f)
		{
			return Quaternion::Identity();
		}

		// 거의 반대 방향
		if (dot < -0.9999f)
		{
			// 임의의 수직 축 찾기
			Vector3 axis = Vector3::UnitX().Cross(fromNorm);
			if (axis.LengthSquared() < EPSILON)
			{
				axis = Vector3::UnitY().Cross(fromNorm);
			}
			return QuaternionFromAxisAngle(axis.Normalized(), PI);
		}

		Vector3 axis = fromNorm.Cross(toNorm);
		Core::float32 angle = std::acos(dot);
		return QuaternionFromAxisAngle(axis.Normalized(), angle);
	}

	/// @brief LookAt 방향을 향하는 Quaternion 생성
	inline Quaternion QuaternionLookAt(const Vector3& forward, const Vector3& up = Vector3::Up())
	{
		Vector3 fwd = forward.Normalized();
		Vector3 right = up.Cross(fwd).Normalized();
		Vector3 upVec = fwd.Cross(right);

		Matrix4x4 rotMat = Matrix4x4::Identity();
		rotMat.m[0][0] = right.x;  rotMat.m[0][1] = right.y;  rotMat.m[0][2] = right.z;
		rotMat.m[1][0] = upVec.x;  rotMat.m[1][1] = upVec.y;  rotMat.m[1][2] = upVec.z;
		rotMat.m[2][0] = fwd.x;    rotMat.m[2][1] = fwd.y;    rotMat.m[2][2] = fwd.z;

		return QuaternionFromRotationMatrix(rotMat);
	}

	//=============================================================================
	// 카메라 & 투영
	//=============================================================================

	inline Matrix4x4 MatrixLookAtLH(const Vector3& eye, const Vector3& target, const Vector3& up)
	{
		return Matrix4x4::FromSIMD(
			DirectX::XMMatrixLookAtLH(eye.ToSIMD(), target.ToSIMD(), up.ToSIMD())
		);
	}

	inline Matrix4x4 MatrixLookToLH(const Vector3& eye, const Vector3& direction, const Vector3& up)
	{
		return Matrix4x4::FromSIMD(
			DirectX::XMMatrixLookToLH(eye.ToSIMD(), direction.ToSIMD(), up.ToSIMD())
		);
	}

	inline Matrix4x4 MatrixPerspectiveFovLH(
		Core::float32 fovY,
		Core::float32 aspect,
		Core::float32 nearZ,
		Core::float32 farZ
	)
	{
		return Matrix4x4::FromSIMD(
			DirectX::XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ)
		);
	}

	inline Matrix4x4 MatrixOrthographicLH(
		Core::float32 width,
		Core::float32 height,
		Core::float32 nearZ,
		Core::float32 farZ
	)
	{
		return Matrix4x4::FromSIMD(
			DirectX::XMMatrixOrthographicLH(width, height, nearZ, farZ)
		);
	}

	inline Matrix4x4 MatrixOrthographicOffCenterLH(
		Core::float32 left,
		Core::float32 right,
		Core::float32 bottom,
		Core::float32 top,
		Core::float32 nearZ,
		Core::float32 farZ
	)
	{
		return Matrix4x4::FromSIMD(
			DirectX::XMMatrixOrthographicOffCenterLH(left, right, bottom, top, nearZ, farZ)
		);
	}

	//=============================================================================
	// 벡터/행렬 변환
	//=============================================================================

	/// @brief 축 기반 회전 행렬 생성
	inline Matrix4x4 MatrixRotationAxis(const Vector3& axis, Core::float32 angle)
	{
		return Matrix4x4::FromSIMD(DirectX::XMMatrixRotationAxis(axis.ToSIMD(), angle));
	}

	/// @brief Normal 벡터 변환 (이동 무시, 회전/스케일만 적용)
	inline Vector3 Vector3TransformNormal(const Vector3& v, const Matrix4x4& m)
	{
		VectorSIMD result = DirectX::XMVector3TransformNormal(v.ToSIMD(), m.ToSIMD());
		return Vector3::FromSIMD(result);
	}

	/// @brief 점 변환 (이동 포함)
	inline Vector3 Vector3TransformCoord(const Vector3& v, const Matrix4x4& m)
	{
		VectorSIMD result = DirectX::XMVector3TransformCoord(v.ToSIMD(), m.ToSIMD());
		return Vector3::FromSIMD(result);
	}

	/// @brief Vector4 변환
	inline Vector4 Vector4Transform(const Vector4& v, const Matrix4x4& m)
	{
		return m * v;
	}

	//=============================================================================
	// SRT 분해/합성
	//=============================================================================

	/**
	 * @brief Scale, Rotation, Translation으로 변환 행렬 생성
	 *
	 * 적용 순서: Scale -> Rotation -> Translation
	 */
	inline Matrix4x4 MatrixSRT(const Vector3& scale, const Quaternion& rotation, const Vector3& translation)
	{
		Matrix4x4 s = MatrixScaling(scale);
		Matrix4x4 r = MatrixRotationQuaternion(rotation);
		Matrix4x4 t = MatrixTranslation(translation);
		return s * r * t;
	}

	/**
	 * @brief 변환 행렬에서 Scale, Rotation, Translation 분해
	 * @return 분해 성공 시 true
	 */
	inline bool MatrixDecompose(
		const Matrix4x4& m,
		Vector3& outScale,
		Quaternion& outRotation,
		Vector3& outTranslation
	)
	{
		VectorSIMD scale, rotQuat, trans;
		if (DirectX::XMMatrixDecompose(&scale, &rotQuat, &trans, m.ToSIMD()))
		{
			outScale = Vector3::FromSIMD(scale);
			outRotation = Quaternion::FromSIMD(rotQuat);
			outTranslation = Vector3::FromSIMD(trans);
			return true;
		}
		return false;
	}

	//=============================================================================
	// 유틸리티 함수
	//=============================================================================

	inline Core::float32 InverseSqrt(Core::float32 value)
	{
		return 1.0f / std::sqrt(value);
	}

	inline Core::float32 Lerp(Core::float32 a, Core::float32 b, Core::float32 t)
	{
		return a + (b - a) * t;
	}

	inline Core::float32 Clamp(Core::float32 value, Core::float32 min, Core::float32 max)
	{
		return (value < min) ? min : (value > max) ? max : value;
	}

	template<typename T>
	inline T Clamp(T value, T min, T max)
	{
		return (value < min) ? min : (value > max) ? max : value;
	}

	inline Core::float32 DegToRad(Core::float32 degrees)
	{
		return degrees * DEG_TO_RAD;
	}

	inline Core::float32 RadToDeg(Core::float32 radians)
	{
		return radians * RAD_TO_DEG;
	}

	/// @brief 값을 0~1 범위로 제한
	inline Core::float32 Saturate(Core::float32 value)
	{
		return Clamp(value, 0.0f, 1.0f);
	}

	/// @brief 두 값 사이에서 t의 위치 비율 반환 (Lerp의 역함수)
	inline Core::float32 InverseLerp(Core::float32 a, Core::float32 b, Core::float32 value)
	{
		if (std::abs(b - a) < EPSILON)
		{
			return 0.0f;
		}
		return (value - a) / (b - a);
	}

	/// @brief 한 범위의 값을 다른 범위로 매핑
	inline Core::float32 Remap(
		Core::float32 value,
		Core::float32 fromMin,
		Core::float32 fromMax,
		Core::float32 toMin,
		Core::float32 toMax
	)
	{
		Core::float32 t = InverseLerp(fromMin, fromMax, value);
		return Lerp(toMin, toMax, t);
	}

	/// @brief Smoothstep 보간 (부드러운 시작/끝)
	inline Core::float32 SmoothStep(Core::float32 edge0, Core::float32 edge1, Core::float32 x)
	{
		Core::float32 t = Saturate(InverseLerp(edge0, edge1, x));
		return t * t * (3.0f - 2.0f * t);
	}

	/// @brief 부호 반환 (-1, 0, 1)
	inline Core::float32 Sign(Core::float32 value)
	{
		if (value > EPSILON)
		{
			return 1.0f;
		}
		if (value < -EPSILON)
		{
			return -1.0f;
		}
		return 0.0f;
	}

	/// @brief 최소값
	inline Core::float32 Min(Core::float32 a, Core::float32 b)
	{
		return (a < b) ? a : b;
	}

	/// @brief 최대값
	inline Core::float32 Max(Core::float32 a, Core::float32 b)
	{
		return (a > b) ? a : b;
	}

	/// @brief 절대값
	inline Core::float32 Abs(Core::float32 value)
	{
		return std::abs(value);
	}

} // namespace Math
