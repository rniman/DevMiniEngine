#pragma once
#include "Math/MathTypes.h"
#include "Core/Types.h"
#include <vector>
#include <cmath>

namespace Graphics
{
	/**
	 * @brief 기본 도형 생성 유틸리티
	 *
	 * Mesh 초기화에 필요한 기본 도형 데이터를 생성합니다.
	 * 생성된 데이터는 MikkTSpaceCalculator와 함께 사용할 수 있습니다.
	 *
	 * @note 모든 프리미티브는 DirectX 표준 시계 방향(CW) 와인딩을 사용합니다.
	 */
	namespace PrimitiveGenerator
	{
		/**
		 * @brief 도형 생성 결과 데이터
		 *
		 * Mesh::InitializeStandard()에 전달할 수 있는 형태입니다.
		 */
		struct MeshData
		{
			std::vector<Math::Vector3> positions;
			std::vector<Math::Vector3> normals;
			std::vector<Math::Vector2> texCoords;
			std::vector<Core::uint16> indices;

			void Clear()
			{
				positions.clear();
				normals.clear();
				texCoords.clear();
				indices.clear();
			}

			void Reserve(size_t vertexCount, size_t indexCount)
			{
				positions.reserve(vertexCount);
				normals.reserve(vertexCount);
				texCoords.reserve(vertexCount);
				indices.reserve(indexCount);
			}
		};

		/**
		 * @brief 큐브 생성
		 *
		 * 원점 중심, 각 면에 독립적인 노멀과 UV를 가진 24개 정점
		 *
		 * @param halfExtent 큐브 절반 크기 (기본 1.0 → 2x2x2 큐브)
		 * @return MeshData 생성된 메시 데이터
		 */
		static inline MeshData GenerateCube(Core::float32 halfExtent = 1.0f)
		{
			MeshData data;

			constexpr Core::uint32 VERTICES_PER_FACE = 4;
			constexpr Core::uint32 FACE_COUNT = 6;
			constexpr Core::uint32 INDICES_PER_FACE = 6;

			data.Reserve(VERTICES_PER_FACE * FACE_COUNT, INDICES_PER_FACE * FACE_COUNT);

			const Core::float32 h = halfExtent;

			// 각 면별로 4개 정점 (총 24개)
			// 주석은 월드 좌표 기준 (X, Y, Z)

			// Front face (Z-) - 노멀이 -Z를 향함
			data.positions.push_back({ -h, -h, -h });  // 0: -X, -Y, -Z
			data.positions.push_back({ -h,  h, -h });  // 1: -X, +Y, -Z
			data.positions.push_back({ h,  h, -h });  // 2: +X, +Y, -Z
			data.positions.push_back({ h, -h, -h });  // 3: +X, -Y, -Z

			// Back face (Z+) - 노멀이 +Z를 향함
			data.positions.push_back({ h, -h,  h });  // 4: +X, -Y, +Z
			data.positions.push_back({ h,  h,  h });  // 5: +X, +Y, +Z
			data.positions.push_back({ -h,  h,  h });  // 6: -X, +Y, +Z
			data.positions.push_back({ -h, -h,  h });  // 7: -X, -Y, +Z

			// Left face (X-) - 노멀이 -X를 향함
			data.positions.push_back({ -h, -h,  h });  // 8:  -X, -Y, +Z
			data.positions.push_back({ -h,  h,  h });  // 9:  -X, +Y, +Z
			data.positions.push_back({ -h,  h, -h });  // 10: -X, +Y, -Z
			data.positions.push_back({ -h, -h, -h });  // 11: -X, -Y, -Z

			// Right face (X+) - 노멀이 +X를 향함
			data.positions.push_back({ h, -h, -h });  // 12: +X, -Y, -Z
			data.positions.push_back({ h,  h, -h });  // 13: +X, +Y, -Z
			data.positions.push_back({ h,  h,  h });  // 14: +X, +Y, +Z
			data.positions.push_back({ h, -h,  h });  // 15: +X, -Y, +Z

			// Top face (Y+) - 노멀이 +Y를 향함
			data.positions.push_back({ -h,  h, -h });  // 16: -X, +Y, -Z
			data.positions.push_back({ -h,  h,  h });  // 17: -X, +Y, +Z
			data.positions.push_back({ h,  h,  h });  // 18: +X, +Y, +Z
			data.positions.push_back({ h,  h, -h });  // 19: +X, +Y, -Z

			// Bottom face (Y-) - 노멀이 -Y를 향함
			data.positions.push_back({ -h, -h,  h });  // 20: -X, -Y, +Z
			data.positions.push_back({ -h, -h, -h });  // 21: -X, -Y, -Z
			data.positions.push_back({ h, -h, -h });  // 22: +X, -Y, -Z
			data.positions.push_back({ h, -h,  h });  // 23: +X, -Y, +Z

			// Normals (각 면별 4개씩)
			for (Core::uint32 i = 0; i < VERTICES_PER_FACE; ++i) data.normals.push_back({ 0.0f,  0.0f, -1.0f });  // Front
			for (Core::uint32 i = 0; i < VERTICES_PER_FACE; ++i) data.normals.push_back({ 0.0f,  0.0f,  1.0f });  // Back
			for (Core::uint32 i = 0; i < VERTICES_PER_FACE; ++i) data.normals.push_back({ -1.0f,  0.0f,  0.0f });  // Left
			for (Core::uint32 i = 0; i < VERTICES_PER_FACE; ++i) data.normals.push_back({ 1.0f,  0.0f,  0.0f });  // Right
			for (Core::uint32 i = 0; i < VERTICES_PER_FACE; ++i) data.normals.push_back({ 0.0f,  1.0f,  0.0f });  // Top
			for (Core::uint32 i = 0; i < VERTICES_PER_FACE; ++i) data.normals.push_back({ 0.0f, -1.0f,  0.0f });  // Bottom

			// TexCoords (각 면 동일한 UV 매핑)
			for (Core::uint32 face = 0; face < FACE_COUNT; ++face)
			{
				data.texCoords.push_back({ 0.0f, 1.0f });  // 좌하
				data.texCoords.push_back({ 0.0f, 0.0f });  // 좌상
				data.texCoords.push_back({ 1.0f, 0.0f });  // 우상
				data.texCoords.push_back({ 1.0f, 1.0f });  // 우하
			}

			// Indices (각 면 2개 삼각형, CW 순서)
			for (Core::uint16 face = 0; face < FACE_COUNT; ++face)
			{
				Core::uint16 base = face * VERTICES_PER_FACE;
				// 첫 번째 삼각형: 0 → 1 → 2 (CW)
				data.indices.push_back(base + 0);
				data.indices.push_back(base + 1);
				data.indices.push_back(base + 2);
				// 두 번째 삼각형: 0 → 2 → 3 (CW)
				data.indices.push_back(base + 0);
				data.indices.push_back(base + 2);
				data.indices.push_back(base + 3);
			}

			return data;
		}

