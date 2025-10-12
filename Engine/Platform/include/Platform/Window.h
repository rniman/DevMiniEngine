#pragma once

#include "Platform/PlatformTypes.h"
#include <functional>

namespace Platform
{
    /**
     * @brief Abstract window interface
     * Platform-specific implementations inherit from this.
     */
    class Window
    {
    public:
        using EventCallback = std::function<void(WindowEvent)>;

        virtual ~Window() = default;

        /**
         * @brief Create and show the window
         */
        virtual bool Create(const WindowDesc& desc) = 0;

        /**
         * @brief Destroy the window
         */
        virtual void Destroy() = 0;

        /**
         * @brief Process pending window events
         * @note Call this every frame
         */
        virtual void ProcessEvents() = 0;

        virtual bool ShouldClose() const = 0;

        virtual WindowHandle GetNativeHandle() const = 0;

        virtual Core::uint32 GetWidth() const = 0;
        virtual Core::uint32 GetHeight() const = 0;

        virtual bool IsFullscreen() const = 0;

        /**
         * @brief Set event callback
         */
        virtual void SetEventCallback(EventCallback callback) = 0;

    protected:
        Window() = default;

        // Non-copyable
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;
    };

    /**
     * @brief Create platform-specific window
     */
    Core::UniquePtr<Window> CreatePlatformWindow();

} // namespace Platform