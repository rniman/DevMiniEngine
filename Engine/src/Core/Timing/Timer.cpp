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
		// Windows 고정밀 타이머 초기화
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
		// Linux 타이머 초기화 (향후 구현)
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

		// 일시정지 상태면 delta time은 0
		if (mPaused)
		{
			mDeltaTime = 0.0f;
			mRawDeltaTime = 0.0f;
			return;
		}

		// 현재 시간 얻기
		mCurrentTime = GetCurrentCounter();

		// 프레임 시간 계산 (초 단위)
		mRawDeltaTime = static_cast<float32>(
			(mCurrentTime - mPreviousTime) * mSecondsPerCount
			);

		// FPS 제한 (선택적)
		if (lockFPS > 0.0f)
		{
			float32 targetFrameTime = 1.0f / lockFPS;

			// 목표 프레임 시간에 도달할 때까지 대기
			while (mRawDeltaTime < targetFrameTime)
			{
				// CPU 점유율을 낮추기 위해 약간의 Sleep
				if (targetFrameTime - mRawDeltaTime > 0.001f)
				{
#ifdef _WIN32
					Sleep(0);  // 다른 스레드에 양보
#endif
				}

				mCurrentTime = GetCurrentCounter();
				mRawDeltaTime = static_cast<float32>(
					(mCurrentTime - mPreviousTime) * mSecondsPerCount
					);
			}
		}

		mPreviousTime = mCurrentTime;

		// 극단적인 값 필터링 (1초 이상 차이는 무시)
		// 디버거 중단이나 포커스 전환 시 발생할 수 있는 큰 delta time 방지
		if (mRawDeltaTime > 1.0f)
		{
			mRawDeltaTime = 0.016667f;  // 60 FPS 기본값
		}

		// 프레임 시간 히스토리 업데이트 (순환 버퍼)
		if (mSampleCount < MAX_SAMPLE_COUNT)
		{
			mFrameTimeHistory[mSampleCount++] = mRawDeltaTime;
		}
		else
		{
			// 오래된 값을 제거하고 새 값 추가
			std::rotate(
				mFrameTimeHistory.begin(),
				mFrameTimeHistory.begin() + 1,
				mFrameTimeHistory.end()
			);
			mFrameTimeHistory[MAX_SAMPLE_COUNT - 1] = mRawDeltaTime;
		}

		// 평균 delta time 계산
		float32 sum = 0.0f;
		for (uint32 i = 0; i < mSampleCount; ++i)
		{
			sum += mFrameTimeHistory[i];
		}
		mDeltaTime = (mSampleCount > 0) ? (sum / static_cast<float32>(mSampleCount)) : mRawDeltaTime;

		// FPS 계산
		mFrameCount++;
		mFpsTimeAccumulator += mRawDeltaTime;

		// 1초마다 FPS 업데이트
		if (mFpsTimeAccumulator >= 1.0f)
		{
			mCurrentFrameRate = mFrameCount;
			mFrameCount = 0;
			mFpsTimeAccumulator -= 1.0f;  // 정확한 1초 간격 유지
		}
	}

	void Timer::Start()
	{
		if (!mIsValid)
		{
			return;
		}

		int64 startTime = GetCurrentCounter();

		// 일시정지 상태에서 재개
		if (mPaused)
		{
			// 일시정지된 시간 누적
			mPausedTime += (startTime - mStopTime);

			// 이전 시간을 현재로 설정하여 큰 delta time 방지
			mPreviousTime = startTime;

			// 상태 초기화
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

		// 이미 정지 상태면 무시
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
		mSampleCount = 0;
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

		// 정지 상태면 정지된 시점까지의 시간 반환
		if (mPaused)
		{
			return static_cast<float32>(
				((mStopTime - mPausedTime) - mBaseTime) * mSecondsPerCount
				);
		}
		// 실행 중이면 현재까지의 시간 반환
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
		// 지원하지 않는 플랫폼
		return 0;
	}
#endif

} // namespace Core::Timing