		/**
		 * @brief 평면 생성
		 *
		 * XZ 평면, Y+ 방향 노멀
		 *
		 * @param width X축 크기
		 * @param depth Z축 크기
		 * @param subdivisionsX X축 분할 수 (기본 1)
		 * @param subdivisionsZ Z축 분할 수 (기본 1)
		 * @return MeshData 생성된 메시 데이터
		 */
		static inline MeshData GeneratePlane(
			Core::float32 width = 2.0f,
			Core::float32 depth = 2.0f,
			Core::uint32 subdivisionsX = 1,
			Core::uint32 subdivisionsZ = 1
		)
		{
			MeshData data;

			const Core::uint32 vertCountX = subdivisionsX + 1;
			const Core::uint32 vertCountZ = subdivisionsZ + 1;
			const Core::uint32 vertexCount = vertCountX * vertCountZ;
			const Core::uint32 indexCount = subdivisionsX * subdivisionsZ * 6;

			data.Reserve(vertexCount, indexCount);

			const Core::float32 halfWidth = width * 0.5f;
			const Core::float32 halfDepth = depth * 0.5f;
			const Core::float32 dx = width / static_cast<Core::float32>(subdivisionsX);
			const Core::float32 dz = depth / static_cast<Core::float32>(subdivisionsZ);

			// 정점 생성
			for (Core::uint32 z = 0; z <= subdivisionsZ; ++z)
			{
				for (Core::uint32 x = 0; x <= subdivisionsX; ++x)
				{
					Core::float32 px = -halfWidth + x * dx;
					Core::float32 pz = -halfDepth + z * dz;

					data.positions.push_back({ px, 0.0f, pz });
					data.normals.push_back({ 0.0f, 1.0f, 0.0f });
					data.texCoords.push_back({
						static_cast<Core::float32>(x) / subdivisionsX,
						static_cast<Core::float32>(z) / subdivisionsZ
						});
				}
			}

			// 인덱스 생성 (CW 순서, +Y 노멀)
			// near/far는 Z축 기준 (near: -Z, far: +Z)
			for (Core::uint32 z = 0; z < subdivisionsZ; ++z)
			{
				for (Core::uint32 x = 0; x < subdivisionsX; ++x)
				{
					Core::uint16 nearLeft = static_cast<Core::uint16>(z * vertCountX + x);
					Core::uint16 nearRight = nearLeft + 1;
					Core::uint16 farLeft = static_cast<Core::uint16>((z + 1) * vertCountX + x);
					Core::uint16 farRight = farLeft + 1;

					// 첫 번째 삼각형 (CW): nearLeft → nearRight → farLeft
					data.indices.push_back(nearLeft);
					data.indices.push_back(nearRight);
					data.indices.push_back(farLeft);

					// 두 번째 삼각형 (CW): nearRight → farRight → farLeft
					data.indices.push_back(nearRight);
					data.indices.push_back(farRight);
					data.indices.push_back(farLeft);
				}
			}

			return data;
		}

