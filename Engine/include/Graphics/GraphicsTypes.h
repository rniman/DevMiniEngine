#pragma once

/**
 * @file GraphicsTypes.h
 * @brief Graphics 모듈의 공통 타입, 상수, 헬퍼 매크로 정의
 */

 // 외부 라이브러리
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

// d3dx12.h - DirectX 12 헬퍼 라이브러리 (외부 라이브러리 경고 비활성화)
#pragma warning(push, 0)
#include "Graphics/DX12/d3dx12.h"
#pragma warning(pop)

// 프로젝트 헤더
#include "Core/Types.h"
#include "Core/Assert.h"
#include "Core/Logging/LogMacros.h"

// DirectX 12 라이브러리 링크
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

namespace Graphics
{
	//=============================================================================
	// 타입 별칭
	//=============================================================================

	/// @brief COM 객체용 스마트 포인터 (자동 참조 카운팅)
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	//=============================================================================
	// 공통 상수
	//=============================================================================

	/// @brief 스왑 체인 백 버퍼 개수 (더블 버퍼링)
	constexpr Core::uint32 FRAME_BUFFER_COUNT = 2;

	//=============================================================================
	// 헬퍼 매크로
	//=============================================================================

	/**
	 * @brief DirectX 12 HRESULT 검사 및 예외 발생 매크로
	 * @param hr 검사할 HRESULT 값
	 * @param msg 실패 시 로그 및 예외 메시지
	 * @note 초기화 단계에서만 사용, 런타임에서는 로깅 후 복구 권장
	 */
#define GRAPHICS_THROW_IF_FAILED(hr, msg) \
		do { \
			if (FAILED(hr)) { \
				LOG_ERROR("DirectX 12 Error: %s (HRESULT: 0x%08X)", msg, hr); \
				throw std::runtime_error(msg); \
			} \
		} while(0)

// 1. GRAPHICS_ASSERT: 내부 로직 검증 (Debug만, Release에서 제거)
#ifdef _DEBUG
    #define GRAPHICS_ASSERT(condition, message) \
        assert((condition) && (message))
#else
    #define GRAPHICS_ASSERT(condition, message) ((void)0)
#endif

// 2. GRAPHICS_VERIFY: API 검증 (항상 검사, 예외 발생)
#define GRAPHICS_VERIFY(condition, message) \
    do { \
        if (!(condition)) { \
            throw std::runtime_error(message); \
        } \
    } while(0)

} // namespace Graphics

