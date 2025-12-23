#include "pch.h"
#include "Core/Logging/Logger.h"

using namespace std;

namespace Core
{
	namespace Logging
	{
		void Logger::Log(
			LogLevel level,
			LogCategory category,
			const string& message,
			const char* file,
			int32 line
		)
		{
			if (level < mMinLevel)
			{
				return;
			}

			const LogMessage msg(level, category, message, file, line);

			lock_guard<mutex> lock(mMutex);

			// TODO: [P2] 고빈도 로깅 시나리오를 위한 비동기 큐 고려
			for (const auto& sink : mSinks)
			{
				sink->Write(msg);
			}
		}

		void Logger::AddSink(unique_ptr<LogSink> sink)
		{
			lock_guard<mutex> lock(mMutex);
			mSinks.push_back(move(sink));
		}

		void Logger::ClearSinks()
		{
			lock_guard<mutex> lock(mMutex);
			mSinks.clear();
		}

		void Logger::Flush()
		{
			lock_guard<mutex> lock(mMutex);

			for (auto& sink : mSinks)
			{
				sink->Flush();
			}
		}

	} // namespace Logging
} // namespace Core
