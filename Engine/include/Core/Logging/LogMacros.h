#pragma once

#include "Core/Logging/Logger.h"
#include "Core/Logging/LogTypes.h"

#include <cstring>
#include <string>

namespace Core
{
	namespace Logging
	{
		/**
		 * @brief 전체 경로에서 파일명만 추출
		 * @param path 전체 파일 경로
		 * @return 파일명만 (예: "C:\\Project\\main.cpp" -> "main.cpp")
		 */
		inline const char* GetFileName(const char* path)
		{
			const char* file = strrchr(path, '\\');
			if (!file) file = strrchr(path, '/');
			return file ? file + 1 : path;
		}

		/**
		 * @brief printf 스타일 문자열 포맷팅
		 * @param format 포맷 문자열
		 * @param args 가변 인자
		 * @return 포맷팅된 문자열 (최대 1024바이트)
		 */
		template<typename... Args>
		std::string FormatLog(const char* format, Args... args)
		{
			char buffer[1024] = {};
			snprintf(buffer, sizeof(buffer), format, args...);
			return std::string(buffer);
		}

		inline std::string FormatLog(const char* format)
		{
			return std::string(format);
		}

	} // namespace Logging
} // namespace Core

//=============================================================================
// 범용 로그 매크로
//=============================================================================

/**
 * @brief 범용 로그 매크로 (모든 카테고리/레벨 지원)
 * @param level 로그 레벨 (Trace, Debug, Info, Warn, Error, Fatal)
 * @param category 로그 카테고리 (Core, Graphics, ECS, Physics 등)
 * @param format printf 스타일 포맷 문자열
 *
 * 사용 예:
 *   LOG(Info, Audio, "Sound loaded: %s", soundName);
 *   LOG(Warn, AI, "Pathfinding timeout");
 */
#define LOG(level, category, format, ...) \
    Core::Logging::Logger::GetInstance().Log( \
        Core::Logging::LogLevel::level, \
        Core::Logging::LogCategory::category, \
        Core::Logging::FormatLog(format, ##__VA_ARGS__), \
        Core::Logging::GetFileName(__FILE__), __LINE__)

 //=============================================================================
 // Core 카테고리 (기본, 가장 자주 사용)
 //=============================================================================

#define LOG_TRACE(format, ...) LOG(Trace, Core, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) LOG(Debug, Core, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...)  LOG(Info,  Core, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...)  LOG(Warn,  Core, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) LOG(Error, Core, format, ##__VA_ARGS__)
#define LOG_FATAL(format, ...) LOG(Fatal, Core, format, ##__VA_ARGS__)

//=============================================================================
// Graphics 카테고리
//=============================================================================

#define LOG_GFX_TRACE(format, ...) LOG(Trace, Graphics, format, ##__VA_ARGS__)
#define LOG_GFX_DEBUG(format, ...) LOG(Debug, Graphics, format, ##__VA_ARGS__)
#define LOG_GFX_INFO(format, ...)  LOG(Info,  Graphics, format, ##__VA_ARGS__)
#define LOG_GFX_WARN(format, ...)  LOG(Warn,  Graphics, format, ##__VA_ARGS__)
#define LOG_GFX_ERROR(format, ...) LOG(Error, Graphics, format, ##__VA_ARGS__)

//=============================================================================
// ECS 카테고리
//=============================================================================

#define LOG_ECS_TRACE(format, ...) LOG(Trace, ECS, format, ##__VA_ARGS__)
#define LOG_ECS_DEBUG(format, ...) LOG(Debug, ECS, format, ##__VA_ARGS__)
#define LOG_ECS_INFO(format, ...)  LOG(Info,  ECS, format, ##__VA_ARGS__)
#define LOG_ECS_WARN(format, ...)  LOG(Warn,  ECS, format, ##__VA_ARGS__)
#define LOG_ECS_ERROR(format, ...) LOG(Error, ECS, format, ##__VA_ARGS__)

//=============================================================================
// Memory 카테고리
//=============================================================================

#define LOG_MEM_TRACE(format, ...) LOG(Trace, Memory, format, ##__VA_ARGS__)
#define LOG_MEM_DEBUG(format, ...) LOG(Debug, Memory, format, ##__VA_ARGS__)
#define LOG_MEM_INFO(format, ...)  LOG(Info,  Memory, format, ##__VA_ARGS__)
#define LOG_MEM_WARN(format, ...)  LOG(Warn,  Memory, format, ##__VA_ARGS__)
#define LOG_MEM_ERROR(format, ...) LOG(Error, Memory, format, ##__VA_ARGS__)

//=============================================================================
// 기타 카테고리 (범용 매크로 사용 권장)
//=============================================================================
// Physics:  LOG(Info, Physics, "Collision detected");
// Platform: LOG(Info, Platform, "Window resized");
// Framework: LOG(Info, Framework, "Application started");
// AI:       LOG(Info, AI, "Path found: %d nodes", nodeCount);
// Audio:    LOG(Info, Audio, "Sound loaded: %s", soundName);
// Input:    LOG(Info, Input, "Key pressed: %d", keyCode);

//=============================================================================
// Release 빌드 최적화: Trace/Debug 제거
//=============================================================================

#ifdef NDEBUG
	// Core
#undef LOG_TRACE
#undef LOG_DEBUG
#define LOG_TRACE(format, ...) ((void)0)
#define LOG_DEBUG(format, ...) ((void)0)

// Graphics
#undef LOG_GFX_TRACE
#undef LOG_GFX_DEBUG
#define LOG_GFX_TRACE(format, ...) ((void)0)
#define LOG_GFX_DEBUG(format, ...) ((void)0)

// ECS
#undef LOG_ECS_TRACE
#undef LOG_ECS_DEBUG
#define LOG_ECS_TRACE(format, ...) ((void)0)
#define LOG_ECS_DEBUG(format, ...) ((void)0)

// Memory
#undef LOG_MEM_TRACE
#undef LOG_MEM_DEBUG
#define LOG_MEM_TRACE(format, ...) ((void)0)
#define LOG_MEM_DEBUG(format, ...) ((void)0)
#endif
