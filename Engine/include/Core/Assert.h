// Engine/include/Core/Assert.h
#pragma once

/**
 * @file Assert.h
 * @brief 런타임 조건 검증 매크로
 */

#include <cassert>
#include <stdexcept>
#include <cstdio>
#include <string>

namespace Core
{
	namespace Detail
	{
		/**
		 * @brief 포맷 문자열을 std::string으로 변환
		 * @param format printf 스타일 포맷 문자열
		 * @param ... 가변 인자
		 * @return 포맷팅된 문자열
		 */
		inline std::string FormatString(const char* format, ...)
		{
			char buffer[2048];

			va_list args;
			va_start(args, format);
			std::vsnprintf(buffer, sizeof(buffer), format, args);
			va_end(args);

			return std::string(buffer);
		}
	}
}

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
 * @param ... 포맷 문자열 및 인자 (printf 스타일)
 *
 * @note Debug 빌드: assert로 즉시 중단 (디버거 연결)
 * @note Release 빌드: std::runtime_error 예외 발생
 *
 * 사용 예:
 *   CORE_ASSERT(size > 0, "Size must be positive");
 *   CORE_ASSERT(index < count, "Index out of bounds: %zu >= %zu", index, count);
 *   CORE_ASSERT(ptr != nullptr, "Null pointer: %s", GetDebugName());
 */
#ifdef _DEBUG
#define CORE_ASSERT(condition, ...) \
		do { \
			if (!(condition)) { \
				std::fprintf(stderr, "Assertion failed: "); \
				std::fprintf(stderr, __VA_ARGS__); \
				std::fprintf(stderr, "\n  Condition: %s\n", #condition); \
				std::fprintf(stderr, "  File: %s:%d\n", __FILE__, __LINE__); \
				std::fprintf(stderr, "  Function: %s\n", __FUNCTION__); \
				assert(false && "CORE_ASSERT failed"); \
			} \
		} while(0)
#else
#define CORE_ASSERT(condition, ...) \
		do { \
			if (!(condition)) { \
				std::string msg = Core::Detail::FormatString(__VA_ARGS__); \
				throw std::runtime_error(msg); \
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
  * @param ... 포맷 문자열 및 인자 (printf 스타일)
  *
  * @note 모든 빌드에서 std::runtime_error 예외 발생
  * @warning 성능에 민감한 핫 패스에서는 사용 자제
  *
  * 사용 예:
  *   CORE_VERIFY(file != nullptr, "Failed to open file: %s", filename);
  *   CORE_VERIFY(memory != nullptr, "Memory allocation failed: %zu bytes", size);
  *   CORE_VERIFY(result == 0, "API call failed with error code: %d", result);
  */
#define CORE_VERIFY(condition, ...) \
	do { \
		if (!(condition)) { \
			std::string msg = Core::Detail::FormatString(__VA_ARGS__); \
			std::fprintf(stderr, "Verification failed: %s\n", msg.c_str()); \
			std::fprintf(stderr, "  Condition: %s\n", #condition); \
			std::fprintf(stderr, "  File: %s:%d\n", __FILE__, __LINE__); \
			throw std::runtime_error(msg); \
		} \
	} while(0)

  //=============================================================================
  // 추가 유틸리티 매크로
  //=============================================================================

  /**
   * @brief 구현되지 않은 기능 표시
   *
   * 사용 예:
   *   void FutureFeature() {
   *       CORE_NOT_IMPLEMENTED();
   *   }
   */
#define CORE_NOT_IMPLEMENTED() \
	CORE_ASSERT(false, "Not implemented: %s", __FUNCTION__)

   /**
	* @brief 도달해서는 안 되는 코드 표시
	*
	* 사용 예:
	*   switch (type) {
	*       case A: break;
	*       case B: break;
	*       default: CORE_UNREACHABLE();
	*   }
	*/
#define CORE_UNREACHABLE() \
	CORE_ASSERT(false, "Unreachable code: %s:%d", __FILE__, __LINE__)

	/**
	 * @brief 항상 실패하는 어설션
	 *
	 * 사용 예:
	 *   if (criticalError) {
	 *       CORE_FAIL("Critical error: %s", errorMsg);
	 *   }
	 */
#define CORE_FAIL(...) \
	CORE_VERIFY(false, __VA_ARGS__)