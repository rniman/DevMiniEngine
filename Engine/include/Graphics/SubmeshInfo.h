#pragma once
#include "Core/Types.h"

namespace Graphics
{
	/**
	 * @brief 서브메시 정보
	 *
	 * 하나의 MeshResource가 여러 머티리얼을 사용할 때 각 부분을 정의합니다.
	 * DrawIndexedInstanced 호출에 필요한 파라미터를 포함합니다.
	 *
	 * @note Phase 4.4: Graphics 모듈로 이동 (의존성 방향 정리)
	 */
	struct SubmeshInfo
	{
		Core::uint32 startIndex = 0;      // 인덱스 버퍼 시작 위치
		Core::uint32 indexCount = 0;      // 인덱스 개수
		Core::uint32 baseVertex = 0;      // 정점 버퍼 오프셋 (DrawIndexedInstanced용)
		Core::uint32 materialIndex = 0;   // 머티리얼 슬롯 인덱스
	};
}
