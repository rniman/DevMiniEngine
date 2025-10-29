#pragma once

#include <DirectXMath.h>
#include "Core/Types.h"

namespace Math
{
    //=============================================================================
    // 벡터 타입
    //=============================================================================

    // 저장용 벡터 타입 (메모리 효율적)
    using Vector2 = DirectX::XMFLOAT2;  // 2D 벡터 (8 bytes)
    using Vector3 = DirectX::XMFLOAT3;  // 3D 벡터 (12 bytes)
    using Vector4 = DirectX::XMFLOAT4;  // 4D 벡터 (16 bytes)

    // SIMD 계산용 벡터 (16 bytes aligned, 연산 최적화)
    using VectorSIMD = DirectX::XMVECTOR;

    //=============================================================================
    // 행렬 타입
    //=============================================================================

    // 저장용 행렬 타입
    using Matrix3x3 = DirectX::XMFLOAT3X3;  // 3x3 행렬 (36 bytes)
    using Matrix4x4 = DirectX::XMFLOAT4X4;  // 4x4 행렬 (64 bytes)

    // SIMD 계산용 행렬 (연산 최적화)
    using MatrixSIMD = DirectX::XMMATRIX;

    //=============================================================================
    // 쿼터니언
    //=============================================================================

    // 저장용 쿼터니언 (회전 표현)
    using Quaternion = DirectX::XMFLOAT4;

    // SIMD 계산용 쿼터니언
    using QuaternionSIMD = DirectX::XMVECTOR;

    //=============================================================================
    // 공통 상수
    //=============================================================================

    constexpr Core::float32 PI = 3.14159265358979323846f;
    constexpr Core::float32 TWO_PI = 6.28318530717958647692f;
    constexpr Core::float32 HALF_PI = 1.57079632679489661923f;
    constexpr Core::float32 DEG_TO_RAD = 0.01745329251994329576f;
    constexpr Core::float32 RAD_TO_DEG = 57.2957795130823208767f;

    //=============================================================================
    // 변환 헬퍼 함수
    // Storage 타입 ↔ SIMD 타입 간 변환
    //=============================================================================

    // Vector3 변환
    inline VectorSIMD LoadVector3(const Vector3& v)
    {
        return DirectX::XMLoadFloat3(&v);
    }

    inline void StoreVector3(Vector3& out, const VectorSIMD& v)
    {
        DirectX::XMStoreFloat3(&out, v);
    }

    // Matrix4x4 변환
    inline MatrixSIMD LoadMatrix(const Matrix4x4& m)
    {
        return DirectX::XMLoadFloat4x4(&m);
    }

    inline void StoreMatrix(Matrix4x4& out, const MatrixSIMD& m)
    {
        DirectX::XMStoreFloat4x4(&out, m);
    }

} // namespace Math