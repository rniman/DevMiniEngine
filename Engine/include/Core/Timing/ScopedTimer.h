#pragma once
#include "Core/Timing/Timer.h"
#include <string>

namespace Core::Timing
{
	/**
	 * @brief 스코프 기반 시간 측정 유틸리티
	 *
	 * RAII를 이용하여 코드 블록의 실행 시간을 자동으로 측정합니다.
	 */
	class ScopedTimer
	{
	public:
		explicit ScopedTimer(const std::string& name);
		~ScopedTimer();

		// 복사/이동 금지
		ScopedTimer(const ScopedTimer&) = delete;
		ScopedTimer& operator=(const ScopedTimer&) = delete;
		ScopedTimer(ScopedTimer&&) = delete;
		ScopedTimer& operator=(ScopedTimer&&) = delete;

	private:
		std::string mName;
		Timer mTimer;
	};

	// 매크로를 통한 편리한 사용
#ifndef NDEBUG // 디버그 모드이거나 NDEBUG가 정의되지 않은 모든 빌드
	#define PROFILE_SCOPE(name) Core::Timing::ScopedTimer _scopedTimer##__LINE__(name)
	#define PROFILE_FUNCTION() PROFILE_SCOPE(__FUNCTION__)
#else // 릴리즈 모드일 때 (NDEBUG가 정의됨)
	// (void)0는 컴파일러에게 "아무것도 하지 말라"고 알리는 표준 관용구입니다.
	// 세미콜론이 필요한 위치에서도 문법 오류를 일으키지 않습니다.
	#define PROFILE_SCOPE(name) (void)0
	#define PROFILE_FUNCTION() (void)0
#endif

} // namespace Core::Timing
