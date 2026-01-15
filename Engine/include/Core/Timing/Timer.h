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

		// 복사 금지, 이동 허용
		Timer(const Timer&) = delete;
		Timer& operator=(const Timer&) = delete;
		Timer(Timer&&) = default;
		Timer& operator=(Timer&&) = default;

		/**
		 * @brief 타이머 업데이트 (매 프레임 호출)
		 * @param lockFPS FPS 제한 (0 = 제한 없음, VSync 사용 시 불필요)
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
		 * @return 평균화된 delta time (게임 로직, 물리, 애니메이션용)
		 */
		float32 GetDeltaTime() const { return mDeltaTime; }

		/**
		 * @brief 순간 프레임 시간 (평균화 안 됨)
		 * @return 실제 프레임 시간 (프로파일링용)
		 */
		float32 GetRawDeltaTime() const { return mRawDeltaTime; }

		/**
		 * @brief 전체 경과 시간 (초 단위)
		 * @return 시작 이후 총 시간 (일시정지 시간 제외)
		 */
		float32 GetTotalTime() const;

		/**
		 * @brief 현재 FPS
		 */
		uint32 GetFrameRate() const { return mCurrentFrameRate; }

		/**
		 * @brief FPS 문자열 얻기
		 * @return "60 FPS" 형식의 문자열
		 */
		std::string GetFrameRateString() const;

		bool IsPaused() const { return mPaused; }
		bool IsValid() const { return mIsValid; }

	private:
		// 플랫폼별 카운터 쿼리
		int64 GetCurrentCounter() const;

#ifdef _WIN32
		float64 mSecondsPerCount = 0.0;
#endif

		// Delta time
		float32 mDeltaTime = 0.0f;
		float32 mRawDeltaTime = 0.0f;

		// 타이머 카운터
		int64 mBaseTime = 0;
		int64 mPausedTime = 0;
		int64 mStopTime = 0;
		int64 mPreviousTime = 0;
		int64 mCurrentTime = 0;

		// 프레임 시간 평균화 (순환 버퍼)
		static constexpr size_t MAX_SAMPLE_COUNT = 50;
		std::array<float32, MAX_SAMPLE_COUNT> mFrameTimeHistory = {};
		uint32 mSampleIndex = 0;      // 현재 삽입 위치
		uint32 mSampleCount = 0;      // 유효한 샘플 수
		float32 mFrameTimeSum = 0.0f; // 합계 캐싱

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
	 */
	Timer& GetGlobalTimer();

} // namespace Core::Timing
