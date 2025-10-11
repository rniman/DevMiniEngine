#pragma once

#include "Core/Logging/Logger.h"
#include <cstdio>  // snprintf
#include <cstring> // strrchr

namespace Core
{
    namespace Logging
    {
        // Helper to extract filename from path
        inline const char* GetFileName(const char* path)
        {
            const char* file = strrchr(path, '\\');
            if (!file) file = strrchr(path, '/');
            return file ? file + 1 : path;
        }

        // Format helper (printf-style)
        template<typename... Args>
        std::string FormatLog(const char* format, Args... args)
        {
            char buffer[1024];
            snprintf(buffer, sizeof(buffer), format, args...);
            return std::string(buffer);
        }

        // No-args version
        inline std::string FormatLog(const char* format)
        {
            return std::string(format);
        }

    } // namespace Logging
} // namespace Core

//=============================================================================
// Logging Macros
//=============================================================================

// Core macros
#define LOG_TRACE(format, ...) \
    Core::Logging::Logger::GetInstance().Log( \
        Core::Logging::LogLevel::Trace, \
        Core::Logging::LogCategory::Core, \
        Core::Logging::FormatLog(format, ##__VA_ARGS__), \
        Core::Logging::GetFileName(__FILE__), __LINE__)

#define LOG_DEBUG(format, ...) \
    Core::Logging::Logger::GetInstance().Log( \
        Core::Logging::LogLevel::Debug, \
        Core::Logging::LogCategory::Core, \
        Core::Logging::FormatLog(format, ##__VA_ARGS__), \
        Core::Logging::GetFileName(__FILE__), __LINE__)

#define LOG_INFO(format, ...) \
    Core::Logging::Logger::GetInstance().Log( \
        Core::Logging::LogLevel::Info, \
        Core::Logging::LogCategory::Core, \
        Core::Logging::FormatLog(format, ##__VA_ARGS__), \
        Core::Logging::GetFileName(__FILE__), __LINE__)

#define LOG_WARN(format, ...) \
    Core::Logging::Logger::GetInstance().Log( \
        Core::Logging::LogLevel::Warn, \
        Core::Logging::LogCategory::Core, \
        Core::Logging::FormatLog(format, ##__VA_ARGS__), \
        Core::Logging::GetFileName(__FILE__), __LINE__)

#define LOG_ERROR(format, ...) \
    Core::Logging::Logger::GetInstance().Log( \
        Core::Logging::LogLevel::Error, \
        Core::Logging::LogCategory::Core, \
        Core::Logging::FormatLog(format, ##__VA_ARGS__), \
        Core::Logging::GetFileName(__FILE__), __LINE__)

#define LOG_FATAL(format, ...) \
    Core::Logging::Logger::GetInstance().Log( \
        Core::Logging::LogLevel::Fatal, \
        Core::Logging::LogCategory::Core, \
        Core::Logging::FormatLog(format, ##__VA_ARGS__), \
        Core::Logging::GetFileName(__FILE__), __LINE__)

// Category-specific macros
#define LOG_GRAPHICS_INFO(format, ...) \
    Core::Logging::Logger::GetInstance().Log( \
        Core::Logging::LogLevel::Info, \
        Core::Logging::LogCategory::Graphics, \
        Core::Logging::FormatLog(format, ##__VA_ARGS__), \
        Core::Logging::GetFileName(__FILE__), __LINE__)

#define LOG_PHYSICS_WARN(format, ...) \
    Core::Logging::Logger::GetInstance().Log( \
        Core::Logging::LogLevel::Warn, \
        Core::Logging::LogCategory::Physics, \
        Core::Logging::FormatLog(format, ##__VA_ARGS__), \
        Core::Logging::GetFileName(__FILE__), __LINE__)

// Remove from release build (optional)
#ifdef NDEBUG
#undef LOG_TRACE
#undef LOG_DEBUG
#define LOG_TRACE(format, ...) ((void)0)
#define LOG_DEBUG(format, ...) ((void)0)
#endif