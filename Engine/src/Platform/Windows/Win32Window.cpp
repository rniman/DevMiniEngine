#include "pch.h"
#include "Platform/Windows/Win32Window.h"
#include "Core/Logging/LogMacros.h"


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

        // 1. 윈도우 클래스 등록
        if (!RegisterWindowClass())
        {
            LOG_ERROR("Failed to register window class");
            return false;
        }

        // 2. 윈도우 사각형 계산
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

        // 클라이언트 영역 크기 조정
        RECT rect = { 0, 0, static_cast<LONG>(mWidth), static_cast<LONG>(mHeight) };
        AdjustWindowRectEx(&rect, style, FALSE, exStyle);

        Core::int32 windowWidth = rect.right - rect.left;
        Core::int32 windowHeight = rect.bottom - rect.top;

        // 화면 중앙에 윈도우 배치
        Core::int32 screenWidth = GetSystemMetrics(SM_CXSCREEN);
        Core::int32 screenHeight = GetSystemMetrics(SM_CYSCREEN);
        Core::int32 windowX = (screenWidth - windowWidth) / 2;
        Core::int32 windowY = (screenHeight - windowHeight) / 2;

        // 3. 타이틀을 와이드 문자열로 변환
        Core::int32 wideLength = MultiByteToWideChar(CP_UTF8, 0, desc.title.c_str(), -1, nullptr, 0);
        std::wstring wideTitle(wideLength, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, desc.title.c_str(), -1, &wideTitle[0], wideLength);

        // 4. 윈도우 생성
        mHwnd = reinterpret_cast<HWND__*>(CreateWindowExW(
            exStyle,
            CLASS_NAME,
            wideTitle.c_str(),
            style,
            windowX,
            windowY,
            windowWidth,
            windowHeight,
            nullptr,        // 부모 윈도우 없음
            nullptr,        // 메뉴 없음
            reinterpret_cast<HINSTANCE>(mHInstance),
            this            // WM_CREATE에서 사용할 'this' 포인터
        ));

        if (!mHwnd)
        {
            DWORD error = GetLastError();
            LOG_ERROR("Failed to create window, error code: %lu", error);
            return false;
        }

        // 5. 윈도우 표시
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

        // 논블로킹 메시지 처리
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

    HWND Win32Window::GetNativeHandle() const
    {
        return mHwnd;
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

    Input& Win32Window::GetInput()
    {
        return mInput;
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

    LRESULT CALLBACK Win32Window::WindowProc(
        HWND__* hwnd,
        UINT msg,
        WPARAM wParam,
        LPARAM lParam
    )
    {
        Win32Window* window = nullptr;

        if (msg == WM_CREATE)
        {
            // WM_CREATE에서 CreateWindowEx의 마지막 인자로 전달된 'this' 포인터 복구
            CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
            window = reinterpret_cast<Win32Window*>(pCreate->lpCreateParams);

            // 이후 메시지에서 사용하기 위해 윈도우의 GWLP_USERDATA에 저장
            SetWindowLongPtrW(
                reinterpret_cast<HWND>(hwnd),
                GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(window)
            );

            window->mHwnd = hwnd;
        }
        else
        {
            // 다른 메시지에서는 저장된 'this' 포인터 가져오기
            window = reinterpret_cast<Win32Window*>(GetWindowLongPtrW(reinterpret_cast<HWND>(hwnd), GWLP_USERDATA));
        }

        if (window)
        {
            return window->HandleMessage(msg, wParam, lParam);
        }

        return DefWindowProcW(reinterpret_cast<HWND>(hwnd), msg, wParam, lParam);
    }

    LRESULT Win32Window::HandleMessage(
        UINT msg,
        WPARAM wParam,
        LPARAM lParam
    )
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
		{
			PostQuitMessage(0);
			return 0;
		}

		case WM_SIZE:
		{
            Core::uint32 width = LOWORD(lParam);
            Core::uint32 height = HIWORD(lParam);

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
		{
			if (mEventCallback)
			{
				mEventCallback(WindowEvent::Focus);
			}
			return 0;
		}

		case WM_KILLFOCUS:
		{
			if (mEventCallback)
			{
				mEventCallback(WindowEvent::LostFocus);
			}
			return 0;
		}

		//=============================================================================
		// 키보드 메시지
		//=============================================================================
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			KeyCode key = static_cast<KeyCode>(wParam);
			mInput.OnKeyDown(key);
			return 0;
		}

		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			KeyCode key = static_cast<KeyCode>(wParam);
			mInput.OnKeyUp(key);
			return 0;
		}

		//=============================================================================
		// 마우스 메시지
		//=============================================================================
		case WM_MOUSEMOVE:
		{
            // LOWORD/HIWORD는 unsigned short 반환
            // signed short로 변환하여 음수 좌표 처리
            Core::int32 xPos = static_cast<Core::int32>(
                static_cast<Core::int16>(LOWORD(lParam))
                );
            Core::int32 yPos = static_cast<Core::int32>(
                static_cast<Core::int16>(HIWORD(lParam))
                );
            mInput.OnMouseMove(xPos, yPos);
            return 0;
		}

		case WM_LBUTTONDOWN:
		{
			mInput.OnMouseButtonDown(MouseButton::Left);
			return 0;
		}

		case WM_LBUTTONUP:
		{
			mInput.OnMouseButtonUp(MouseButton::Left);
			return 0;
		}

		case WM_RBUTTONDOWN:
		{
			mInput.OnMouseButtonDown(MouseButton::Right);
			return 0;
		}

		case WM_RBUTTONUP:
		{
			mInput.OnMouseButtonUp(MouseButton::Right);
			return 0;
		}

		case WM_MBUTTONDOWN:
		{
			mInput.OnMouseButtonDown(MouseButton::Middle);
			return 0;
		}

		case WM_MBUTTONUP:
		{
			mInput.OnMouseButtonUp(MouseButton::Middle);
			return 0;
		}

		case WM_MOUSEWHEEL:
		{
			Core::int32 delta = GET_WHEEL_DELTA_WPARAM(wParam);
			Core::float32 normalizedDelta = static_cast<Core::float32>(delta) / WHEEL_DELTA;
			mInput.OnMouseWheel(normalizedDelta);
			return 0;
		}

		default:
			return DefWindowProcW(reinterpret_cast<HWND>(mHwnd), msg, wParam, lParam);
		}
    }

} // namespace Platform