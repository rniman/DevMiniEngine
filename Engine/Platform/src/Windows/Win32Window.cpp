#include "Platform/Windows/Win32Window.h"
#include "Core/Logging/LogMacros.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// Link with Windows libraries
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

namespace Platform
{
    Win32Window::Win32Window()
        : mHwnd(nullptr)
        , mHInstance(nullptr)
        , mWidth(0)
        , mHeight(0)
        , mShouldClose(false)
        , mIsFullscreen(false)
        , mEventCallback(nullptr)
    {
        LOG_TRACE("Win32Window created");
    }

    Win32Window::~Win32Window()
    {
        Destroy();
        LOG_TRACE("Win32Window destroyed");
    }

    bool Win32Window::Create(const WindowDesc& desc)
    {
        LOG_INFO("Creating Win32 window: %s (%dx%d)", desc.title.c_str(), desc.width, desc.height);

        mWidth = desc.width;
        mHeight = desc.height;
        mIsFullscreen = desc.fullscreen;

        // 1. Register window class
        if (!RegisterWindowClass())
        {
            LOG_ERROR("Failed to register window class");
            return false;
        }

        // 2. Calculate window rectangle
        DWORD style = WS_OVERLAPPEDWINDOW;
        DWORD exStyle = WS_EX_APPWINDOW;

        if (mIsFullscreen)
        {
            style = WS_POPUP;
            exStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;
        }
        else if (!desc.resizable)
        {
            style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
        }

        // Adjust for client area size
        RECT rect = { 0, 0, static_cast<LONG>(mWidth), static_cast<LONG>(mHeight) };
        AdjustWindowRectEx(&rect, style, FALSE, exStyle);

        int windowWidth = rect.right - rect.left;
        int windowHeight = rect.bottom - rect.top;

        // Center window on screen
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        int windowX = (screenWidth - windowWidth) / 2;
        int windowY = (screenHeight - windowHeight) / 2;

        // 3. Convert title to wide string
        int wideLength = MultiByteToWideChar(CP_UTF8, 0, desc.title.c_str(), -1, nullptr, 0);
        std::wstring wideTitle(wideLength, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, desc.title.c_str(), -1, &wideTitle[0], wideLength);

        // 4. Create window
        mHwnd = reinterpret_cast<HWND__*>(CreateWindowExW(
            exStyle,
            CLASS_NAME,
            wideTitle.c_str(),
            style,
            windowX, windowY,
            windowWidth, windowHeight,
            nullptr,        // No parent window
            nullptr,        // No menu
            reinterpret_cast<HINSTANCE>(mHInstance),
            this            // Pass 'this' pointer to WM_CREATE
        ));

        if (!mHwnd)
        {
            DWORD error = GetLastError();
            LOG_ERROR("Failed to create window, error code: %lu", error);
            return false;
        }

        // 5. Show window
        ShowWindow(reinterpret_cast<HWND>(mHwnd), SW_SHOW);
        UpdateWindow(reinterpret_cast<HWND>(mHwnd));

        LOG_INFO("Win32 window created successfully");
        return true;
    }

    void Win32Window::Destroy()
    {
        if (mHwnd)
        {
            LOG_TRACE("Destroying Win32 window");
            DestroyWindow(reinterpret_cast<HWND>(mHwnd));
            mHwnd = nullptr;
        }

        UnregisterWindowClass();
    }

    void Win32Window::ProcessEvents()
    {
        MSG msg = {};

        // Non-blocking message processing
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                mShouldClose = true;
            }

            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    bool Win32Window::ShouldClose() const
    {
        return mShouldClose;
    }

    WindowHandle Win32Window::GetNativeHandle() const
    {
        return static_cast<WindowHandle>(mHwnd);
    }

    Core::uint32 Win32Window::GetWidth() const
    {
        return mWidth;
    }

    Core::uint32 Win32Window::GetHeight() const
    {
        return mHeight;
    }

    bool Win32Window::IsFullscreen() const
    {
        return mIsFullscreen;
    }

    void Win32Window::SetEventCallback(EventCallback callback)
    {
        mEventCallback = callback;
    }

    bool Win32Window::RegisterWindowClass()
    {
        mHInstance = GetModuleHandleW(nullptr);
        if (!mHInstance)
        {
            LOG_ERROR("Failed to get module handle");
            return false;
        }

        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = WindowProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = reinterpret_cast<HINSTANCE>(mHInstance);
        wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = nullptr;  // DirectX가 그릴 것이므로 nullptr
        wc.lpszMenuName = nullptr;
        wc.lpszClassName = CLASS_NAME;
        wc.hIconSm = LoadIconW(nullptr, IDI_APPLICATION);

        if (!RegisterClassExW(&wc))
        {
            DWORD error = GetLastError();
            if (error != ERROR_CLASS_ALREADY_EXISTS)
            {
                LOG_ERROR("Failed to register window class, error code: %lu", error);
                return false;
            }
            LOG_WARN("Window class already exists, reusing");
        }

        LOG_TRACE("Window class registered successfully");
        return true;
    }

    void Win32Window::UnregisterWindowClass()
    {
        if (mHInstance)
        {
            UnregisterClassW(CLASS_NAME, reinterpret_cast<HINSTANCE>(mHInstance));
            LOG_TRACE("Window class unregistered");
        }
    }

    long long __stdcall Win32Window::WindowProc(
        HWND__* hwnd,
        unsigned int msg,
        unsigned long long wParam,
        long long lParam)
    {
        Win32Window* window = nullptr;

        if (msg == WM_CREATE)
        {
            // Get 'this' pointer from CreateWindowEx
            CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
            window = reinterpret_cast<Win32Window*>(pCreate->lpCreateParams);

            // Store in window's user data
            SetWindowLongPtrW(
                reinterpret_cast<HWND>(hwnd),
                GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(window)
            );

            window->mHwnd = hwnd;
        }
        else
        {
            // Retrieve stored 'this' pointer
            window = reinterpret_cast<Win32Window*>(GetWindowLongPtrW(reinterpret_cast<HWND>(hwnd), GWLP_USERDATA));
        }

        if (window)
        {
            return window->HandleMessage(msg, wParam, lParam);
        }

        return DefWindowProcW(reinterpret_cast<HWND>(hwnd), msg, wParam, lParam);
    }

    long long Win32Window::HandleMessage(
        unsigned int msg,
        unsigned long long wParam,
        long long lParam)
    {
        switch (msg)
        {
        case WM_CLOSE:
        {
            LOG_INFO("Window close requested");
            mShouldClose = true;
            if (mEventCallback)
            {
                mEventCallback(WindowEvent::Close);
            }
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_SIZE:
        {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);

            if (width != mWidth || height != mHeight)
            {
                mWidth = width;
                mHeight = height;

                LOG_INFO("Window resized: %dx%d", width, height);

                if (mEventCallback)
                {
                    mEventCallback(WindowEvent::Resize);
                }
            }
            return 0;
        }

        case WM_SETFOCUS:
            if (mEventCallback)
            {
                mEventCallback(WindowEvent::Focus);
            }
            return 0;

        case WM_KILLFOCUS:
            if (mEventCallback)
            {
                mEventCallback(WindowEvent::LostFocus);
            }
            return 0;

        default:
            return DefWindowProcW(reinterpret_cast<HWND>(mHwnd), msg, wParam, lParam);
        }
    }

} // namespace Platform