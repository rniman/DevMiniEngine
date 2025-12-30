/**
 * @file MikkTSpaceCalculator.h
 * @brief MikkTSpace 라이브러리를 사용한 Tangent 계산 유틸리티
 *
 * 표준 Tangent Space 계산 알고리즘입니다.
 *
 * @note Phase 4.2: glTF 모델 로딩용
 */
#pragma once
#include "Math/MathTypes.h"
#include "Core/Types.h"
#include <vector>

namespace Framework
{
	/**
	 * @brief MikkTSpace 기반 Tangent 계산기
	 *
	 * MikkTSpace 알고리즘을 사용하여 메시의 Tangent를 계산합니다.
	 * 결과는 Vector4 형식으로, w 성분에 Bitangent 부호가 저장됩니다.
	 *
	 * @example
	 * std::vector<Math::Vector4> tangents;
	 * MikkTSpaceCalculator::Calculate(positions, normals, texCoords, indices, tangents);
	 * // tangent.xyz = tangent 방향
	 * // tangent.w = bitangent 부호 (+1 또는 -1)
	 */
	class MikkTSpaceCalculator
	{
	public:
		/**
		 * @brief 메시의 Tangent를 계산합니다
		 *
		 * @param positions 정점 위치 배열
		 * @param normals 정점 노말 배열
		 * @param texCoords 텍스처 좌표 배열
		 * @param indices 인덱스 배열 (uint16)
		 * @param outTangents 출력 Tangent 배열 (xyz = tangent, w = bitangent sign)
		 * @return 성공 여부
		 */
		static bool Calculate(
			const std::vector<Math::Vector3>& positions,
			const std::vector<Math::Vector3>& normals,
			const std::vector<Math::Vector2>& texCoords,
			const std::vector<Core::uint16>& indices,
			std::vector<Math::Vector4>& outTangents
		);

		/**
		 * @brief 메시의 Tangent를 계산합니다 (uint32 인덱스 버전)
		 *
		 * @param positions 정점 위치 배열
		 * @param normals 정점 노말 배열
		 * @param texCoords 텍스처 좌표 배열
		 * @param indices 인덱스 배열 (uint32)
		 * @param outTangents 출력 Tangent 배열 (xyz = tangent, w = bitangent sign)
		 * @return 성공 여부
		 */
		static bool Calculate(
			const std::vector<Math::Vector3>& positions,
			const std::vector<Math::Vector3>& normals,
			const std::vector<Math::Vector2>& texCoords,
			const std::vector<Core::uint32>& indices,
			std::vector<Math::Vector4>& outTangents
		);

	private:
		MikkTSpaceCalculator() = delete;
	};

} // namespace Framework
