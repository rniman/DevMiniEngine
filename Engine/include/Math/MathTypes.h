#pragma once
#include "Core/Types.h"
#include <DirectXMath.h>
#include <cmath>

namespace Math
{
	//=============================================================================
	// 전방 선언
	//=============================================================================

	struct Vector2;
	struct Vector3;
	struct Vector4;
	struct Matrix4x4;
	struct Quaternion;

	// SIMD 계산용 타입 (16 bytes aligned, 연산 최적화)
	using VectorSIMD = DirectX::XMVECTOR;
	using MatrixSIMD = DirectX::XMMATRIX;

	//=============================================================================
	// 공통 상수
	//=============================================================================

	constexpr Core::float32 PI = 3.14159265358979323846f;
	constexpr Core::float32 TWO_PI = 6.28318530717958647692f;
	constexpr Core::float32 HALF_PI = 1.57079632679489661923f;
	constexpr Core::float32 DEG_TO_RAD = 0.01745329251994329576f;
	constexpr Core::float32 RAD_TO_DEG = 57.2957795130823208767f;
	constexpr Core::float32 EPSILON = 1e-6f;

	//=============================================================================
	// Vector2
	//=============================================================================

	struct Vector2
	{
		Core::float32 x = 0.0f;
		Core::float32 y = 0.0f;

		// 생성자
		Vector2() = default;
		constexpr Vector2(Core::float32 x, Core::float32 y) : x(x), y(y) {}
		constexpr explicit Vector2(Core::float32 scalar) : x(scalar), y(scalar) {}
		Vector2(const DirectX::XMFLOAT2& xm) : x(xm.x), y(xm.y) {}

		// 정적 생성자
		static constexpr Vector2 Zero() { return Vector2(0.0f, 0.0f); }
		static constexpr Vector2 One() { return Vector2(1.0f, 1.0f); }
		static constexpr Vector2 UnitX() { return Vector2(1.0f, 0.0f); }
		static constexpr Vector2 UnitY() { return Vector2(0.0f, 1.0f); }

		// DirectX 변환
		DirectX::XMFLOAT2 ToXMFLOAT2() const { return DirectX::XMFLOAT2(x, y); }
		VectorSIMD ToSIMD() const { return DirectX::XMLoadFloat2(reinterpret_cast<const DirectX::XMFLOAT2*>(this)); }
		static Vector2 FromSIMD(VectorSIMD v)
		{
			Vector2 result;
			DirectX::XMStoreFloat2(reinterpret_cast<DirectX::XMFLOAT2*>(&result), v);
			return result;
		}

		// 벡터 연산
		Core::float32 Length() const { return std::sqrt(x * x + y * y); }
		Core::float32 LengthSquared() const { return x * x + y * y; }
		Vector2 Normalized() const
		{
			Core::float32 len = Length();
			if (len > EPSILON)
			{
				return Vector2(x / len, y / len);
			}
			return Zero();
		}
		void Normalize()
		{
			Core::float32 len = Length();
			if (len > EPSILON)
			{
				x /= len;
				y /= len;
			}
		}

		Core::float32 Dot(const Vector2& other) const { return x * other.x + y * other.y; }

		// 인덱스 접근
		Core::float32& operator[](Core::size_t index) { return (&x)[index]; }
		const Core::float32& operator[](Core::size_t index) const { return (&x)[index]; }

		// 단항 연산자
		Vector2 operator-() const { return Vector2(-x, -y); }

		// 이항 연산자
		Vector2 operator+(const Vector2& other) const { return Vector2(x + other.x, y + other.y); }
		Vector2 operator-(const Vector2& other) const { return Vector2(x - other.x, y - other.y); }
		Vector2 operator*(Core::float32 scalar) const { return Vector2(x * scalar, y * scalar); }
		Vector2 operator/(Core::float32 scalar) const { return Vector2(x / scalar, y / scalar); }
		Vector2 operator*(const Vector2& other) const { return Vector2(x * other.x, y * other.y); }
		Vector2 operator/(const Vector2& other) const { return Vector2(x / other.x, y / other.y); }

