// Engine/include/Graphics/DebugDraw/DebugShapes.h
#pragma once
#include "Graphics/VertexTypes.h"
#include "Math/MathTypes.h"
#include "Core/Types.h"
#include <vector>
#include <cmath>

namespace Graphics
{
	/**
	 * @brief Debug 와이어프레임 도형 생성 유틸리티
	 *
	 * Light Gizmo, Collider 시각화 등 디버그 렌더링용 도형을 생성합니다.
	 * Line List 토폴로지용 DebugVertex를 출력합니다.
	 */
	namespace DebugShapes
	{
		// 상수
		constexpr Core::uint32 DEFAULT_CIRCLE_SEGMENTS = 32;

		/**
		 * @brief 단위 원 생성 (반지름 1, 원점 중심)
		 *
		 * @param outVertices 출력 정점 배열 (Line List용)
		 * @param segments 원의 세그먼트 수 (기본 32)
		 * @param axis 원이 위치할 평면 (0: XY, 1: YZ, 2: XZ)
		 */
		static inline void GenerateCircle(
			std::vector<DebugVertex>& outVertices,
			Core::uint32 segments,
			Core::uint32 axis
		)
		{
			const Core::float32 angleStep = Math::TWO_PI / static_cast<Core::float32>(segments);

			for (Core::uint32 i = 0; i < segments; ++i)
			{
				Core::float32 angle0 = angleStep * static_cast<Core::float32>(i);
				Core::float32 angle1 = angleStep * static_cast<Core::float32>(i + 1);

				Core::float32 cos0 = std::cos(angle0);
				Core::float32 sin0 = std::sin(angle0);
				Core::float32 cos1 = std::cos(angle1);
				Core::float32 sin1 = std::sin(angle1);

				Math::Vector3 p0, p1;

				switch (axis)
				{
				case 0:  // XY 평면 (Z 법선)
					p0 = Math::Vector3(cos0, sin0, 0.0f);
					p1 = Math::Vector3(cos1, sin1, 0.0f);
					break;
				case 1:  // YZ 평면 (X 법선)
					p0 = Math::Vector3(0.0f, cos0, sin0);
					p1 = Math::Vector3(0.0f, cos1, sin1);
					break;
				case 2:  // XZ 평면 (Y 법선)
				default:
					p0 = Math::Vector3(cos0, 0.0f, sin0);
					p1 = Math::Vector3(cos1, 0.0f, sin1);
					break;
				}

				outVertices.emplace_back(p0);
				outVertices.emplace_back(p1);
			}
		}

		/**
		 * @brief 단위 구체 와이어프레임 생성 (3개의 원)
		 *
		 * Point Light 범위 시각화용
		 *
		 * @param outVertices 출력 정점 배열
		 * @param segments 각 원의 세그먼트 수 (기본 32)
		 */
		static inline void GenerateSphereWireframe(
			std::vector<DebugVertex>& outVertices,
			Core::uint32 segments
		)
		{
			GenerateCircle(outVertices, segments, 0);  // XY
			GenerateCircle(outVertices, segments, 1);  // YZ
			GenerateCircle(outVertices, segments, 2);  // XZ
		}

		/**
		 * @brief 단위 화살표 생성 (원점 -> +Z 방향)
		 *
		 * Directional Light 방향 시각화용
		 *
		 * @param outVertices 출력 정점 배열
		 * @param headRatio 머리 비율 (0.0 ~ 1.0)
		 * @param headSegments 머리 세그먼트 수
		 */
		static inline void GenerateArrow(
			std::vector<DebugVertex>& outVertices,
			Core::float32 headRatio = 0.2f,
			Core::uint32 headSegments = 8
		)
		{
			// 몸통(2) + 머리 베이스(headSegments * 2) + 머리 원뿔(headSegments * 2)
			size_t requiredVertexCount = 2 + (headSegments * 2) + (headSegments * 2);
			outVertices.reserve(outVertices.size() + requiredVertexCount);

			Math::Vector3 origin(0.0f, 0.0f, 0.0f);
			Math::Vector3 tip(0.0f, 0.0f, 1.0f);

			// 몸통
			outVertices.emplace_back(origin);
			outVertices.emplace_back(tip);

			// 머리 (원뿔)
			Core::float32 headLength = headRatio;
			Core::float32 headRadius = headRatio * 0.5f;
			Core::float32 headBase = 1.0f - headLength;
			Core::float32 angleStep = Math::TWO_PI / static_cast<Core::float32>(headSegments);

			for (Core::uint32 i = 0; i < headSegments; ++i)
			{
				Core::float32 angle = angleStep * static_cast<Core::float32>(i);

				Math::Vector3 basePoint(
					std::cos(angle) * headRadius,
					std::sin(angle) * headRadius,
					headBase
				);

				// 팁 → 베이스
				outVertices.emplace_back(tip);
				outVertices.emplace_back(basePoint);
			}

			// 베이스 원
			for (Core::uint32 i = 0; i < headSegments; ++i)
			{
				Core::float32 angle0 = angleStep * static_cast<Core::float32>(i);
				Core::float32 angle1 = angleStep * static_cast<Core::float32>(i + 1);

				Math::Vector3 p0(std::cos(angle0) * headRadius, std::sin(angle0) * headRadius, headBase);
				Math::Vector3 p1(std::cos(angle1) * headRadius, std::sin(angle1) * headRadius, headBase);

				outVertices.emplace_back(p0);
				outVertices.emplace_back(p1);
			}
		}

		/**
		 * @brief 단일 라인 생성
		 */
		static inline void GenerateLine(
			std::vector<DebugVertex>& outVertices,
			const Math::Vector3& start,
			const Math::Vector3& end
		)
		{
			outVertices.emplace_back(start);
			outVertices.emplace_back(end);
		}

		/**
		 * @brief 와이어프레임 박스 생성 (AABB)
		 *
		 * @param outVertices 출력 정점 배열
		 * @param halfExtents 박스 절반 크기
		 */
		static inline void GenerateBox(
			std::vector<DebugVertex>& outVertices,
			const Math::Vector3& halfExtents
		)
		{
			Core::float32 hx = halfExtents.x;
			Core::float32 hy = halfExtents.y;
			Core::float32 hz = halfExtents.z;

			Math::Vector3 v[8] = {
				{ -hx, -hy, -hz }, {  hx, -hy, -hz },
				{  hx,  hy, -hz }, { -hx,  hy, -hz },
				{ -hx, -hy,  hz }, {  hx, -hy,  hz },
				{  hx,  hy,  hz }, { -hx,  hy,  hz },
			};

			// 12개 엣지
			Core::uint32 edges[12][2] = {
				{0,1}, {1,2}, {2,3}, {3,0},  // 앞면
				{4,5}, {5,6}, {6,7}, {7,4},  // 뒷면
				{0,4}, {1,5}, {2,6}, {3,7},  // 연결
			};

			for (const auto& edge : edges)
			{
				GenerateLine(outVertices, v[edge[0]], v[edge[1]]);
			}
		}

	} // namespace DebugShapes

} // namespace Graphics
