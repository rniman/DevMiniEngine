#include "Platform/Window.h"

#ifdef _WIN32
#include "Platform/Windows/Win32Window.h"
#endif

namespace Platform
{
    Core::UniquePtr<Window> CreatePlatformWindow()
    {
#ifdef _WIN32
        return Core::MakeUnique<Win32Window>();
#else
#error "Unsupported platform"
        return nullptr;
#endif
    }

} // namespace Platform