		// 복합 대입 연산자
		Vector2& operator+=(const Vector2& other) { x += other.x; y += other.y; return *this; }
		Vector2& operator-=(const Vector2& other) { x -= other.x; y -= other.y; return *this; }
		Vector2& operator*=(Core::float32 scalar) { x *= scalar; y *= scalar; return *this; }
		Vector2& operator/=(Core::float32 scalar) { x /= scalar; y /= scalar; return *this; }
		Vector2& operator*=(const Vector2& other) { x *= other.x; y *= other.y; return *this; }
		Vector2& operator/=(const Vector2& other) { x /= other.x; y /= other.y; return *this; }

		// 비교 연산자
		bool operator==(const Vector2& other) const
		{
			return std::abs(x - other.x) < EPSILON && std::abs(y - other.y) < EPSILON;
		}
		bool operator!=(const Vector2& other) const { return !(*this == other); }
	};

	// 스칼라 * 벡터 (교환법칙)
	inline Vector2 operator*(Core::float32 scalar, const Vector2& v) { return v * scalar; }

	//=============================================================================
	// Vector3
	//=============================================================================

	struct Vector3
	{
		Core::float32 x = 0.0f;
		Core::float32 y = 0.0f;
		Core::float32 z = 0.0f;

		// 생성자
		Vector3() = default;
		constexpr Vector3(Core::float32 x, Core::float32 y, Core::float32 z) : x(x), y(y), z(z) {}
		constexpr explicit Vector3(Core::float32 scalar) : x(scalar), y(scalar), z(scalar) {}
		Vector3(const DirectX::XMFLOAT3& xm) : x(xm.x), y(xm.y), z(xm.z) {}
		Vector3(const Vector2& v2, Core::float32 z = 0.0f) : x(v2.x), y(v2.y), z(z) {}

		// 정적 생성자
		static constexpr Vector3 Zero() { return Vector3(0.0f, 0.0f, 0.0f); }
		static constexpr Vector3 One() { return Vector3(1.0f, 1.0f, 1.0f); }
		static constexpr Vector3 UnitX() { return Vector3(1.0f, 0.0f, 0.0f); }
		static constexpr Vector3 UnitY() { return Vector3(0.0f, 1.0f, 0.0f); }
		static constexpr Vector3 UnitZ() { return Vector3(0.0f, 0.0f, 1.0f); }
		static constexpr Vector3 Up() { return Vector3(0.0f, 1.0f, 0.0f); }
		static constexpr Vector3 Down() { return Vector3(0.0f, -1.0f, 0.0f); }
		static constexpr Vector3 Forward() { return Vector3(0.0f, 0.0f, 1.0f); }
		static constexpr Vector3 Backward() { return Vector3(0.0f, 0.0f, -1.0f); }
		static constexpr Vector3 Right() { return Vector3(1.0f, 0.0f, 0.0f); }
		static constexpr Vector3 Left() { return Vector3(-1.0f, 0.0f, 0.0f); }

