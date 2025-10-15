#include "Platform/Window.h"
#include "Platform/PlatformTypes.h"
#include "Platform/Input.h"
#include "Core/Logging/Logger.h"
#include "Core/Logging/ConsoleSink.h"
#include "Core/Logging/LogMacros.h"

// Graphics headers
#include "Graphics/DX12/DX12Device.h"

int main()
{
    // Initialize logging
    auto& logger = Core::Logging::Logger::GetInstance();
    logger.AddSink(std::make_unique<Core::Logging::ConsoleSink>(true));

    LOG_INFO("=== 08_DX12Init Sample Started ===");

    // Create window
    Platform::WindowDesc windowDesc;
    windowDesc.title = "08_DX12Init - DevMiniEngine";
    windowDesc.width = 1280;
    windowDesc.height = 720;
    windowDesc.resizable = true;

    auto window = Platform::CreatePlatformWindow();
    if (!window->Create(windowDesc))
    {
        LOG_ERROR("Failed to create window");
        return -1;
    }

    LOG_INFO("Window created successfully");

    // Get input manager
    auto& input = window->GetInput();

    // Initialize DirectX 12 Device
    Graphics::DX12Device device;
    if (!device.Initialize(true))  // Enable debug layer in debug builds
    {
        LOG_ERROR("Failed to initialize DirectX 12 Device");
        window->Destroy();
        return -1;
    }

    LOG_INFO("DirectX 12 Device initialized successfully");
    LOG_INFO("Press ESC to exit");

    // TODO: Step 3 - Command Queue 积己
    // TODO: Step 4 - SwapChain 积己
    // TODO: Step 5 - Descriptor Heap 积己
    // TODO: Step 6 - 坊歹傅 风橇

    // Main loop
    while (!window->ShouldClose())
    {
        input.Update();
        window->ProcessEvents();

        if (input.IsKeyPressed(Platform::KeyCode::Escape))
        {
            LOG_INFO("ESC pressed - Exiting");
            break;
        }

        // TODO: Render here

        input.Reset();
    }

    // Shutdown
    device.Shutdown();
    window->Destroy();

    LOG_INFO("=== 08_DX12Init Sample Completed ===");

    return 0;
}