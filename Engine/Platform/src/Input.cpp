#include "Platform/Input.h"
#include <cstring>

namespace Platform
{
    Input::Input()
        : mMouseWheelDelta(0.0f)
    {
        // Initialize all states to false
        std::memset(mKeyState, 0, sizeof(mKeyState));
        std::memset(mPrevKeyState, 0, sizeof(mPrevKeyState));
        std::memset(mMouseButtonState, 0, sizeof(mMouseButtonState));
        std::memset(mPrevMouseButtonState, 0, sizeof(mPrevMouseButtonState));

        mMousePosition = { 0.0f, 0.0f };
        mPrevMousePosition = { 0.0f, 0.0f };
    }

    void Input::Update()
    {
        // Copy current state to previous state
        std::memcpy(mPrevKeyState, mKeyState, sizeof(mKeyState));
        std::memcpy(mPrevMouseButtonState, mMouseButtonState, sizeof(mMouseButtonState));

        mPrevMousePosition = mMousePosition;
    }

    void Input::Reset()
    {
        // Reset frame-specific state
        mMouseWheelDelta = 0.0f;
    }

    //=============================================================================
    // Keyboard Input
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
    // Mouse Input
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

    Core::Math::Vector2 Input::GetMousePosition() const
    {
        return mMousePosition;
    }

    Core::Math::Vector2 Input::GetMouseDelta() const
    {
        return { mMousePosition.x - mPrevMousePosition.x, mMousePosition.y - mPrevMousePosition.y };
    }

    float Input::GetMouseWheelDelta() const
    {
        return mMouseWheelDelta;
    }

    //=============================================================================
    // Internal Update Methods
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