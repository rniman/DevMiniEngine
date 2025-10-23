#pragma once

#include "Math/MathTypes.h"

namespace Math 
{
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

    inline Vector3 Multiply(const Vector3& v, float scalar)
    {
        VectorSIMD vec = LoadVector3(v);
        VectorSIMD result = DirectX::XMVectorScale(vec, scalar);
        Vector3 out;
        StoreVector3(out, result);
        return out;
    }

    inline float Dot(const Vector3& a, const Vector3& b)
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

    inline float Length(const Vector3& v)
    {
        VectorSIMD vec = LoadVector3(v);
        VectorSIMD result = DirectX::XMVector3Length(vec);
        return DirectX::XMVectorGetX(result);
    }

    inline float LengthSquared(const Vector3& v)
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

    inline float Distance(const Vector3& a, const Vector3& b)
    {
        return Length(Subtract(b, a));
    }

    inline Vector3 Lerp(const Vector3& a, const Vector3& b, float t)
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

    inline Matrix4x4 MatrixTranslation(float x, float y, float z)
    {
        Matrix4x4 out;
        StoreMatrix(out, DirectX::XMMatrixTranslation(x, y, z));
        return out;
    }

    inline Matrix4x4 MatrixTranslation(const Vector3& v)
    {
        return MatrixTranslation(v.x, v.y, v.z);
    }

    inline Matrix4x4 MatrixRotationX(float angle)
    {
        Matrix4x4 out;
        StoreMatrix(out, DirectX::XMMatrixRotationX(angle));
        return out;
    }

    inline Matrix4x4 MatrixRotationY(float angle)
    {
        Matrix4x4 out;
        StoreMatrix(out, DirectX::XMMatrixRotationY(angle));
        return out;
    }

    inline Matrix4x4 MatrixRotationZ(float angle)
    {
        Matrix4x4 out;
        StoreMatrix(out, DirectX::XMMatrixRotationZ(angle));
        return out;
    }

    // Pitch, Yaw, Roll 복합 회전 (라디안)
    // 회전 적용 순서: Y(Yaw) → X(Pitch) → Z(Roll)
    inline Matrix4x4 MatrixRotationRollPitchYaw(float pitch, float yaw, float roll)
    {
        Matrix4x4 out;
        StoreMatrix(out, DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll));
        return out;
    }

    inline Matrix4x4 MatrixRotationRollPitchYaw(const Vector3& angles)
    {
        return MatrixRotationRollPitchYaw(angles.x, angles.y, angles.z);
    }

    inline Matrix4x4 MatrixScaling(float x, float y, float z)
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

    inline Quaternion QuaternionFromEuler(float pitch, float yaw, float roll)
    {
        VectorSIMD q = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
        Quaternion out;
        DirectX::XMStoreFloat4(&out, q);
        return out;
    }

    inline Quaternion QuaternionFromAxisAngle(const Vector3& axis, float angle)
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

    inline Quaternion QuaternionSlerp(const Quaternion& a, const Quaternion& b, float t)
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

    inline Matrix4x4 MatrixPerspectiveFovLH(float fovY, float aspect, float nearZ, float farZ)
    {
        Matrix4x4 out;
        StoreMatrix(out, DirectX::XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ));
        return out;
    }

    inline Matrix4x4 MatrixOrthographicLH(float width, float height, float nearZ, float farZ)
    {
        Matrix4x4 out;
        StoreMatrix(out, DirectX::XMMatrixOrthographicLH(width, height, nearZ, farZ));
        return out;
    }

    //=============================================================================
    // 유틸리티 함수
    //=============================================================================

    inline float Clamp(float value, float min, float max)
    {
        return (value < min) ? min : (value > max) ? max : value;
    }

    inline float DegToRad(float degrees)
    {
        return degrees * DEG_TO_RAD;
    }

    inline float RadToDeg(float radians)
    {
        return radians * RAD_TO_DEG;
    }

} // namespace Math