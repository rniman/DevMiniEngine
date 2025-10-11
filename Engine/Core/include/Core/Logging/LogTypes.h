#pragma once

#include <string>
#include <chrono>

namespace Core
{
    namespace Logging
    {
        /**
         * @brief Log severity levels
         */
        enum class LogLevel
        {
            Trace,    // Detailed trace information
            Debug,    // Debugging information  
            Info,     // General information
            Warn,     // Warning messages
            Error,    // Error messages
            Fatal     // Fatal errors (program termination)
        };

        /**
         * @brief Log categories for filtering
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
         * @brief Log message structure
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
         * @brief Convert log level to string
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
         * @brief Convert log category to string
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