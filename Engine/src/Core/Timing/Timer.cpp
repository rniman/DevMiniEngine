#include "pch.h"
#include "Core/Timing/Timer.h"
#include "Core/Logging/LogMacros.h"

namespace Core::Timing
{
	// 전역 타이머 인스턴스
	static Timer gGlobalTimer;

	Timer& GetGlobalTimer()
	{
		return gGlobalTimer;
	}

	Timer::Timer()
	{
#ifdef _WIN32
		int64 countsPerSec = 0;
		if (QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&countsPerSec)))
		{
			mSecondsPerCount = 1.0 / static_cast<float64>(countsPerSec);
			mIsValid = true;
			LOG_TRACE("High-precision timer initialized (Frequency: %lld Hz)", countsPerSec);
		}
		else
		{
			LOG_ERROR("Failed to query performance frequency");
			mIsValid = false;
			return;
		}
#elif defined(__linux__)
		mIsValid = false;
		LOG_WARN("Linux timer not yet implemented");
#else
		mIsValid = false;
		LOG_ERROR("Unsupported platform for high-precision timer");
#endif

		Reset();
	}

	void Timer::Tick(float32 lockFPS)
	{
		if (!mIsValid)
		{
			return;
		}

		if (mPaused)
		{
			mDeltaTime = 0.0f;
			mRawDeltaTime = 0.0f;
			return;
		}

		mCurrentTime = GetCurrentCounter();

		mRawDeltaTime = static_cast<float32>((mCurrentTime - mPreviousTime) * mSecondsPerCount);

		// FPS 제한 (VSync 미사용 시 활용)
		if (lockFPS > 0.0f)
		{
			float32 targetFrameTime = 1.0f / lockFPS;

			while (mRawDeltaTime < targetFrameTime)
			{
				// CPU 양보 후 즉시 반환
				if (targetFrameTime - mRawDeltaTime > 0.001f)
				{
#ifdef _WIN32
					Sleep(0);
#endif
				}

				mCurrentTime = GetCurrentCounter();
				mRawDeltaTime = static_cast<float32>((mCurrentTime - mPreviousTime) * mSecondsPerCount);
			}
		}

		mPreviousTime = mCurrentTime;

		// 극단적인 값 필터링 (디버거 중단, 포커스 전환 등)
		if (mRawDeltaTime > 1.0f)
		{
			mRawDeltaTime = 0.016667f;
		}

		// 순환 버퍼 업데이트 - O(1)
		if (mSampleCount < MAX_SAMPLE_COUNT)
		{
			// 초기 채우기 단계
			mFrameTimeHistory[mSampleCount] = mRawDeltaTime;
			mFrameTimeSum += mRawDeltaTime;
			mSampleCount++;
		}
		else
		{
			// 순환 교체: 오래된 값 빼고 새 값 더하기
			mFrameTimeSum -= mFrameTimeHistory[mSampleIndex];
			mFrameTimeSum += mRawDeltaTime;
			mFrameTimeHistory[mSampleIndex] = mRawDeltaTime;
			mSampleIndex = (mSampleIndex + 1) % MAX_SAMPLE_COUNT;
		}

		// 평균 delta time
		mDeltaTime = mFrameTimeSum / static_cast<float32>(mSampleCount);

		// FPS 계산 (1초마다 업데이트)
		mFrameCount++;
		mFpsTimeAccumulator += mRawDeltaTime;

		if (mFpsTimeAccumulator >= 1.0f)
		{
			mCurrentFrameRate = mFrameCount;
			mFrameCount = 0;
			mFpsTimeAccumulator -= 1.0f;
		}
	}

	void Timer::Start()
	{
		if (!mIsValid)
		{
			return;
		}

		int64 startTime = GetCurrentCounter();

		if (mPaused)
		{
			// 일시정지된 시간 누적
			mPausedTime += (startTime - mStopTime);
			mPreviousTime = startTime;
			mStopTime = 0;
			mPaused = false;

			LOG_TRACE("Timer resumed");
		}
	}

	void Timer::Stop()
	{
		if (!mIsValid)
		{
			return;
		}

		if (!mPaused)
		{
			mStopTime = GetCurrentCounter();
			mPaused = true;
			LOG_TRACE("Timer paused");
		}
	}

	void Timer::Reset()
	{
		if (!mIsValid)
		{
			return;
		}

		int64 currentTime = GetCurrentCounter();

		mBaseTime = currentTime;
		mPreviousTime = currentTime;
		mCurrentTime = currentTime;
		mStopTime = 0;
		mPausedTime = 0;
		mPaused = false;

		// 프레임 히스토리 초기화
		mFrameTimeHistory.fill(0.0f);
		mSampleIndex = 0;
		mSampleCount = 0;
		mFrameTimeSum = 0.0f;
		mDeltaTime = 0.0f;
		mRawDeltaTime = 0.0f;

		// FPS 카운터 초기화
		mCurrentFrameRate = 0;
		mFrameCount = 0;
		mFpsTimeAccumulator = 0.0f;

		LOG_TRACE("Timer reset");
	}

	float32 Timer::GetTotalTime() const
	{
		if (!mIsValid)
		{
			return 0.0f;
		}

		if (mPaused)
		{
			return static_cast<float32>(
				((mStopTime - mPausedTime) - mBaseTime) * mSecondsPerCount
				);
		}
		else
		{
			return static_cast<float32>(
				((mCurrentTime - mPausedTime) - mBaseTime) * mSecondsPerCount
				);
		}
	}

	std::string Timer::GetFrameRateString() const
	{
		return std::to_string(mCurrentFrameRate) + " FPS";
	}

#ifdef _WIN32
	int64 Timer::GetCurrentCounter() const
	{
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		return counter.QuadPart;
	}
#else
	int64 Timer::GetCurrentCounter() const
	{
		return 0;
	}
#endif

} // namespace Core::Timing
