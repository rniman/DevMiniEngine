#include "pch.h"
#include "Platform/Window.h"

// 플랫폼별 구현체
#ifdef _WIN32
#include "Platform/Windows/Win32Window.h"
#endif

namespace Platform
{
    std::unique_ptr<Window> CreatePlatformWindow()
    {
#ifdef _WIN32
        return std::make_unique<Win32Window>();
#else
#error "Unsupported platform: Only Windows is currently supported"
        return nullptr;
#endif
    }

} // namespace Platform