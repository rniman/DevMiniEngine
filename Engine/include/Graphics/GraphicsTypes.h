#pragma once

//=============================================================================
// 외부 헤더
//=============================================================================
#include <wrl/client.h>  // ComPtr

//=============================================================================
// 프로젝트 헤더
//=============================================================================
#include "Core/Types.h"
#include "Core/Assert.h"
#include "Core/Logging/LogMacros.h"

//=============================================================================
// DirectX 전방 선언
//=============================================================================
struct ID3D12Device;
struct ID3D12Device5;
struct ID3D12CommandQueue;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;
struct ID3D12Resource;
struct ID3D12DescriptorHeap;
struct ID3D12RootSignature;
struct ID3D12PipelineState;
struct ID3D12Fence;
struct IDXGISwapChain3;
struct IDXGIFactory4;
struct IDXGIAdapter1;

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
     * @brief DirectX 12 HRESULT 검사 및 예외 발생 매크로
     * @param hr 검사할 HRESULT 값
     * @param msg 실패 시 로그 및 예외 메시지
     * @note 초기화 단계에서만 사용, 런타임에서는 로깅 후 복구 권장
     */
#define GRAPHICS_THROW_IF_FAILED(hr, msg) \
        do { \
            HRESULT _hr = (hr); \
            if (FAILED(_hr)) { \
                LOG_ERROR("DirectX 12 Error: %s (HRESULT: 0x%08X)", msg, static_cast<unsigned int>(_hr)); \
                throw std::runtime_error(msg); \
            } \
        } while(0)

     /**
      * @brief 내부 로직 검증 (Debug만, Release에서 제거)
      */
#ifdef _DEBUG
    #define GRAPHICS_ASSERT(condition, message) \
                CORE_ASSERT(condition, message)
    #else
    #define GRAPHICS_ASSERT(condition, message) ((void)0)
#endif

      /**
       * @brief API 검증 (항상 검사, 예외 발생)
       */
#define GRAPHICS_VERIFY(condition, message) \
        do { \
            if (!(condition)) { \
                LOG_ERROR("Graphics Verification Failed: %s", message); \
                throw std::runtime_error(message); \
            } \
        } while(0)

} // namespace Graphics