#pragma once

#include "Platform/Window.h"

// Windows.h 전방선언 (헤더 오염 방지)
struct HWND__;
struct HINSTANCE__;

namespace Platform
{
    /**
     * @brief Win32 API window implementation
     * @note Private implementation - not exposed to users
     */
    class Win32Window : public Window
    {
    public:
        Win32Window();
        ~Win32Window() override;

        bool Create(const WindowDesc& desc) override;
        void Destroy() override;
        void ProcessEvents() override;
        bool ShouldClose() const override;

        WindowHandle GetNativeHandle() const override;
        Core::uint32 GetWidth() const override;
        Core::uint32 GetHeight() const override;
        bool IsFullscreen() const override;

        void SetEventCallback(EventCallback callback) override;

    private:
        // Win32 specific initialization
        bool RegisterWindowClass();
        void UnregisterWindowClass();

        // Win32 message handler (static)
        static long long __stdcall WindowProc(
            HWND__* hwnd,
            unsigned int msg,
            unsigned long long wParam,
            long long lParam
        );

        // Instance message handler
        long long HandleMessage(
            unsigned int msg,
            unsigned long long wParam,
            long long lParam
        );

    private:
        // Win32 handles
        HWND__* mHwnd;           // Window handle
        HINSTANCE__* mHInstance; // Application instance

        // Window state
        Core::uint32 mWidth;
        Core::uint32 mHeight;
        bool mShouldClose;
        bool mIsFullscreen;

        // Event callback
        EventCallback mEventCallback;

        // Window class name
        static constexpr const wchar_t* CLASS_NAME = L"DevMiniEngineWindowClass";
    };

} // namespace Platform