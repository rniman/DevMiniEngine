#pragma once

//=============================================================================
// 프로젝트 헤더
//=============================================================================
#include "Core/Types.h"
#include "Core/Assert.h"
#include "Core/Logging/LogMacros.h"

//=============================================================================
// DirectX 12 Core
//=============================================================================
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

//=============================================================================
// DirectX Math
//=============================================================================
#include <DirectXMath.h>
// 필요 시 추가:
// #include <DirectXPackedVector.h>
// #include <DirectXColors.h>
// #include <DirectXCollision.h>

//=============================================================================
// DirectX Helper (외부 라이브러리 - 경고 억제)
//=============================================================================
#pragma warning(push, 0)
#include "d3dx12.h"
#pragma warning(pop)

//=============================================================================
// DirectX Debug (Debug 빌드 전용)
//=============================================================================
#ifdef _DEBUG
	// 향후 메모리 누수 검사용
	// #include <dxgidebug.h>
#endif

//=============================================================================
// DirectX 라이브러리 링크
//=============================================================================
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace Graphics
{
	//=============================================================================
	// 타입 별칭
	//=============================================================================

	/// @brief COM 객체용 스마트 포인터 (자동 참조 카운팅)
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	// Core 타입 가져오기
	using Core::uint8;
	using Core::uint16;
	using Core::uint32;
	using Core::uint64;
	using Core::int8;
	using Core::int16;
	using Core::int32;
	using Core::int64;
	using Core::float32;
	using Core::float64;
	using Core::size_t;
	using Core::byte;

	//=============================================================================
	// 공통 상수
	//=============================================================================

	/// @brief 스왑 체인 백 버퍼 개수 (더블 버퍼링)
	constexpr uint32 FRAME_BUFFER_COUNT = 2;

	/// @brief 최대 렌더 타겟 개수
	constexpr uint32 MAX_RENDER_TARGETS = 8;

	/// @brief 디스크립터 힙 기본 크기
	constexpr uint32 DEFAULT_DESCRIPTOR_HEAP_SIZE = 1000;

	//=============================================================================
	// 헬퍼 매크로
	//=============================================================================

	/**
	 * @brief DirectX 12 HRESULT 검사 매크로
	 *
	 * @param hr 검사할 HRESULT 값
	 * @param msg 실패 시 출력할 메시지
	 *
	 * @note 초기화 단계에서만 사용 권장
	 * @note 런타임에서는 로깅 후 복구 로직 구현 권장
	 *
	 * @example
	 * GRAPHICS_THROW_IF_FAILED(
	 *     device->CreateCommandQueue(&desc, IID_PPV_ARGS(&queue)),
	 *     "Failed to create command queue"
	 * );
	 */
#define GRAPHICS_THROW_IF_FAILED(hr, msg) \
        do { \
            HRESULT _hr = (hr); \
            if (FAILED(_hr)) { \
                LOG_ERROR("[Graphics] %s (HRESULT: 0x%08X)", msg, static_cast<unsigned int>(_hr)); \
                throw std::runtime_error(msg); \
            } \
        } while(0)

	 /**
	  * @brief 내부 로직 검증 매크로 (Debug 빌드만)
	  *
	  * @param condition 검증할 조건
	  * @param message 실패 시 메시지
	  *
	  * @note Release 빌드에서는 완전히 제거됨
	  * @note 성능에 영향을 주는 검증에 사용
	  */
#ifdef _DEBUG
#define GRAPHICS_ASSERT(condition, message) \
            CORE_ASSERT(condition, message)
#else
#define GRAPHICS_ASSERT(condition, message) ((void)0)
#endif

	  /**
	   * @brief API 파라미터 검증 매크로 (항상 검사)
	   *
	   * @param condition 검증할 조건
	   * @param message 실패 시 메시지
	   *
	   * @note Release 빌드에서도 검사됨
	   * @note 사용자 입력 검증 등에 사용
	   */
#define GRAPHICS_VERIFY(condition, message) \
        do { \
            if (!(condition)) { \
                LOG_ERROR("[Graphics] Verification Failed: %s", message); \
                throw std::runtime_error(message); \
            } \
        } while(0)

} // namespace Graphics
