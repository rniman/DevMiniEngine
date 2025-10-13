#pragma once

#include "Platform/PlatformTypes.h"
#include "Math/MathTypes.h"

namespace Platform
{
    /**
     * @brief Input state manager
     * Tracks keyboard and mouse state per frame
     */
    class Input
    {
    public:
        Input();
        ~Input() = default;

        /**
         * @brief Update input state (call once per frame)
         */
        void Update();

        /**
         * @brief Reset input state (call at end of frame)
         */
        void Reset();

        //=============================================================================
        // Keyboard Input
        //=============================================================================

        /**
         * @brief Check if key is currently held down
         */
        bool IsKeyDown(KeyCode key) const;

        /**
         * @brief Check if key was pressed this frame
         */
        bool IsKeyPressed(KeyCode key) const;

        /**
         * @brief Check if key was released this frame
         */
        bool IsKeyReleased(KeyCode key) const;

        //=============================================================================
        // Mouse Input
        //=============================================================================

        /**
         * @brief Check if mouse button is currently held down
         */
        bool IsMouseButtonDown(MouseButton button) const;

        /**
         * @brief Check if mouse button was pressed this frame
         */
        bool IsMouseButtonPressed(MouseButton button) const;

        /**
         * @brief Check if mouse button was released this frame
         */
        bool IsMouseButtonReleased(MouseButton button) const;

        /**
         * @brief Get current mouse position (screen coordinates)
         */
        Core::Math::Vector2 GetMousePosition() const;

        /**
         * @brief Get mouse movement delta this frame
         */
        Core::Math::Vector2 GetMouseDelta() const;

        /**
         * @brief Get mouse wheel delta this frame
         */
        float GetMouseWheelDelta() const;

        //=============================================================================
        // Internal Update Methods (called by Window)
        //=============================================================================

        void OnKeyDown(KeyCode key);
        void OnKeyUp(KeyCode key);
        void OnMouseMove(int x, int y);
        void OnMouseButtonDown(MouseButton button);
        void OnMouseButtonUp(MouseButton button);
        void OnMouseWheel(float delta);

    private:
        // Keyboard state
        static constexpr size_t KEY_COUNT = 256;
        bool mKeyState[KEY_COUNT];          // Current frame state
        bool mPrevKeyState[KEY_COUNT];      // Previous frame state

        // Mouse state
        bool mMouseButtonState[3];          // Current frame state
        bool mPrevMouseButtonState[3];      // Previous frame state

        Core::Math::Vector2 mMousePosition;
        Core::Math::Vector2 mPrevMousePosition;
        float mMouseWheelDelta;
    };

} // namespace Platform