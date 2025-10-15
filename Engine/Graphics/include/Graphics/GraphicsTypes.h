#pragma once

#include "Core/Types.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

// d3dx12.h - DirectX 12 Helper Library
// Disable warnings from external library
#pragma warning(push)
#pragma warning(push, 0)  // 모든 경고 레벨 0으로 (전부 끄기)
#include "Graphics/DX12/d3dx12.h"
#pragma warning(pop)

// Link DirectX 12 libraries
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

namespace Graphics
{
    using Microsoft::WRL::ComPtr;

    //=============================================================================
    // Common Constants
    //=============================================================================

    constexpr Core::uint32 FRAME_BUFFER_COUNT = 2;  // Double buffering

    //=============================================================================
    // Helper Macros
    //=============================================================================

    #define GRAPHICS_THROW_IF_FAILED(hr, msg) \
        if (FAILED(hr)) { \
            LOG_ERROR("DirectX 12 Error: %s (HRESULT: 0x%08X)", msg, hr); \
            throw std::runtime_error(msg); \
        }

} // namespace Graphics