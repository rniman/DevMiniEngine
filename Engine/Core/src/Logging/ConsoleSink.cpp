#include "Core/Logging/ConsoleSink.h"
#include <iostream>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#endif

namespace Core
{
    namespace Logging
    {
        ConsoleSink::ConsoleSink(bool useColors)
            : mUseColors(useColors)
#ifdef _WIN32
            , mConsoleHandle(GetStdHandle(STD_OUTPUT_HANDLE))
#endif
        {
        }

        void ConsoleSink::Write(const LogMessage& msg)
        {
            if (mUseColors)
            {
                SetConsoleColor(msg.level);
            }

            std::cout << FormatLogMessage(msg) << std::endl;

            if (mUseColors)
            {
                ResetConsoleColor();
            }
        }

        std::string ConsoleSink::FormatLogMessage(const LogMessage& msg)
        {
            std::stringstream ss;

            // Timestamp
            auto time = std::chrono::system_clock::to_time_t(msg.timestamp);
            std::tm tm;
            localtime_s(&tm, &time);

            ss << "[" << std::put_time(&tm, "%H:%M:%S") << "] ";

            // Level
            ss << "[" << LogLevelToString(msg.level) << "] ";

            // Category
            ss << "[" << LogCategoryToString(msg.category) << "] ";

            // Message
            ss << msg.message;

            // File & Line (Debug only)
#ifdef _DEBUG
            ss << " (" << msg.file << ":" << msg.line << ")";
#endif

            return ss.str();
        }

        void ConsoleSink::SetConsoleColor(LogLevel level)
        {
#ifdef _WIN32
            WORD color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;  // White

            switch (level)
            {
            case LogLevel::Trace:
                color = FOREGROUND_INTENSITY;  // Gray
                break;
            case LogLevel::Debug:
                color = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;  // Cyan
                break;
            case LogLevel::Info:
                color = FOREGROUND_GREEN | FOREGROUND_INTENSITY;  // Green
                break;
            case LogLevel::Warn:
                color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;  // Yellow
                break;
            case LogLevel::Error:
                color = FOREGROUND_RED | FOREGROUND_INTENSITY;  // Red
                break;
            case LogLevel::Fatal:
                color = FOREGROUND_RED | BACKGROUND_INTENSITY;  // Red on white
                break;
            }

            SetConsoleTextAttribute(mConsoleHandle, color);
#endif
        }

        void ConsoleSink::ResetConsoleColor()
        {
#ifdef _WIN32
            SetConsoleTextAttribute(mConsoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
        }

    } // namespace Logging
} // namespace Core