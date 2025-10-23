#include "Platform/Input.h"
#include <cstring>

namespace Platform
{
    Input::Input()
        : mMouseWheelDelta(0.0f)
    {
        // 모든 상태를 false로 초기화
        std::memset(mKeyState, 0, sizeof(mKeyState));
        std::memset(mPrevKeyState, 0, sizeof(mPrevKeyState));
        std::memset(mMouseButtonState, 0, sizeof(mMouseButtonState));
        std::memset(mPrevMouseButtonState, 0, sizeof(mPrevMouseButtonState));

        mMousePosition = { 0.0f, 0.0f };
        mPrevMousePosition = { 0.0f, 0.0f };
    }

    void Input::Update()
    {
        // 현재 상태를 이전 상태로 복사 (더블 버퍼링)
        std::memcpy(mPrevKeyState, mKeyState, sizeof(mKeyState));
        std::memcpy(mPrevMouseButtonState, mMouseButtonState, sizeof(mMouseButtonState));

        mPrevMousePosition = mMousePosition;
    }

    void Input::Reset()
    {
        // 프레임별 상태 리셋 (마우스 휠 등)
        mMouseWheelDelta = 0.0f;
    }

    //=============================================================================
    // 키보드 입력
    //=============================================================================

    bool Input::IsKeyDown(KeyCode key) const
    {
        size_t index = static_cast<size_t>(key);
        if (index >= KEY_COUNT)
        {
            return false;
        }

        return mKeyState[index];
    }

    bool Input::IsKeyPressed(KeyCode key) const
    {
        size_t index = static_cast<size_t>(key);
        if (index >= KEY_COUNT)
        {
            return false;
        }

        return mKeyState[index] && !mPrevKeyState[index];
    }

    bool Input::IsKeyReleased(KeyCode key) const
    {
        size_t index = static_cast<size_t>(key);
        if (index >= KEY_COUNT)
        {
            return false;
        }

        return !mKeyState[index] && mPrevKeyState[index];
    }

    //=============================================================================
    // 마우스 입력
    //=============================================================================

    bool Input::IsMouseButtonDown(MouseButton button) const
    {
        size_t index = static_cast<size_t>(button);
        if (index >= 3)
        {
            return false;
        }

        return mMouseButtonState[index];
    }

    bool Input::IsMouseButtonPressed(MouseButton button) const
    {
        size_t index = static_cast<size_t>(button);
        if (index >= 3)
        {
            return false;
        }

        return mMouseButtonState[index] && !mPrevMouseButtonState[index];
    }

    bool Input::IsMouseButtonReleased(MouseButton button) const
    {
        size_t index = static_cast<size_t>(button);
        if (index >= 3)
        {
            return false;
        }

        return !mMouseButtonState[index] && mPrevMouseButtonState[index];
    }

    Math::Vector2 Input::GetMousePosition() const
    {
        return mMousePosition;
    }

    Math::Vector2 Input::GetMouseDelta() const
    {
        return { mMousePosition.x - mPrevMousePosition.x, mMousePosition.y - mPrevMousePosition.y };
    }

    float Input::GetMouseWheelDelta() const
    {
        return mMouseWheelDelta;
    }

    //=============================================================================
    // 내부 업데이트 메서드
    //=============================================================================

    void Input::OnKeyDown(KeyCode key)
    {
        size_t index = static_cast<size_t>(key);
        if (index < KEY_COUNT)
        {
            mKeyState[index] = true;
        }
    }

    void Input::OnKeyUp(KeyCode key)
    {
        size_t index = static_cast<size_t>(key);
        if (index < KEY_COUNT)
        {
            mKeyState[index] = false;
        }
    }

    void Input::OnMouseMove(int x, int y)
    {
        mMousePosition.x = static_cast<float>(x);
        mMousePosition.y = static_cast<float>(y);
    }

    void Input::OnMouseButtonDown(MouseButton button)
    {
        size_t index = static_cast<size_t>(button);
        if (index < 3)
        {
            mMouseButtonState[index] = true;
        }
    }

    void Input::OnMouseButtonUp(MouseButton button)
    {
        size_t index = static_cast<size_t>(button);
        if (index < 3)
        {
            mMouseButtonState[index] = false;
        }
    }

    void Input::OnMouseWheel(float delta)
    {
        mMouseWheelDelta = delta;
    }

} // namespace Platform