#pragma once

#include "Core/Logging/LogSink.h"

namespace Core
{
    namespace Logging
    {
        /**
         * @brief Console output sink with color support
         */
        class ConsoleSink : public LogSink
        {
        public:
            ConsoleSink(bool useColors = true);
            ~ConsoleSink() override = default;

            void Write(const LogMessage& msg) override;

        private:
            void SetConsoleColor(LogLevel level);
            void ResetConsoleColor();

            std::string FormatLogMessage(const LogMessage& msg);

            bool mUseColors;

#ifdef _WIN32
            void* mConsoleHandle;  // HANDLE
#endif
        };

    } // namespace Logging
} // namespace Core