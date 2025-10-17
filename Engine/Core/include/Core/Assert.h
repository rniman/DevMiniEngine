#pragma once

#include <cassert>
#include <stdexcept>

//=============================================================================
// 어설션 매크로
//=============================================================================

/**
 * @brief 디버그 전용 어설션
 *
 * Debug 빌드에서는 표준 assert 사용, Release 빌드에서는 예외 발생.
 * "절대 발생하지 않아야 하는" 조건 검증에 사용.
 *
 * @param condition 검증할 조건
 * @param message 실패 시 표시할 메시지
 *
 * @note Debug 빌드: assert로 즉시 중단 (디버거 연결)
 * @note Release 빌드: std::runtime_error 예외 발생
 *
 * 사용 예:
 *   CORE_ASSERT(size > 0, "Size must be positive");
 *   CORE_ASSERT((alignment & (alignment - 1)) == 0, "Alignment must be power of 2");
 */
#ifdef _DEBUG
#define CORE_ASSERT(condition, message) \
		assert((condition) && (message))
#else
#define CORE_ASSERT(condition, message) \
		do { \
			if (!(condition)) { \
				throw std::runtime_error(message); \
			} \
		} while(0)
#endif

 /**
  * @brief 항상 검증하는 어설션
  *
  * Debug와 Release 빌드 모두에서 항상 검증.
  * "외부 요인으로 실패 가능한" 조건 검증에 사용.
  *
  * @param condition 검증할 조건
  * @param message 실패 시 표시할 메시지
  *
  * @note 모든 빌드에서 std::runtime_error 예외 발생
  * @warning 성능에 민감한 핫 패스에서는 사용 자제
  *
  * 사용 예:
  *   CORE_VERIFY(file != nullptr, "Failed to open file");
  *   CORE_VERIFY(memory != nullptr, "Memory allocation failed");
  */
#define CORE_VERIFY(condition, message) \
	do { \
		if (!(condition)) { \
			throw std::runtime_error(message); \
		} \
	} while(0)