#pragma once

#include "Core/Logging/LogTypes.h"
#include "Core/Logging/LogSink.h"
#include "Core/Types.h"
#include <vector>
#include <memory>
#include <mutex>

namespace Core
{
    namespace Logging
    {
        /**
         * @brief Main logging system (Singleton)
         */
        class Logger
        {
        public:
            static Logger& GetInstance();

            /**
             * @brief Log a message
             */
            void Log(
                LogLevel level, 
                LogCategory category,
                const std::string& message,
                const char* file, 
                int line
            );

            /**
             * @brief Add output sink
             */
            void AddSink(std::unique_ptr<LogSink> sink);

            /**
             * @brief Remove all sinks
             */
            void ClearSinks();

            /**
             * @brief Set minimum log level
             */
            void SetMinLevel(LogLevel level) { mMinLevel = level; }

            /**
             * @brief Get minimum log level
             */
            LogLevel GetMinLevel() const { return mMinLevel; }

            /**
             * @brief Flush all sinks
             */
            void Flush();

        private:
            Logger() = default;
            ~Logger() = default;

            // Non-copyable, non-movable
            Logger(const Logger&) = delete;
            Logger& operator=(const Logger&) = delete;

            std::vector<std::unique_ptr<LogSink>> mSinks;
            LogLevel mMinLevel = LogLevel::Trace;
            std::mutex mMutex;
        };

    } // namespace Logging
} // namespace Core