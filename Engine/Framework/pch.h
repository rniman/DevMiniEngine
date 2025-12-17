#pragma once

// ============================================================================
// 표준 라이브러리
// ============================================================================
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

// ============================================================================
// Windows 플랫폼
// ============================================================================
#ifdef _WIN32
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#include <Windows.h>
	#include <wrl/client.h>  // ComPtr
#endif

// ============================================================================
// DirectX 12
// ============================================================================
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>

// DirectX Math
#include <DirectXMath.h>
#include <DirectXColors.h>

// D3DX12 헬퍼 (경고 억제)
#pragma warning(push, 0)
#include "d3dx12.h"
#pragma warning(pop)

// ============================================================================
// 엔진 Core 타입
// ============================================================================
#include "Core/Types.h"
#include "Core/Logging/LogMacros.h"

// ============================================================================
// 자주 사용되는 Math 타입
// ============================================================================
#include "Math/MathTypes.h"

// ============================================================================
// 유틸리티 매크로
// ============================================================================

// COM 오브젝트 안전 릴리즈
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p) = nullptr; } }
#endif

// 배열 요소 개수
#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

// ============================================================================
// 디버그 설정
// ============================================================================
#ifdef _DEBUG
	#define FRAMEWORK_DEBUG 1
#else
	#define FRAMEWORK_DEBUG 0
#endif

// ============================================================================
// 전역 using 선언 (Framework 모듈 내부용)
// ============================================================================
using Microsoft::WRL::ComPtr;
