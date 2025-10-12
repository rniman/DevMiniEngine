#include "Platform/Window.h"
#include "Platform/PlatformTypes.h"
#include "Core/Logging/Logger.h"
#include "Core/Logging/ConsoleSink.h"
#include "Core/Logging/LogMacros.h"

int main()
{
    // Initialize logging
    auto& logger = Core::Logging::Logger::GetInstance();
    logger.AddSink(std::make_unique<Core::Logging::ConsoleSink>(true));

    // Create window
    Platform::WindowDesc desc;
    desc.title = "06_WindowTest - DevMiniEngine";
    desc.width = 1280;
    desc.height = 720;
    desc.resizable = true;
    desc.fullscreen = false;

    auto window = Platform::CreatePlatformWindow();
    if (!window->Create(desc))
    {
        LOG_ERROR("Failed to create window");
        return -1;
    }

    LOG_INFO("Window created successfully");
    LOG_INFO("Press ESC or close window to exit");

    // Main loop
    while (!window->ShouldClose())
    {
        window->ProcessEvents();

        // TODO: Render here (Phase 4)
    }

    window->Destroy();
    LOG_INFO("Application terminated");

    return 0;
}