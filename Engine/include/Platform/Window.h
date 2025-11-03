#pragma once

#include "Core/Types.h"
#include "Platform/PlatformTypes.h"
#include "Platform/Input.h"
#include <functional>
#include <memory>

// Windows 타입 전방 선언 (Windows.h 오염 방지)
struct HWND__;
using HWND = HWND__*;

namespace Platform
{
    /**
     * @brief 추상 윈도우 인터페이스
     *
     * 플랫폼별 구현 클래스(Win32Window 등)가 이를 상속합니다.
     */
    class Window
    {
    public:
        using EventCallback = std::function<void(WindowEvent)>;

        virtual ~Window() = default;

        /**
         * @brief 윈도우 생성 및 표시
         * @param desc 윈도우 생성 설정
         * @return 성공 시 true
         */
        virtual bool Create(const WindowDesc& desc) = 0;

        /**
         * @brief 윈도우 파괴
         */
        virtual void Destroy() = 0;

        /**
         * @brief 대기 중인 윈도우 이벤트 처리
         * @note 매 프레임 호출 필요
         */
        virtual void ProcessEvents() = 0;

        /**
         * @brief 윈도우가 닫혀야 하는지 확인
         * @return 닫혀야 하면 true
         */
        virtual bool ShouldClose() const = 0;

        /**
         * @brief 네이티브 윈도우 핸들 반환
         * @return Windows의 경우 HWND
         */
        virtual HWND GetNativeHandle() const = 0;

        /**
         * @brief 윈도우 너비 반환
         * @return 너비 (픽셀)
         */
        virtual Core::uint32 GetWidth() const = 0;

        /**
         * @brief 윈도우 높이 반환
         * @return 높이 (픽셀)
         */
        virtual Core::uint32 GetHeight() const = 0;

        /**
         * @brief 전체화면 모드 확인
         * @return 전체화면이면 true
         */
        virtual bool IsFullscreen() const = 0;

        /**
         * @brief 입력 매니저 반환
         * @return 이 윈도우의 입력 매니저 참조
         */
        virtual Input& GetInput() = 0;

        /**
         * @brief 이벤트 콜백 설정
         * @param callback 윈도우 이벤트 발생 시 호출될 함수
         */
        virtual void SetEventCallback(EventCallback callback) = 0;

    protected:
        Window() = default;

        // 복사 불가
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;
    };

    /**
     * @brief 플랫폼별 윈도우 생성
     * @return 플랫폼에 맞는 Window 구현 (Windows: Win32Window)
     */
    std::unique_ptr<Window> CreatePlatformWindow();

} // namespace Platform