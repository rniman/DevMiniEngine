#pragma once

#include "Core/Logging/Logger.h"
#include <cstdio>  // snprintf
#include <cstring> // strrchr

namespace Core
{
    namespace Logging
    {
        // 전체 경로에서 파일명만 추출 (예: "C:\Project\main.cpp" -> "main.cpp")
        inline const char* GetFileName(const char* path)
        {
            const char* file = strrchr(path, '\\');
            if (!file) file = strrchr(path, '/');
            return file ? file + 1 : path;
        }

        // printf 스타일 문자열 포맷팅 (최대 1024바이트)
        template<typename... Args>
        std::string FormatLog(const char* format, Args... args)
        {
            char buffer[1024];
            snprintf(buffer, sizeof(buffer), format, args...);
            return std::string(buffer);
        }

        // 오버플로우 방지를 위한 인자 없는 오버로드
        inline std::string FormatLog(const char* format)
        {
            return std::string(format);
        }

    } // namespace Logging
} // namespace Core

//-----------------------------------------------------------------------------
// 로깅 매크로
//-----------------------------------------------------------------------------
// NOTE: LOG_TRACE/LOG_DEBUG는 Release 빌드에서 컴파일 제외 (성능 최적화)

// Core 카테고리 매크로
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

// 특정 카테고리 매크로
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

// Release 빌드 최적화: Trace/Debug 로그 제거
#ifdef NDEBUG
#undef LOG_TRACE
#undef LOG_DEBUG
#define LOG_TRACE(format, ...) ((void)0)
#define LOG_DEBUG(format, ...) ((void)0)
#endif