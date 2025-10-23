#include "Platform/Window.h"

// ÇÃ·§Æûº° ±¸ÇöÃ¼
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
#error "Unsupported platform: Only Windows is currently supported"
        return nullptr;
#endif
    }

} // namespace Platform