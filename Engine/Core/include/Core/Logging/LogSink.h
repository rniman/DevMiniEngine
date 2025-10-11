#pragma once

#include "Core/Logging/LogTypes.h"

namespace Core
{
    namespace Logging
    {
        /**
         * @brief Base interface for log output targets
         */
        class LogSink
        {
        public:
            virtual ~LogSink() = default;

            /**
             * @brief Write a log message
             */
            virtual void Write(const LogMessage& msg) = 0;

            /**
             * @brief Flush pending messages
             */
            virtual void Flush() {}

        protected:
            LogSink() = default;

            // Non-copyable
            LogSink(const LogSink&) = delete;
            LogSink& operator=(const LogSink&) = delete;
        };

    } // namespace Logging
} // namespace Core