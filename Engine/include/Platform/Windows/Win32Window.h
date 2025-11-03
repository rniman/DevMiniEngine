#pragma once

#include "Core/Types.h"
#include "Platform/PlatformTypes.h"
#include "Platform/Window.h"
#include "Platform/Input.h"

// OS 타입 전방 선언 (Windows.h 오염 방지)
struct HWND__;
struct HINSTANCE__;

namespace Platform
{
    /**
     * @brief Win32 API 윈도우 구현
     *
     * Windows 플랫폼용 Window 인터페이스 구현체입니다.
     *
     * @note 내부 구현 클래스 - 사용자는 Window 인터페이스를 통해 접근
     */
    class Win32Window : public Window
    {
    public:
        Win32Window();
        ~Win32Window() override;

        // Window 인터페이스 구현
        bool Create(const WindowDesc& desc) override;
        void Destroy() override;
        void ProcessEvents() override;
        bool ShouldClose() const override;

        HWND__* GetNativeHandle() const override;
        Core::uint32 GetWidth() const override;
        Core::uint32 GetHeight() const override;
        bool IsFullscreen() const override;

        Input& GetInput() override;

        void SetEventCallback(EventCallback callback) override;

    private:
        // Win32 전용 초기화
        bool RegisterWindowClass();
        void UnregisterWindowClass();

        // Win32 메시지 핸들러 (정적 - OS 콜백)
        static LRESULT CALLBACK WindowProc(
            HWND__* hwnd,
            UINT msg,
            WPARAM wParam,
            LPARAM lParam
        );
        
        // 인스턴스 메시지 핸들러
        LRESULT  HandleMessage(
            UINT msg,
            WPARAM wParam,
            LPARAM lParam
        );

        // 멤버 변수

        // Win32 핸들
        HWND__* mHwnd;           // 윈도우 핸들
        HINSTANCE__* mHInstance; // 애플리케이션 인스턴스

        // 윈도우 상태
        Core::uint32 mWidth;
        Core::uint32 mHeight;
        bool mShouldClose;
        bool mIsFullscreen;

        // 입력 매니저
        Input mInput;

        // 이벤트 콜백
        EventCallback mEventCallback;

        // 윈도우 클래스 이름
        static constexpr const wchar_t* CLASS_NAME = L"DevMiniEngineWindowClass";
    };

} // namespace Platform