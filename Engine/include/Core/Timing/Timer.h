#pragma once

#include "Core/Types.h"
#include <array>
#include <string>

namespace Core::Timing
{
	/**
	 * @brief 고정밀 타이머 클래스
	 *
	 * 플랫폼별 고정밀 타이머를 사용하여 정확한 시간 측정을 제공합니다.
	 * Windows: QueryPerformanceCounter
	 */
	class Timer
	{
	public:
		Timer();
		~Timer() = default;

		// 복사/이동 금지 (싱글톤 아니지만 타이머는 복사될 필요 없음)
		Timer(const Timer&) = delete;
		Timer& operator=(const Timer&) = delete;
		Timer(Timer&&) = default;
		Timer& operator=(Timer&&) = default;

		/**
		 * @brief 타이머 업데이트 (매 프레임 호출)
		 * @param lockFPS FPS 제한 (0 = 제한 없음)
		 */
		void Tick(float32 lockFPS = 0.0f);

		/**
		 * @brief 타이머 시작/재개
		 */
		void Start();

		/**
		 * @brief 타이머 일시정지
		 */
		void Stop();

		/**
		 * @brief 타이머 리셋
		 */
		void Reset();

		/**
		 * @brief 프레임 간 경과 시간 (초 단위)
		 * @return 평균화된 delta time
		 */
		float32 GetDeltaTime() const { return mDeltaTime; }

		/**
		 * @brief 순간 프레임 시간 (평균화 안 됨)
		 * @return 실제 프레임 시간
		 */
		float32 GetRawDeltaTime() const { return mRawDeltaTime; }

		/**
		 * @brief 전체 경과 시간 (초 단위)
		 * @return 시작 이후 총 시간 (일시정지 시간 제외)
		 */
		float32 GetTotalTime() const;

		/**
		 * @brief 현재 FPS
		 * @return 초당 프레임 수
		 */
		uint32 GetFrameRate() const { return mCurrentFrameRate; }

		/**
		 * @brief FPS 문자열 얻기
		 * @return "60 FPS" 형식의 문자열
		 */
		std::string GetFrameRateString() const;

		/**
		 * @brief 일시정지 상태 확인
		 * @return 일시정지 중이면 true
		 */
		bool IsPaused() const { return mPaused; }

		/**
		 * @brief 플랫폼별 타이머 초기화 확인
		 * @return 타이머가 정상적으로 초기화되었으면 true
		 */
		bool IsValid() const { return mIsValid; }

	private:
		// 플랫폼별 구현
#ifdef _WIN32
		float64 mSecondsPerCount = 0.0;
#endif

		// 플랫폼별 카운터 쿼리
		int64 GetCurrentCounter() const;

		// Delta time
		float32 mDeltaTime = 0.0f;
		float32 mRawDeltaTime = 0.0f;

		// 타이머 카운터
		int64 mBaseTime = 0;
		int64 mPausedTime = 0;
		int64 mStopTime = 0;
		int64 mPreviousTime = 0;
		int64 mCurrentTime = 0;

		// 프레임 시간 평균화
		static constexpr uint32 MAX_SAMPLE_COUNT = 50;
		std::array<float32, MAX_SAMPLE_COUNT> mFrameTimeHistory = {};
		uint32 mSampleCount = 0;

		// FPS 계산
		uint32 mCurrentFrameRate = 0;
		uint32 mFrameCount = 0;
		float32 mFpsTimeAccumulator = 0.0f;

		// 상태
		bool mPaused = false;
		bool mIsValid = false;
	};

	/**
	 * @brief 전역 타이머 인스턴스 획득
	 * @return 전역 타이머 참조
	 */
	Timer& GetGlobalTimer();

} // namespace Core::Timing