		/**
		 * @brief 구체 생성
		 *
		 * UV Sphere 방식
		 *
		 * @param radius 반지름
		 * @param slices 경도 분할 수 (가로, 기본 32)
		 * @param stacks 위도 분할 수 (세로, 기본 16)
		 * @return MeshData 생성된 메시 데이터
		 */
		static inline MeshData GenerateSphere(
			Core::float32 radius = 1.0f,
			Core::uint32 slices = 32,
			Core::uint32 stacks = 16
		)
		{
			MeshData data;

			const Core::uint32 vertexCount = (stacks + 1) * (slices + 1);
			const Core::uint32 indexCount = stacks * slices * 6;
			data.Reserve(vertexCount, indexCount);

			// 정점 생성
			for (Core::uint32 stack = 0; stack <= stacks; ++stack)
			{
				Core::float32 phi = Math::PI * static_cast<Core::float32>(stack) / static_cast<Core::float32>(stacks);
				Core::float32 sinPhi = std::sin(phi);
				Core::float32 cosPhi = std::cos(phi);

				for (Core::uint32 slice = 0; slice <= slices; ++slice)
				{
					Core::float32 theta = Math::TWO_PI * static_cast<Core::float32>(slice) / static_cast<Core::float32>(slices);
					Core::float32 sinTheta = std::sin(theta);
					Core::float32 cosTheta = std::cos(theta);

					Math::Vector3 normal(
						sinPhi * cosTheta,
						cosPhi,
						sinPhi * sinTheta
					);

					data.positions.push_back({
						normal.x * radius,
						normal.y * radius,
						normal.z * radius
						});
					data.normals.push_back(normal);
					data.texCoords.push_back({
						static_cast<Core::float32>(slice) / slices,
						static_cast<Core::float32>(stack) / stacks
						});
				}
			}

			// 인덱스 생성 (CW 순서)
			for (Core::uint32 stack = 0; stack < stacks; ++stack)
			{
				for (Core::uint32 slice = 0; slice < slices; ++slice)
				{
					Core::uint16 first = static_cast<Core::uint16>(stack * (slices + 1) + slice);
					Core::uint16 second = static_cast<Core::uint16>(first + slices + 1);

					// 첫 번째 삼각형 (CW)
					data.indices.push_back(first);
					data.indices.push_back(first + 1);
					data.indices.push_back(second);

					// 두 번째 삼각형 (CW)
					data.indices.push_back(first + 1);
					data.indices.push_back(second + 1);
					data.indices.push_back(second);
				}
			}

			return data;
		}

