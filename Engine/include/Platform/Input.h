#pragma once

#include "Core/Types.h"
#include "Math/MathTypes.h"
#include "Platform/PlatformTypes.h"

namespace Platform
{
    /**
     * @brief 입력 상태 관리자
     *
     * 키보드와 마우스 상태를 프레임별로 추적합니다.
     * 더블 버퍼링을 사용하여 "pressed", "held", "released" 구분을 지원합니다.
     */
    class Input
    {
    public:
        Input();
        ~Input() = default;

        /**
         * @brief 입력 상태 업데이트
         * @note 매 프레임 시작 시 호출 (이전 상태 복사)
         */
        void Update();

        /**
         * @brief 입력 상태 리셋
         * @note 매 프레임 종료 시 호출 (프레임별 이벤트 초기화)
         */
        void Reset();

        //=============================================================================
        // 키보드 입력
        //=============================================================================

        /**
         * @brief 키가 현재 눌려있는지 확인
         * @param key 확인할 키 코드
         * @return 눌려있으면 true
         */
        bool IsKeyDown(KeyCode key) const;

        /**
         * @brief 키가 이번 프레임에 눌렸는지 확인
         * @param key 확인할 키 코드
         * @return 이번 프레임에 눌렸으면 true (이전 프레임에는 안 눌림)
         */
        bool IsKeyPressed(KeyCode key) const;

        /**
         * @brief 키가 이번 프레임에 떼어졌는지 확인
         * @param key 확인할 키 코드
         * @return 이번 프레임에 떼어졌으면 true (이전 프레임에는 눌림)
         */
        bool IsKeyReleased(KeyCode key) const;

        //=============================================================================
        // 마우스 입력
        //=============================================================================

        /**
         * @brief 마우스 버튼이 현재 눌려있는지 확인
         * @param button 확인할 마우스 버튼
         * @return 눌려있으면 true
         */
        bool IsMouseButtonDown(MouseButton button) const;

        /**
         * @brief 마우스 버튼이 이번 프레임에 눌렸는지 확인
         * @param button 확인할 마우스 버튼
         * @return 이번 프레임에 눌렸으면 true
         */
        bool IsMouseButtonPressed(MouseButton button) const;

        /**
         * @brief 마우스 버튼이 이번 프레임에 떼어졌는지 확인
         * @param button 확인할 마우스 버튼
         * @return 이번 프레임에 떼어졌으면 true
         */
        bool IsMouseButtonReleased(MouseButton button) const;

        /**
         * @brief 현재 마우스 위치 반환
         * @return 스크린 좌표계 기준 마우스 위치
         */
        Math::Vector2 GetMousePosition() const;

        /**
         * @brief 이번 프레임 마우스 이동량 반환
         * @return 마우스 이동 델타 (현재 위치 - 이전 위치)
         */
        Math::Vector2 GetMouseDelta() const;

        /**
         * @brief 이번 프레임 마우스 휠 델타 반환
         * @return 휠 스크롤 값 (정규화: ±1.0)
         */
        float GetMouseWheelDelta() const;

        //=============================================================================
        // 내부 업데이트 메서드 (Window가 호출)
        //=============================================================================

        void OnKeyDown(KeyCode key);
        void OnKeyUp(KeyCode key);
        void OnMouseMove(Core::int32 x, Core::int32 y);
        void OnMouseButtonDown(MouseButton button);
        void OnMouseButtonUp(MouseButton button);
        void OnMouseWheel(Core::float32 delta);

    private:
        // 키보드 상태
        static constexpr Core::uint32 KEY_COUNT = 256;
        bool mKeyState[KEY_COUNT];          // 현재 프레임 상태
        bool mPrevKeyState[KEY_COUNT];      // 이전 프레임 상태

        // 마우스 상태
        bool mMouseButtonState[3];          // 현재 프레임 상태 (Left, Right, Middle)
        bool mPrevMouseButtonState[3];      // 이전 프레임 상태

        Math::Vector2 mMousePosition;
        Math::Vector2 mPrevMousePosition;
        Core::float32 mMouseWheelDelta;
    };

} // namespace Platform