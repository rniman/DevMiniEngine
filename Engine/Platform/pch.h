#pragma once

//=============================================================================
// Windows API
//=============================================================================
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#include <windows.h>

	// Windows 라이브러리 링크
	#pragma comment(lib, "user32.lib")
	#pragma comment(lib, "gdi32.lib")
#endif

//=============================================================================
// 표준 라이브러리
//=============================================================================
#include <cstdint>
#include <memory>
#include <string>
#include <vector>