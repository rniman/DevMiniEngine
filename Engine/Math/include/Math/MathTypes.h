#pragma once

#include <DirectXMath.h>

namespace Core
{
    namespace Math
    {
        //=============================================================================
        // Vector Types
        //=============================================================================

        using Vector2 = DirectX::XMFLOAT2;
        using Vector3 = DirectX::XMFLOAT3;
        using Vector4 = DirectX::XMFLOAT4;

        // SIMD vector (use for calculations)
        using VectorSIMD = DirectX::XMVECTOR;

        //=============================================================================
        // Matrix Types
        //=============================================================================

        using Matrix3x3 = DirectX::XMFLOAT3X3;
        using Matrix4x4 = DirectX::XMFLOAT4X4;

        // SIMD matrix (use for calculations)
        using MatrixSIMD = DirectX::XMMATRIX;

        //=============================================================================
        // Quaternion
        //=============================================================================

        using Quaternion = DirectX::XMFLOAT4;  // Storage type
        using QuaternionSIMD = DirectX::XMVECTOR;  // Calculation type

        //=============================================================================
        // Common Constants
        //=============================================================================

        constexpr float PI = 3.14159265358979323846f;
        constexpr float TWO_PI = 6.28318530717958647692f;
        constexpr float HALF_PI = 1.57079632679489661923f;
        constexpr float DEG_TO_RAD = 0.01745329251994329576f;
        constexpr float RAD_TO_DEG = 57.2957795130823208767f;

        //=============================================================================
        // Helper Functions (Load/Store between storage and SIMD)
        //=============================================================================

        inline VectorSIMD LoadVector3(const Vector3& v)
        {
            return DirectX::XMLoadFloat3(&v);
        }

        inline void StoreVector3(Vector3& out, const VectorSIMD& v)
        {
            DirectX::XMStoreFloat3(&out, v);
        }

        inline MatrixSIMD LoadMatrix(const Matrix4x4& m)
        {
            return DirectX::XMLoadFloat4x4(&m);
        }

        inline void StoreMatrix(Matrix4x4& out, const MatrixSIMD& m)
        {
            DirectX::XMStoreFloat4x4(&out, m);
        }

    } // namespace Math
} // namespace Core