		// DirectX 변환
		DirectX::XMFLOAT3 ToXMFLOAT3() const { return DirectX::XMFLOAT3(x, y, z); }
		VectorSIMD ToSIMD() const { return DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(this)); }
		static Vector3 FromSIMD(VectorSIMD v)
		{
			Vector3 result;
			DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&result), v);
			return result;
		}

		// 벡터 연산
		Core::float32 Length() const { return std::sqrt(x * x + y * y + z * z); }
		Core::float32 LengthSquared() const { return x * x + y * y + z * z; }
		Vector3 Normalized() const
		{
			Core::float32 len = Length();
			if (len > EPSILON)
			{
				return Vector3(x / len, y / len, z / len);
			}
			return Zero();
		}
		void Normalize()
		{
			Core::float32 len = Length();
			if (len > EPSILON)
			{
				x /= len;
				y /= len;
				z /= len;
			}
		}

		Core::float32 Dot(const Vector3& other) const { return x * other.x + y * other.y + z * other.z; }
		Vector3 Cross(const Vector3& other) const
		{
			return Vector3(
				y * other.z - z * other.y,
				z * other.x - x * other.z,
				x * other.y - y * other.x
			);
		}

		// 동차 좌표 변환
		inline Vector4 ToPoint() const;
		inline Vector4 ToDirection() const;

		// 인덱스 접근
		Core::float32& operator[](Core::size_t index) { return (&x)[index]; }
		const Core::float32& operator[](Core::size_t index) const { return (&x)[index]; }

		// 단항 연산자
		Vector3 operator-() const { return Vector3(-x, -y, -z); }

		// 이항 연산자
		Vector3 operator+(const Vector3& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
		Vector3 operator-(const Vector3& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }
		Vector3 operator*(Core::float32 scalar) const { return Vector3(x * scalar, y * scalar, z * scalar); }
		Vector3 operator/(Core::float32 scalar) const { return Vector3(x / scalar, y / scalar, z / scalar); }
		Vector3 operator*(const Vector3& other) const { return Vector3(x * other.x, y * other.y, z * other.z); }
		Vector3 operator/(const Vector3& other) const { return Vector3(x / other.x, y / other.y, z / other.z); }

		// 복합 대입 연산자
		Vector3& operator+=(const Vector3& other) { x += other.x; y += other.y; z += other.z; return *this; }
		Vector3& operator-=(const Vector3& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }
		Vector3& operator*=(Core::float32 scalar) { x *= scalar; y *= scalar; z *= scalar; return *this; }
		Vector3& operator/=(Core::float32 scalar) { x /= scalar; y /= scalar; z /= scalar; return *this; }
		Vector3& operator*=(const Vector3& other) { x *= other.x; y *= other.y; z *= other.z; return *this; }
		Vector3& operator/=(const Vector3& other) { x /= other.x; y /= other.y; z /= other.z; return *this; }

		// 비교 연산자
		bool operator==(const Vector3& other) const
		{
			return std::abs(x - other.x) < EPSILON &&
				std::abs(y - other.y) < EPSILON &&
				std::abs(z - other.z) < EPSILON;
		}
		bool operator!=(const Vector3& other) const { return !(*this == other); }
	};

	// 스칼라 * 벡터 (교환법칙)
	inline Vector3 operator*(Core::float32 scalar, const Vector3& v) { return v * scalar; }

	//=============================================================================
	// Vector4
	//=============================================================================

	struct Vector4
	{
		Core::float32 x = 0.0f;
		Core::float32 y = 0.0f;
		Core::float32 z = 0.0f;
		Core::float32 w = 0.0f;

		// 생성자
		Vector4() = default;
		constexpr Vector4(Core::float32 x, Core::float32 y, Core::float32 z, Core::float32 w)
			: x(x), y(y), z(z), w(w) {
		}
		constexpr explicit Vector4(Core::float32 scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
		Vector4(const DirectX::XMFLOAT4& xm) : x(xm.x), y(xm.y), z(xm.z), w(xm.w) {}
		Vector4(const Vector3& v3, Core::float32 w = 0.0f) : x(v3.x), y(v3.y), z(v3.z), w(w) {}
		Vector4(const Vector2& v2, Core::float32 z = 0.0f, Core::float32 w = 0.0f)
			: x(v2.x), y(v2.y), z(z), w(w)
		{
		}

		// 정적 생성자
		static constexpr Vector4 Zero() { return Vector4(0.0f, 0.0f, 0.0f, 0.0f); }
		static constexpr Vector4 One() { return Vector4(1.0f, 1.0f, 1.0f, 1.0f); }
		static constexpr Vector4 UnitX() { return Vector4(1.0f, 0.0f, 0.0f, 0.0f); }
		static constexpr Vector4 UnitY() { return Vector4(0.0f, 1.0f, 0.0f, 0.0f); }
		static constexpr Vector4 UnitZ() { return Vector4(0.0f, 0.0f, 1.0f, 0.0f); }
		static constexpr Vector4 UnitW() { return Vector4(0.0f, 0.0f, 0.0f, 1.0f); }

		// DirectX 변환
		DirectX::XMFLOAT4 ToXMFLOAT4() const { return DirectX::XMFLOAT4(x, y, z, w); }
		VectorSIMD ToSIMD() const { return DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(this)); }
		static Vector4 FromSIMD(VectorSIMD v)
		{
			Vector4 result;
			DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result), v);
			return result;
		}

		// 하위 차원 추출
		Vector2 XY() const { return Vector2(x, y); }
		Vector3 XYZ() const { return Vector3(x, y, z); }

		// 벡터 연산
		Core::float32 Length() const { return std::sqrt(x * x + y * y + z * z + w * w); }
		Core::float32 LengthSquared() const { return x * x + y * y + z * z + w * w; }
		Vector4 Normalized() const
		{
			Core::float32 len = Length();
			if (len > EPSILON)
			{
				return Vector4(x / len, y / len, z / len, w / len);
			}
			return Zero();
		}
		void Normalize()
		{
			Core::float32 len = Length();
			if (len > EPSILON)
			{
				x /= len;
				y /= len;
				z /= len;
				w /= len;
			}
		}

		Core::float32 Dot(const Vector4& other) const
		{
			return x * other.x + y * other.y + z * other.z + w * other.w;
		}

		// 인덱스 접근
		Core::float32& operator[](Core::size_t index) { return (&x)[index]; }
		const Core::float32& operator[](Core::size_t index) const { return (&x)[index]; }

		// 단항 연산자
		Vector4 operator-() const { return Vector4(-x, -y, -z, -w); }

		// 이항 연산자
		Vector4 operator+(const Vector4& other) const
		{
			return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
		}
		Vector4 operator-(const Vector4& other) const
		{
			return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
		}
		Vector4 operator*(Core::float32 scalar) const
		{
			return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
		}
		Vector4 operator/(Core::float32 scalar) const
		{
			return Vector4(x / scalar, y / scalar, z / scalar, w / scalar);
		}
		Vector4 operator*(const Vector4& other) const
		{
			return Vector4(x * other.x, y * other.y, z * other.z, w * other.w);
		}
		Vector4 operator/(const Vector4& other) const
		{
			return Vector4(x / other.x, y / other.y, z / other.z, w / other.w);
		}

		// 복합 대입 연산자
		Vector4& operator+=(const Vector4& other)
		{
			x += other.x; y += other.y; z += other.z; w += other.w;
			return *this;
		}
		Vector4& operator-=(const Vector4& other)
		{
			x -= other.x; y -= other.y; z -= other.z; w -= other.w;
			return *this;
		}
		Vector4& operator*=(Core::float32 scalar)
		{
			x *= scalar; y *= scalar; z *= scalar; w *= scalar;
			return *this;
		}
		Vector4& operator/=(Core::float32 scalar)
		{
			x /= scalar; y /= scalar; z /= scalar; w /= scalar;
			return *this;
		}
		Vector4& operator*=(const Vector4& other)
		{
			x *= other.x; y *= other.y; z *= other.z; w *= other.w;
			return *this;
		}
		Vector4& operator/=(const Vector4& other)
		{
			x /= other.x; y /= other.y; z /= other.z; w /= other.w;
			return *this;
		}

		// 비교 연산자
		bool operator==(const Vector4& other) const
		{
			return std::abs(x - other.x) < EPSILON &&
				std::abs(y - other.y) < EPSILON &&
				std::abs(z - other.z) < EPSILON &&
				std::abs(w - other.w) < EPSILON;
		}
		bool operator!=(const Vector4& other) const { return !(*this == other); }
	};

	// 스칼라 * 벡터 (교환법칙)
	inline Vector4 operator*(Core::float32 scalar, const Vector4& v) { return v * scalar; }

	inline Vector4 Vector3::ToPoint() const { return Vector4(x, y, z, 1.0f); }
	inline Vector4 Vector3::ToDirection() const { return Vector4(x, y, z, 0.0f); }

	//=============================================================================
	// Quaternion
	//=============================================================================

	struct Quaternion
	{
		Core::float32 x = 0.0f;
		Core::float32 y = 0.0f;
		Core::float32 z = 0.0f;
		Core::float32 w = 1.0f;  // 단위 쿼터니언 기본값

		// 생성자
		Quaternion() = default;
		constexpr Quaternion(Core::float32 x, Core::float32 y, Core::float32 z, Core::float32 w)
			: x(x), y(y), z(z), w(w) {
		}
		Quaternion(const DirectX::XMFLOAT4& xm) : x(xm.x), y(xm.y), z(xm.z), w(xm.w) {}
		Quaternion(const Vector4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}

		// 정적 생성자
		static constexpr Quaternion Identity() { return Quaternion(0.0f, 0.0f, 0.0f, 1.0f); }

		// DirectX 변환
		DirectX::XMFLOAT4 ToXMFLOAT4() const { return DirectX::XMFLOAT4(x, y, z, w); }
		VectorSIMD ToSIMD() const { return DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(this)); }
		static Quaternion FromSIMD(VectorSIMD v)
		{
			Quaternion result;
			DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result), v);
			return result;
		}

		// 쿼터니언 연산
		Core::float32 Length() const { return std::sqrt(x * x + y * y + z * z + w * w); }
		Core::float32 LengthSquared() const { return x * x + y * y + z * z + w * w; }
		Quaternion Normalized() const
		{
			Core::float32 len = Length();
			if (len > EPSILON)
			{
				return Quaternion(x / len, y / len, z / len, w / len);
			}
			return Identity();
		}
		void Normalize()
		{
			Core::float32 len = Length();
			if (len > EPSILON)
			{
				x /= len;
				y /= len;
				z /= len;
				w /= len;
			}
		}

		Quaternion Conjugate() const { return Quaternion(-x, -y, -z, w); }
		Quaternion Inverse() const
		{
			Core::float32 lenSq = LengthSquared();
			if (lenSq > EPSILON)
			{
				Core::float32 invLenSq = 1.0f / lenSq;
				return Quaternion(-x * invLenSq, -y * invLenSq, -z * invLenSq, w * invLenSq);
			}
			return Identity();
		}

		Core::float32 Dot(const Quaternion& other) const
		{
			return x * other.x + y * other.y + z * other.z + w * other.w;
		}

		// 벡터 회전
		Vector3 RotateVector(const Vector3& v) const
		{
			VectorSIMD result = DirectX::XMVector3Rotate(v.ToSIMD(), ToSIMD());
			return Vector3::FromSIMD(result);
		}

		// 방향 벡터 추출
		Vector3 GetForward() const { return RotateVector(Vector3::Forward()); }
		Vector3 GetUp() const { return RotateVector(Vector3::Up()); }
		Vector3 GetRight() const { return RotateVector(Vector3::Right()); }

		// Euler 변환 
		inline Vector3 ToEuler() const;

		// 인덱스 접근
		Core::float32& operator[](Core::size_t index) { return (&x)[index]; }
		const Core::float32& operator[](Core::size_t index) const { return (&x)[index]; }

		// 쿼터니언 곱셈 (회전 합성)
		Quaternion operator*(const Quaternion& other) const
		{
			return Quaternion(
				w * other.x + x * other.w + y * other.z - z * other.y,
				w * other.y - x * other.z + y * other.w + z * other.x,
				w * other.z + x * other.y - y * other.x + z * other.w,
				w * other.w - x * other.x - y * other.y - z * other.z
			);
		}

		Quaternion& operator*=(const Quaternion& other)
		{
			*this = *this * other;
			return *this;
		}

		// 비교 연산자
		bool operator==(const Quaternion& other) const
		{
			return std::abs(x - other.x) < EPSILON &&
				std::abs(y - other.y) < EPSILON &&
				std::abs(z - other.z) < EPSILON &&
				std::abs(w - other.w) < EPSILON;
		}
		bool operator!=(const Quaternion& other) const { return !(*this == other); }
	};

	//=============================================================================
	// Matrix4x4
	//=============================================================================

	struct Matrix4x4
	{
		// Row-major storage (DirectX 호환)
		Core::float32 m[4][4] = {
			{1.0f, 0.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 1.0f, 0.0f},
			{0.0f, 0.0f, 0.0f, 1.0f}
		};

		// 생성자
		Matrix4x4() = default;
		Matrix4x4(
			Core::float32 m00, Core::float32 m01, Core::float32 m02, Core::float32 m03,
			Core::float32 m10, Core::float32 m11, Core::float32 m12, Core::float32 m13,
			Core::float32 m20, Core::float32 m21, Core::float32 m22, Core::float32 m23,
			Core::float32 m30, Core::float32 m31, Core::float32 m32, Core::float32 m33
		)
		{
			m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
			m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
			m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
			m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
		}
		Matrix4x4(const DirectX::XMFLOAT4X4& xm)
		{
			std::memcpy(m, &xm, sizeof(m));
		}

		// 정적 생성자
		static Matrix4x4 Identity()
		{
			return Matrix4x4(
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			);
		}
		static Matrix4x4 Zero()
		{
			return Matrix4x4(
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f
			);
		}

		// DirectX 변환
		DirectX::XMFLOAT4X4 ToXMFLOAT4X4() const
		{
			DirectX::XMFLOAT4X4 result;
			std::memcpy(&result, m, sizeof(m));
			return result;
		}
		MatrixSIMD ToSIMD() const
		{
			return DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(this));
		}
		static Matrix4x4 FromSIMD(MatrixSIMD mat)
		{
			Matrix4x4 result;
			DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&result), mat);
			return result;
		}

		// 행/열 접근
		Vector4 GetRow(Core::size_t row) const
		{
			return Vector4(m[row][0], m[row][1], m[row][2], m[row][3]);
		}
		Vector4 GetColumn(Core::size_t col) const
		{
			return Vector4(m[0][col], m[1][col], m[2][col], m[3][col]);
		}
		void SetRow(Core::size_t row, const Vector4& v)
		{
			m[row][0] = v.x; m[row][1] = v.y; m[row][2] = v.z; m[row][3] = v.w;
		}
		void SetColumn(Core::size_t col, const Vector4& v)
		{
			m[0][col] = v.x; m[1][col] = v.y; m[2][col] = v.z; m[3][col] = v.w;
		}

		// Translation 추출/설정
		Vector3 GetTranslation() const { return Vector3(m[3][0], m[3][1], m[3][2]); }
		void SetTranslation(const Vector3& v) { m[3][0] = v.x; m[3][1] = v.y; m[3][2] = v.z; }

		// 인덱스 접근
		Core::float32* operator[](Core::size_t row) { return m[row]; }
		const Core::float32* operator[](Core::size_t row) const { return m[row]; }

		// 행렬 곱셈
		Matrix4x4 operator*(const Matrix4x4& other) const
		{
			MatrixSIMD a = ToSIMD();
			MatrixSIMD b = other.ToSIMD();
			return FromSIMD(DirectX::XMMatrixMultiply(a, b));
		}

		Matrix4x4& operator*=(const Matrix4x4& other)
		{
			*this = *this * other;
			return *this;
		}

		// 행렬 * 벡터 (점 변환)
		Vector4 operator*(const Vector4& v) const
		{
			VectorSIMD vec = v.ToSIMD();
			MatrixSIMD mat = ToSIMD();
			return Vector4::FromSIMD(DirectX::XMVector4Transform(vec, mat));
		}

		// 비교 연산자
		bool operator==(const Matrix4x4& other) const
		{
			for (int i = 0; i < 4; ++i)
			{
				for (int j = 0; j < 4; ++j)
				{
					if (std::abs(m[i][j] - other.m[i][j]) >= EPSILON)
					{
						return false;
					}
				}
			}
			return true;
		}
		bool operator!=(const Matrix4x4& other) const { return !(*this == other); }
	};

	// Quaternion::ToEuler 구현 (Matrix4x4 정의 후)
	inline Vector3 Quaternion::ToEuler() const
	{
		// Quaternion을 행렬로 변환
		MatrixSIMD rotMat = DirectX::XMMatrixRotationQuaternion(ToSIMD());
		Matrix4x4 mat = Matrix4x4::FromSIMD(rotMat);

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

	//=============================================================================
	// Matrix3x3 (using alias 유지 - 사용 빈도 낮음)
	//=============================================================================

	using Matrix3x3 = DirectX::XMFLOAT3X3;

	//=============================================================================
	// SIMD 변환 헬퍼 함수 (하위 호환성)
	//=============================================================================

	inline VectorSIMD LoadVector2(const Vector2& v) { return v.ToSIMD(); }
	inline VectorSIMD LoadVector3(const Vector3& v) { return v.ToSIMD(); }
	inline VectorSIMD LoadVector4(const Vector4& v) { return v.ToSIMD(); }
	inline MatrixSIMD LoadMatrix(const Matrix4x4& m) { return m.ToSIMD(); }
	inline VectorSIMD LoadQuaternion(const Quaternion& q) { return q.ToSIMD(); }

	inline void StoreVector2(Vector2& out, VectorSIMD v) { out = Vector2::FromSIMD(v); }
	inline void StoreVector3(Vector3& out, VectorSIMD v) { out = Vector3::FromSIMD(v); }
	inline void StoreVector4(Vector4& out, VectorSIMD v) { out = Vector4::FromSIMD(v); }
	inline void StoreMatrix(Matrix4x4& out, MatrixSIMD m) { out = Matrix4x4::FromSIMD(m); }
	inline void StoreQuaternion(Quaternion& out, VectorSIMD v) { out = Quaternion::FromSIMD(v); }

} // namespace Math
