#pragma once
#include "Core/Logging/LogSink.h"
#include "Core/Logging/LogTypes.h"
#include <string>

namespace Core
{
    namespace Logging
    {
        /**
         * @brief 콘솔 출력 Sink
         * 
         * @note Windows Console API를 사용한 컬러 출력 지원
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
