#pragma once
#include "Math/MathTypes.h"
#include "Math/MathUtils.h"
#include "Core/Types.h"
#include <vector>

namespace Math
{
	/**
	 * @brief 삼각형 메시의 Tangent 벡터를 자동 계산
	 *
	 * Lengyel's Method를 사용한 Tangent Space 계산
	 * 참고: "Mathematics for 3D Game Programming" by Eric Lengyel
	 */
	static inline void CalculateTangents(
		const std::vector<Vector3>& positions,
		const std::vector<Vector3>& normals,
		const std::vector<Vector2>& texCoords,
		const std::vector<Core::uint16>& indices,
		std::vector<Vector3>& outTangents)
	{
		const size_t vertexCount = positions.size();

		// 출력 배열 초기화
		outTangents.clear();
		outTangents.resize(vertexCount, Vector3(0.0f, 0.0f, 0.0f));

		// 임시 배열: Tangent와 Bitangent 누적
		std::vector<Vector3> tan1(vertexCount, Vector3(0.0f, 0.0f, 0.0f));
		std::vector<Vector3> tan2(vertexCount, Vector3(0.0f, 0.0f, 0.0f));

		// 각 삼각형마다 Tangent/Bitangent 계산
		const size_t triangleCount = indices.size() / 3;

		for (size_t i = 0; i < triangleCount; ++i)
		{
			Core::uint32 i0 = static_cast<Core::uint32>(indices[i * 3 + 0]);
			Core::uint32 i1 = static_cast<Core::uint32>(indices[i * 3 + 1]);
			Core::uint32 i2 = static_cast<Core::uint32>(indices[i * 3 + 2]);

			const Vector3& v0 = positions[i0];
			const Vector3& v1 = positions[i1];
			const Vector3& v2 = positions[i2];

			const Vector2& w0 = texCoords[i0];
			const Vector2& w1 = texCoords[i1];
			const Vector2& w2 = texCoords[i2];

			// Edge 벡터
			Vector3 edge1 = Subtract(v1, v0);
			Vector3 edge2 = Subtract(v2, v0);

			// UV delta
			Core::float32 deltaU1 = w1.x - w0.x;
			Core::float32 deltaV1 = w1.y - w0.y;
			Core::float32 deltaU2 = w2.x - w0.x;
			Core::float32 deltaV2 = w2.y - w0.y;

			// Tangent & Bitangent 계산
			Core::float32 denominator = (deltaU1 * deltaV2 - deltaU2 * deltaV1);
			Core::float32 f = (abs(denominator) > 1e-6f) ? (1.0f / denominator) : 1.0f;

			Vector3 tangent;
			tangent.x = f * (deltaV2 * edge1.x - deltaV1 * edge2.x);
			tangent.y = f * (deltaV2 * edge1.y - deltaV1 * edge2.y);
			tangent.z = f * (deltaV2 * edge1.z - deltaV1 * edge2.z);

			Vector3 bitangent;
			bitangent.x = f * (-deltaU2 * edge1.x + deltaU1 * edge2.x);
			bitangent.y = f * (-deltaU2 * edge1.y + deltaU1 * edge2.y);
			bitangent.z = f * (-deltaU2 * edge1.z + deltaU1 * edge2.z);

			// 삼각형의 세 버텍스에 누적
			tan1[i0] = Add(tan1[i0], tangent);
			tan1[i1] = Add(tan1[i1], tangent);
			tan1[i2] = Add(tan1[i2], tangent);

			tan2[i0] = Add(tan2[i0], tangent);
			tan2[i1] = Add(tan2[i1], tangent);
			tan2[i2] = Add(tan2[i2], tangent);
		}

		// 각 버텍스마다 최종 Tangent 계산 (Gram-Schmidt Orthogonalization)
		for (size_t i = 0; i < vertexCount; ++i)
		{
			const Vector3& n = normals[i];
			const Vector3& t = tan1[i];

			// Gram-Schmidt: tangent - (tangent·normal) * normal
			Vector3 tangent = Subtract(t, Multiply(n, Dot(n, t)));

			// 길이 체크
			Core::float32 length = Length(tangent);
			if (length > 1e-6f)
			{

				tangent = Normalize(tangent);  // Normalize
			}
			else
			{
				// Degenerate case: n과 t가 평행
				// 임의의 수직 벡터 생성
				if (abs(n.x) < 0.9f)
				{
					tangent = Normalize(Cross(Vector3(1, 0, 0), n));
				}
				else
				{
					tangent = Normalize(Cross(Vector3(0, 1, 0), n));
				}
			}

			outTangents[i] = tangent;
		}
	}

	/**
	 * @brief 단일 삼각형의 Tangent 계산
	 */
	static inline Vector3 CalculateTriangleTangent(
		const Vector3& pos0, const Vector3& pos1, const Vector3& pos2,
		const Vector2& uv0, const Vector2& uv1, const Vector2& uv2
	)
	{
		Vector3 edge1 = Subtract(pos1, pos2);
		Vector3 edge2 = Subtract(pos2, pos0);

		Core::float32 deltaU1 = uv1.x - uv0.x;
		Core::float32 deltaV1 = uv1.y - uv0.y;
		Core::float32 deltaU2 = uv2.x - uv0.x;
		Core::float32 deltaV2 = uv2.y - uv0.y;

		Core::float32 denominator = (deltaU1 * deltaV2 - deltaU2 * deltaV1);
		Core::float32 f = (abs(denominator) > 1e-6f) ? (1.0f / denominator) : 1.0f;

		Vector3 tangent;
		tangent.x = f * (deltaV2 * edge1.x - deltaV1 * edge2.x);
		tangent.y = f * (deltaV2 * edge1.y - deltaV1 * edge2.y);
		tangent.z = f * (deltaV2 * edge1.z - deltaV1 * edge2.z);

		return Normalize(tangent);
	}

}
