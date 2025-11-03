#pragma once

#include "Core/Types.h"
#include "string"

namespace Platform
{
    //=============================================================================
    // 윈도우 설정
    //=============================================================================

    struct WindowDesc
    {
        std::string title = "DevMiniEngine";
        Core::uint32 width = 1280;
        Core::uint32 height = 720;
        bool fullscreen = false;
        bool resizable = true;
        bool vsync = true;
    };

    //=============================================================================
    // 윈도우 이벤트
    //=============================================================================

    enum class WindowEvent
    {
        None,
        Close,
        Resize,
        Focus,
        LostFocus,
        Moved
    };

    //=============================================================================
    // 키보드 키 코드
    //=============================================================================

    enum class KeyCode : Core::uint16
    {
        Unknown = 0,

        // 알파벳 키
        A = 'A', B = 'B', C = 'C', D = 'D', E = 'E',
        F = 'F', G = 'G', H = 'H', I = 'I', J = 'J',
        K = 'K', L = 'L', M = 'M', N = 'N', O = 'O',
        P = 'P', Q = 'Q', R = 'R', S = 'S', T = 'T',
        U = 'U', V = 'V', W = 'W', X = 'X', Y = 'Y',
        Z = 'Z',

        // 숫자 키
        Num0 = '0', Num1 = '1', Num2 = '2', Num3 = '3', Num4 = '4',
        Num5 = '5', Num6 = '6', Num7 = '7', Num8 = '8', Num9 = '9',

        // 기능 키
        F1 = 0x70, F2, F3, F4, F5, F6,
        F7, F8, F9, F10, F11, F12,

        // 특수 키
        Escape = 0x1B,
        Space = 0x20,
        Enter = 0x0D,
        Tab = 0x09,
        Backspace = 0x08,
        Delete = 0x2E,

        // 방향 키
        Left = 0x25,
        Up = 0x26,
        Right = 0x27,
        Down = 0x28,

        // 수식 키
        Shift = 0x10,
        Control = 0x11,
        Alt = 0x12
    };

    //=============================================================================
    // 마우스 버튼 코드
    //=============================================================================

    enum class MouseButton : Core::uint8
    {
        Left = 0,
        Right = 1,
        Middle = 2
    };

} // namespace Platform