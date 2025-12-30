#pragma once
#include "Math/MathTypes.h"
#include "Core/Types.h"
#include <vector>
#include <cmath>

namespace Math
{
	/**
	 * @brief 단일 삼각형의 Tangent 계산
	 *
	 * 개별 삼각형의 Tangent를 계산합니다.
	 * 전체 메시의 Tangent 계산은 MikkTSpaceCalculator를 사용하세요.
	 *
	 * @see Framework::MikkTSpaceCalculator
	 */
	static inline Vector3 CalculateTriangleTangent(
		const Vector3& pos0, const Vector3& pos1, const Vector3& pos2,
		const Vector2& uv0, const Vector2& uv1, const Vector2& uv2
	)
	{
		Vector3 edge1 = pos1 - pos0;
		Vector3 edge2 = pos2 - pos0;

		Core::float32 deltaU1 = uv1.x - uv0.x;
		Core::float32 deltaV1 = uv1.y - uv0.y;
		Core::float32 deltaU2 = uv2.x - uv0.x;
		Core::float32 deltaV2 = uv2.y - uv0.y;

		Core::float32 denominator = (deltaU1 * deltaV2 - deltaU2 * deltaV1);
		Core::float32 f = (std::abs(denominator) > 1e-6f) ? (1.0f / denominator) : 1.0f;

		Vector3 tangent;
		tangent.x = f * (deltaV2 * edge1.x - deltaV1 * edge2.x);
		tangent.y = f * (deltaV2 * edge1.y - deltaV1 * edge2.y);
		tangent.z = f * (deltaV2 * edge1.z - deltaV1 * edge2.z);

		return tangent.Normalized();
	}

} // namespace Math
