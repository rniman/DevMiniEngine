#include "pch.h"
#include "Core/Timing/ScopedTimer.h"

#include "Core/Logging/LogMacros.h"
#include "Core/Types.h"

namespace Core::Timing
{
	ScopedTimer::ScopedTimer(const std::string& name)
		: mName(name)
	{
		mTimer.Reset();
		mTimer.Start();
	}

	ScopedTimer::~ScopedTimer()
	{
		mTimer.Stop();
		float32 elapsedMs = mTimer.GetTotalTime() * 1000.0f;

		// 시간에 따라 다른 로그 레벨 사용
		if (elapsedMs > 100.0f)
		{
			LOG_WARN("[PROFILE] %s took %.2f ms", mName.c_str(), elapsedMs);
		}
		else if (elapsedMs > 50.0f)
		{
			LOG_INFO("[PROFILE] %s took %.2f ms", mName.c_str(), elapsedMs);
		}
		else
		{
			LOG_TRACE("[PROFILE] %s took %.2f ms", mName.c_str(), elapsedMs);
		}
	}

} // namespace Core::Timing