		/**
		 * @brief 실린더 생성
		 *
		 * Y축 기준, 원점 중심
		 *
		 * @param radius 반지름
		 * @param height 높이
		 * @param slices 원주 분할 수 (기본 32)
		 * @param hasCaps 뚜껑 생성 여부 (기본 true)
		 * @return MeshData 생성된 메시 데이터
		 */
		static inline MeshData GenerateCylinder(
			Core::float32 radius = 1.0f,
			Core::float32 height = 2.0f,
			Core::uint32 slices = 32,
			bool hasCaps = true
		)
		{
			MeshData data;

			const Core::float32 halfHeight = height * 0.5f;
			const Core::float32 angleStep = Math::TWO_PI / static_cast<Core::float32>(slices);

			// 옆면 정점
			for (Core::uint32 i = 0; i <= slices; ++i)
			{
				Core::float32 angle = angleStep * static_cast<Core::float32>(i);
				Core::float32 cosA = std::cos(angle);
				Core::float32 sinA = std::sin(angle);

				Math::Vector3 normal(cosA, 0.0f, sinA);
				Core::float32 u = static_cast<Core::float32>(i) / slices;

				// 상단
				data.positions.push_back({ cosA * radius,  halfHeight, sinA * radius });
				data.normals.push_back(normal);
				data.texCoords.push_back({ u, 0.0f });

				// 하단
				data.positions.push_back({ cosA * radius, -halfHeight, sinA * radius });
				data.normals.push_back(normal);
				data.texCoords.push_back({ u, 1.0f });
			}

			// 옆면 인덱스 (CW 순서)
			for (Core::uint32 i = 0; i < slices; ++i)
			{
				Core::uint16 top1 = static_cast<Core::uint16>(i * 2);
				Core::uint16 bottom1 = top1 + 1;
				Core::uint16 top2 = top1 + 2;
				Core::uint16 bottom2 = top1 + 3;

				// 첫 번째 삼각형 (CW): top1 → top2 → bottom1
				data.indices.push_back(top1);
				data.indices.push_back(top2);
				data.indices.push_back(bottom1);

				// 두 번째 삼각형 (CW): top2 → bottom2 → bottom1
				data.indices.push_back(top2);
				data.indices.push_back(bottom2);
				data.indices.push_back(bottom1);
			}

			// 뚜껑 (옵션)
			if (hasCaps)
			{
				Core::uint16 baseIndex = static_cast<Core::uint16>(data.positions.size());

				// 상단 뚜껑 중심
				data.positions.push_back({ 0.0f, halfHeight, 0.0f });
				data.normals.push_back({ 0.0f, 1.0f, 0.0f });
				data.texCoords.push_back({ 0.5f, 0.5f });

				Core::uint16 topCenterIndex = baseIndex;

				// 상단 뚜껑 가장자리
				for (Core::uint32 i = 0; i <= slices; ++i)
				{
					Core::float32 angle = angleStep * static_cast<Core::float32>(i);
					Core::float32 cosA = std::cos(angle);
					Core::float32 sinA = std::sin(angle);

					data.positions.push_back({ cosA * radius, halfHeight, sinA * radius });
					data.normals.push_back({ 0.0f, 1.0f, 0.0f });
					data.texCoords.push_back({ cosA * 0.5f + 0.5f, sinA * 0.5f + 0.5f });
				}

				// 상단 뚜껑 인덱스 (CW, 위에서 볼 때)
				for (Core::uint32 i = 0; i < slices; ++i)
				{
					data.indices.push_back(topCenterIndex);
					data.indices.push_back(static_cast<Core::uint16>(topCenterIndex + 1 + i + 1));
					data.indices.push_back(static_cast<Core::uint16>(topCenterIndex + 1 + i));
				}

				baseIndex = static_cast<Core::uint16>(data.positions.size());

				// 하단 뚜껑 중심
				data.positions.push_back({ 0.0f, -halfHeight, 0.0f });
				data.normals.push_back({ 0.0f, -1.0f, 0.0f });
				data.texCoords.push_back({ 0.5f, 0.5f });

				Core::uint16 bottomCenterIndex = baseIndex;

				// 하단 뚜껑 가장자리
				for (Core::uint32 i = 0; i <= slices; ++i)
				{
					Core::float32 angle = angleStep * static_cast<Core::float32>(i);
					Core::float32 cosA = std::cos(angle);
					Core::float32 sinA = std::sin(angle);

					data.positions.push_back({ cosA * radius, -halfHeight, sinA * radius });
					data.normals.push_back({ 0.0f, -1.0f, 0.0f });
					data.texCoords.push_back({ cosA * 0.5f + 0.5f, sinA * 0.5f + 0.5f });
				}

				// 하단 뚜껑 인덱스 (CW, 아래에서 볼 때)
				for (Core::uint32 i = 0; i < slices; ++i)
				{
					data.indices.push_back(bottomCenterIndex);
					data.indices.push_back(static_cast<Core::uint16>(bottomCenterIndex + 1 + i));
					data.indices.push_back(static_cast<Core::uint16>(bottomCenterIndex + 1 + i + 1));
				}
			}

			return data;
		}

	} // namespace PrimitiveGenerator

} // namespace Graphics
