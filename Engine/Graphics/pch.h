#pragma once

//=============================================================================
// Windows API
//=============================================================================
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX  // min/max 매크로 충돌 방지 (추천)
	#include <windows.h>
	#include <wrl/client.h>
#endif

//=============================================================================
// DirectX 12
//=============================================================================
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

//#include <DirectXPackedVector.h>
//#include <DirectXColors.h>
//#include <DirectXCollision.h>

// dxgidebug.h는 나중에 추가
// #ifdef _DEBUG
// #include <dxgidebug.h>
// #endif

#pragma warning(push, 0)
#include "d3dx12.h"
#pragma warning(pop)

// DirectX 라이브러리 링크
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

//=============================================================================
// 표준 라이브러리
//=============================================================================
#include <cstdint>
#include <memory>
#include <vector>
#include <array>
#include <string>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <functional>
#include <stdexcept>  // std::runtime_error


// 멀티스레딩
//#include <thread>
//#include <mutex>
//#include <atomic>

// 성능 측정
//#include <chrono>

// 유틸리티
//#include <utility>
//#include <optional>
//#include <variant>
