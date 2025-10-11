#include "Core/Logging/Logger.h"

namespace Core
{
    namespace Logging
    {
        Logger& Logger::GetInstance()
        {
            static Logger instance;
            return instance;
        }

        void Logger::Log(
            LogLevel level,
            LogCategory category,
            const std::string& message,
            const char* file,
            int line
        )
        {
            // Level filtering
            if (level < mMinLevel)
            {
                return;
            }

            // Create log message
            LogMessage msg(level, category, message, file, line);

            // Thread-safe output to all sinks
            std::lock_guard<std::mutex> lock(mMutex);

            for (auto& sink : mSinks)
            {
                sink->Write(msg);
            }
        }

        void Logger::AddSink(std::unique_ptr<LogSink> sink)
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mSinks.push_back(std::move(sink));
        }

        void Logger::ClearSinks()
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mSinks.clear();
        }

        void Logger::Flush()
        {
            std::lock_guard<std::mutex> lock(mMutex);

            for (auto& sink : mSinks)
            {
                sink->Flush();
            }
        }

    } // namespace Logging
} // namespace Core