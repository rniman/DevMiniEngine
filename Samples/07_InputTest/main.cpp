#include "Platform/Window.h"
#include "Platform/PlatformTypes.h"
#include "Platform/Input.h"
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
    desc.title = "07_InputTest - DevMiniEngine";
    desc.width = 1280;
    desc.height = 720;

    auto window = Platform::CreatePlatformWindow();
    if (!window->Create(desc))
    {
        LOG_ERROR("Failed to create window");
        return -1;
    }

    LOG_INFO("=== Input Test Started ===");
    LOG_INFO("Controls:");
    LOG_INFO("  - Press keys to test keyboard input");
    LOG_INFO("  - Click mouse buttons to test mouse input");
    LOG_INFO("  - Move mouse to test position tracking");
    LOG_INFO("  - Scroll wheel to test wheel input");
    LOG_INFO("  - Press ESC to exit");

    auto& input = window->GetInput();

    // Main loop
    while (!window->ShouldClose())
    {
        // Update input (start of frame)
        input.Update();

        // Process window events
        window->ProcessEvents();

        // Test keyboard input
        if (input.IsKeyPressed(Platform::KeyCode::Escape))
        {
            LOG_INFO("ESC pressed - Exiting");
            break;
        }

        if (input.IsKeyPressed(Platform::KeyCode::Space))
        {
            LOG_INFO("SPACE pressed");
        }

        if (input.IsKeyDown(Platform::KeyCode::W))
        {
            LOG_INFO("W is held down");
        }

        if (input.IsKeyPressed(Platform::KeyCode::A))
        {
            LOG_INFO("A pressed");
        }

        // Test mouse buttons
        if (input.IsMouseButtonPressed(Platform::MouseButton::Left))
        {
            auto pos = input.GetMousePosition();
            LOG_INFO("Left mouse clicked at (%.0f, %.0f)", pos.x, pos.y);
        }

        if (input.IsMouseButtonPressed(Platform::MouseButton::Right))
        {
            LOG_INFO("Right mouse clicked");
        }

        if (input.IsMouseButtonDown(Platform::MouseButton::Middle))
        {
            LOG_INFO("Middle mouse held");
        }

        // Test mouse movement
        auto delta = input.GetMouseDelta();
        if (delta.x != 0.0f || delta.y != 0.0f)
        {
            // Only log significant movement to avoid spam
            if (std::abs(delta.x) > 5.0f || std::abs(delta.y) > 5.0f)
            {
                LOG_TRACE("Mouse moved: delta(%.1f, %.1f)", delta.x, delta.y);
            }
        }

        // Test mouse wheel
        float wheelDelta = input.GetMouseWheelDelta();
        if (wheelDelta != 0.0f)
        {
            LOG_INFO("Mouse wheel: %.2f", wheelDelta);
        }

        // Reset frame-specific state (end of frame)
        input.Reset();
    }

    window->Destroy();
    LOG_INFO("=== Input Test Completed ===");

    return 0;
}