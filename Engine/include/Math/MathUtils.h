#pragma once

#include "Core/Types.h"
#include "Math/MathTypes.h"

namespace Math 
{
    //=============================================================================
    // 부동소수점 비교 상수
    //=============================================================================

    /// @brief 기본 부동소수점 비교 임계값
    constexpr Core::float32 EPSILON = 1e-6f;

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
        return (fabsf(value) < EPSILON);
    }

    /**
     * @brief 부동소수점 값이 0에 가까운지 검사 (커스텀 epsilon)
     * @param value 검사할 값
     * @param epsilon 비교 임계값
     * @return 0에 가까우면 true
     */
    inline bool IsZero(Core::float32 value, Core::float32 epsilon)
    {
        return (fabsf(value) < epsilon);
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
    // 벡터 연산
    //=============================================================================

    inline Vector3 Add(const Vector3& a, const Vector3& b)
    {
        VectorSIMD va = LoadVector3(a);
        VectorSIMD vb = LoadVector3(b);
        VectorSIMD result = DirectX::XMVectorAdd(va, vb);
        Vector3 out;
        StoreVector3(out, result);
        return out;
    }

    inline Vector3 Subtract(const Vector3& a, const Vector3& b)
    {
        VectorSIMD va = LoadVector3(a);
        VectorSIMD vb = LoadVector3(b);
        VectorSIMD result = DirectX::XMVectorSubtract(va, vb);
        Vector3 out;
        StoreVector3(out, result);
        return out;
    }

    inline Vector3 Multiply(const Vector3& v, Core::float32 scalar)
    {
        VectorSIMD vec = LoadVector3(v);
        VectorSIMD result = DirectX::XMVectorScale(vec, scalar);
        Vector3 out;
        StoreVector3(out, result);
        return out;
    }

    inline Core::float32 Dot(const Vector3& a, const Vector3& b)
    {
        VectorSIMD va = LoadVector3(a);
        VectorSIMD vb = LoadVector3(b);
        VectorSIMD result = DirectX::XMVector3Dot(va, vb);
        return DirectX::XMVectorGetX(result);
    }

    inline Vector3 Cross(const Vector3& a, const Vector3& b)
    {
        VectorSIMD va = LoadVector3(a);
        VectorSIMD vb = LoadVector3(b);
        VectorSIMD result = DirectX::XMVector3Cross(va, vb);
        Vector3 out;
        StoreVector3(out, result);
        return out;
    }

    inline Core::float32 Length(const Vector3& v)
    {
        VectorSIMD vec = LoadVector3(v);
        VectorSIMD result = DirectX::XMVector3Length(vec);
        return DirectX::XMVectorGetX(result);
    }

    inline Core::float32 LengthSquared(const Vector3& v)
    {
        VectorSIMD vec = LoadVector3(v);
        VectorSIMD result = DirectX::XMVector3LengthSq(vec);
        return DirectX::XMVectorGetX(result);
    }

    inline Vector3 Normalize(const Vector3& v)
    {
        VectorSIMD vec = LoadVector3(v);
        VectorSIMD result = DirectX::XMVector3Normalize(vec);
        Vector3 out;
        StoreVector3(out, result);
        return out;
    }

    inline Core::float32 Distance(const Vector3& a, const Vector3& b)
    {
        return Length(Subtract(b, a));
    }

    inline Vector3 Lerp(const Vector3& a, const Vector3& b, Core::float32 t)
    {
        VectorSIMD va = LoadVector3(a);
        VectorSIMD vb = LoadVector3(b);
        VectorSIMD result = DirectX::XMVectorLerp(va, vb, t);
        Vector3 out;
        StoreVector3(out, result);
        return out;
    }

    //=============================================================================
    // 행렬 연산
    //=============================================================================

    inline Matrix4x4 MatrixMultiply(const Matrix4x4& a, const Matrix4x4& b)
    {
        MatrixSIMD ma = LoadMatrix(a);
        MatrixSIMD mb = LoadMatrix(b);
        MatrixSIMD result = DirectX::XMMatrixMultiply(ma, mb);
        Matrix4x4 out;
        StoreMatrix(out, result);
        return out;
    }

    inline Matrix4x4 MatrixInverse(const Matrix4x4& m)
    {
        MatrixSIMD mat = LoadMatrix(m);
        VectorSIMD det;
        MatrixSIMD result = DirectX::XMMatrixInverse(&det, mat);
        Matrix4x4 out;
        StoreMatrix(out, result);
        return out;
    }

    inline Matrix4x4 MatrixTranspose(const Matrix4x4& m)
    {
        MatrixSIMD mat = LoadMatrix(m);
        MatrixSIMD result = DirectX::XMMatrixTranspose(mat);
        Matrix4x4 out;
        StoreMatrix(out, result);
        return out;
    }

    inline Matrix4x4 MatrixIdentity()
    {
        Matrix4x4 out;
        StoreMatrix(out, DirectX::XMMatrixIdentity());
        return out;
    }

    //=============================================================================
    // 변환 행렬
    //=============================================================================

    inline Matrix4x4 MatrixTranslation(Core::float32 x, Core::float32 y, Core::float32 z)
    {
        Matrix4x4 out;
        StoreMatrix(out, DirectX::XMMatrixTranslation(x, y, z));
        return out;
    }

    inline Matrix4x4 MatrixTranslation(const Vector3& v)
    {
        return MatrixTranslation(v.x, v.y, v.z);
    }

    inline Matrix4x4 MatrixRotationX(Core::float32 angle)
    {
        Matrix4x4 out;
        StoreMatrix(out, DirectX::XMMatrixRotationX(angle));
        return out;
    }

    inline Matrix4x4 MatrixRotationY(Core::float32 angle)
    {
        Matrix4x4 out;
        StoreMatrix(out, DirectX::XMMatrixRotationY(angle));
        return out;
    }

    inline Matrix4x4 MatrixRotationZ(Core::float32 angle)
    {
        Matrix4x4 out;
        StoreMatrix(out, DirectX::XMMatrixRotationZ(angle));
        return out;
    }

    // Pitch, Yaw, Roll 복합 회전 (라디안)
    // 회전 적용 순서: Y(Yaw) → X(Pitch) → Z(Roll)
    inline Matrix4x4 MatrixRotationRollPitchYaw(Core::float32 pitch, Core::float32 yaw, Core::float32 roll)
    {
        Matrix4x4 out;
        StoreMatrix(out, DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll));
        return out;
    }

    inline Matrix4x4 MatrixRotationRollPitchYaw(const Vector3& angles)
    {
        return MatrixRotationRollPitchYaw(angles.x, angles.y, angles.z);
    }

    inline Matrix4x4 MatrixScaling(Core::float32 x, Core::float32 y, Core::float32 z)
    {
        Matrix4x4 out;
        StoreMatrix(out, DirectX::XMMatrixScaling(x, y, z));
        return out;
    }

    inline Matrix4x4 MatrixScaling(const Vector3& v)
    {
        return MatrixScaling(v.x, v.y, v.z);
    }

    //=============================================================================
    // 쿼터니언 연산
    //=============================================================================

    inline Quaternion QuaternionIdentity()
    {
        Quaternion out;
        DirectX::XMStoreFloat4(&out, DirectX::XMQuaternionIdentity());
        return out;
    }

    inline Quaternion QuaternionFromEuler(Core::float32 pitch, Core::float32 yaw, Core::float32 roll)
    {
        VectorSIMD q = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
        Quaternion out;
        DirectX::XMStoreFloat4(&out, q);
        return out;
    }

    inline Quaternion QuaternionFromAxisAngle(const Vector3& axis, Core::float32 angle)
    {
        VectorSIMD axisVec = LoadVector3(axis);
        VectorSIMD q = DirectX::XMQuaternionRotationAxis(axisVec, angle);
        Quaternion out;
        DirectX::XMStoreFloat4(&out, q);
        return out;
    }

    inline Quaternion QuaternionMultiply(const Quaternion& a, const Quaternion& b)
    {
        VectorSIMD qa = DirectX::XMLoadFloat4(&a);
        VectorSIMD qb = DirectX::XMLoadFloat4(&b);
        VectorSIMD result = DirectX::XMQuaternionMultiply(qa, qb);
        Quaternion out;
        DirectX::XMStoreFloat4(&out, result);
        return out;
    }

    inline Quaternion QuaternionNormalize(const Quaternion& q)
    {
        VectorSIMD quat = DirectX::XMLoadFloat4(&q);
        VectorSIMD result = DirectX::XMQuaternionNormalize(quat);
        Quaternion out;
        DirectX::XMStoreFloat4(&out, result);
        return out;
    }

    inline Quaternion QuaternionSlerp(const Quaternion& a, const Quaternion& b, Core::float32 t)
    {
        VectorSIMD qa = DirectX::XMLoadFloat4(&a);
        VectorSIMD qb = DirectX::XMLoadFloat4(&b);
        VectorSIMD result = DirectX::XMQuaternionSlerp(qa, qb, t);
        Quaternion out;
        DirectX::XMStoreFloat4(&out, result);
        return out;
    }

    inline Matrix4x4 MatrixRotationQuaternion(const Quaternion& q)
    {
        VectorSIMD quat = DirectX::XMLoadFloat4(&q);
        Matrix4x4 out;
        StoreMatrix(out, DirectX::XMMatrixRotationQuaternion(quat));
        return out;
    }

    inline Vector3 QuaternionRotateVector(const Quaternion& q, const Vector3& v)
    {
        VectorSIMD quat = DirectX::XMLoadFloat4(&q);
        VectorSIMD vec = LoadVector3(v);
        VectorSIMD result = DirectX::XMVector3Rotate(vec, quat);
        Vector3 out;
        StoreVector3(out, result);
        return out;
    }

    //=============================================================================
    // 카메라 & 투영
    //=============================================================================

    inline Matrix4x4 MatrixLookAtLH(const Vector3& eye, const Vector3& target, const Vector3& up)
    {
        VectorSIMD eyeVec = LoadVector3(eye);
        VectorSIMD targetVec = LoadVector3(target);
        VectorSIMD upVec = LoadVector3(up);
        Matrix4x4 out;
        StoreMatrix(out, DirectX::XMMatrixLookAtLH(eyeVec, targetVec, upVec));
        return out;
    }

    inline Matrix4x4 MatrixLookToLH(const Vector3& eye, const Vector3& direction, const Vector3& up)
    {
        VectorSIMD eyeVec = LoadVector3(eye);
        VectorSIMD dirVec = LoadVector3(direction);
        VectorSIMD upVec = LoadVector3(up);
        Matrix4x4 out;
        StoreMatrix(out, DirectX::XMMatrixLookToLH(eyeVec, dirVec, upVec));
        return out;
    }

    inline Matrix4x4 MatrixPerspectiveFovLH(Core::float32 fovY, Core::float32 aspect, Core::float32 nearZ, Core::float32 farZ)
    {
        Matrix4x4 out;
        StoreMatrix(out, DirectX::XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ));
        return out;
    }

    inline Matrix4x4 MatrixOrthographicLH(Core::float32 width, Core::float32 height, Core::float32 nearZ, Core::float32 farZ)
    {
        Matrix4x4 out;
        StoreMatrix(out, DirectX::XMMatrixOrthographicLH(width, height, nearZ, farZ));
        return out;
    }

    //=============================================================================
    // 벡터/행렬 변환
    //=============================================================================

    // 축 기반 회전 행렬 생성 (Vector3 버전)
    inline Matrix4x4 MatrixRotationAxis(const Vector3& axis, Core::float32 angle)
    {
        VectorSIMD axisVec = LoadVector3(axis);
        Matrix4x4 out;
        StoreMatrix(out, DirectX::XMMatrixRotationAxis(axisVec, angle));
        return out;
    }

    // 벡터를 행렬로 변환 (Normal 변환용)
    inline Vector3 Vector3TransformNormal(const Vector3& v, const Matrix4x4& m)
    {
        VectorSIMD vec = LoadVector3(v);
        MatrixSIMD mat = LoadMatrix(m);
        VectorSIMD result = DirectX::XMVector3TransformNormal(vec, mat);
        Vector3 out;
        StoreVector3(out, result);
        return out;
    }

    //=============================================================================
    // 유틸리티 함수
    //=============================================================================

    inline Core::float32 InverseSqrt(Core::float32 value)
    {
        return 1.0f / sqrtf(value);
    }

    inline Core::float32 Lerp(Core::float32 a, Core::float32 b, Core::float32 t)
    {
        return a + (b - a) * t;
    }

    inline Core::float32 Clamp(Core::float32 value, Core::float32 min, Core::float32 max)
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

} // namespace Math