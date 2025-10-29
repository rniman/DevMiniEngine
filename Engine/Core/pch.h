#pragma once

//=============================================================================
// Windows API
//=============================================================================
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX  // min/max 매크로 충돌 방지 (추천)
	#include <Windows.h>
#endif


//=============================================================================
// 표준 라이브러리
//=============================================================================
#include <cstdint>
#include <cstdlib>

#include <memory>
#include <vector>
#include <array>
#include <string>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <functional>

#include <iostream>
#include <fstream>
#include <sstream>

#include <iomanip>