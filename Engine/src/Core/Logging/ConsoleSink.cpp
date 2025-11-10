#include "pch.h"
#include "Core/Logging/ConsoleSink.h"
#include "Core/Logging/LogTypes.h"

using namespace std;

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

            cout << FormatLogMessage(msg) << endl;

            if (mUseColors)
            {
                ResetConsoleColor();
            }
        }

        string ConsoleSink::FormatLogMessage(const LogMessage& msg)
        {
            stringstream ss;

            auto time = chrono::system_clock::to_time_t(msg.timestamp);
            tm tm;
            localtime_s(&tm, &time);

            ss << "[" << put_time(&tm, "%H:%M:%S") << "] ";

            ss << "[" << LogLevelToString(msg.level) << "] ";

            ss << "[" << LogCategoryToString(msg.category) << "] ";

            ss << msg.message;

#ifdef _DEBUG
            // 디버그 빌드에서만 파일 위치 표시로 빠른 문제 추적 지원
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
