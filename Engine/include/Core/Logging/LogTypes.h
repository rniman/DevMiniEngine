#pragma once

#include "Core/Types.h"
#include <string>
#include <chrono>

namespace Core
{
    namespace Logging
    {
        /**
         * @brief 로그 심각도 레벨
         * 
         * @note Release 빌드에서 Trace/Debug는 컴파일 시점에 제거됨
         */
        enum class LogLevel
        {
            Trace,
            Debug,
            Info,
            Warn,
            Error,
            Fatal
        };

        /**
         * @brief 로그 카테고리
         */
        enum class LogCategory
        {
            Core,
            Graphics,
            Physics,
            AI,
            Audio,
            Input,
            Memory
        };

        /**
         * @brief 로그 메시지 구조체
         */
        struct LogMessage
        {
            LogLevel level;
            LogCategory category;
            std::string message;
            std::string file;
            int line;
            std::chrono::system_clock::time_point timestamp;

            LogMessage(LogLevel lvl, LogCategory cat, const std::string& msg, const char* f, int l)
                : level(lvl)
                , category(cat)
                , message(msg)
                , file(f)
                , line(l)
                , timestamp(std::chrono::system_clock::now())
            {
            }
        };

        /**
         * @brief 로그 레벨을 문자열로 변환
         */
        inline const char* LogLevelToString(LogLevel level)
        {
            switch (level)
            {
            case LogLevel::Trace: return "TRACE";
            case LogLevel::Debug: return "DEBUG";
            case LogLevel::Info:  return "INFO ";
            case LogLevel::Warn:  return "WARN ";
            case LogLevel::Error: return "ERROR";
            case LogLevel::Fatal: return "FATAL";
            default: return "UNKNOWN";
            }
        }

        /**
         * @brief 로그 카테고리를 문자열로 변환
         */
        inline const char* LogCategoryToString(LogCategory category)
        {
            switch (category)
            {
            case LogCategory::Core:     return "Core";
            case LogCategory::Graphics: return "Graphics";
            case LogCategory::Physics:  return "Physics";
            case LogCategory::AI:       return "AI";
            case LogCategory::Audio:    return "Audio";
            case LogCategory::Input:    return "Input";
            case LogCategory::Memory:   return "Memory";
            default: return "Unknown";
            }
        }

    } // namespace Logging
} // namespace Core