#include "Graphics/DX12/DX12CommandContext.h"
#include "Core/Logging/LogMacros.h"

namespace Graphics
{
    //=============================================================================
    // Constructor / Destructor
    //=============================================================================

    DX12CommandContext::~DX12CommandContext()
    {
        Shutdown();
    }

    //=============================================================================
    // Public Methods
    //=============================================================================

    bool DX12CommandContext::Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
    {
        if (!device)
        {
            LOG_ERROR("[DX12CommandContext] Device is null");
            return false;
        }

        mType = type;

        // Create Command Allocator
        HRESULT hr = device->CreateCommandAllocator(type, IID_PPV_ARGS(&mCommandAllocator));
        if (FAILED(hr))
        {
            LOG_ERROR("[DX12CommandContext] Failed to create Command Allocator (HRESULT: 0x%08X)", hr);
            return false;
        }

        // Create Command List
        hr = device->CreateCommandList(
            0,
            type,
            mCommandAllocator.Get(),
            nullptr,
            IID_PPV_ARGS(&mCommandList)
        );

        if (FAILED(hr))
        {
            LOG_ERROR("[DX12CommandContext] Failed to create Command List (HRESULT: 0x%08X)", hr);
            return false;
        }

        // Close command list (생성 시 열린 상태)
        mCommandList->Close();

        return true;
    }

    void DX12CommandContext::Shutdown()
    {
        if (!IsInitialized())
        {
            return;
        }

        mCommandList.Reset();
        mCommandAllocator.Reset();
    }

    bool DX12CommandContext::Reset()
    {
        if (!IsInitialized())
        {
            LOG_ERROR("[DX12CommandContext] Command Context not initialized");
            return false;
        }

        // Reset allocator
        HRESULT hr = mCommandAllocator->Reset();
        if (FAILED(hr))
        {
            LOG_ERROR("[DX12CommandContext] Failed to reset Command Allocator (HRESULT: 0x%08X)", hr);
            return false;
        }

        // Reset command list
        hr = mCommandList->Reset(mCommandAllocator.Get(), nullptr);
        if (FAILED(hr))
        {
            LOG_ERROR("[DX12CommandContext] Failed to reset Command List (HRESULT: 0x%08X)", hr);
            return false;
        }

        return true;
    }

    bool DX12CommandContext::Close()
    {
        if (!IsInitialized())
        {
            LOG_ERROR("[DX12CommandContext] Command Context not initialized");
            return false;
        }

        HRESULT hr = mCommandList->Close();
        if (FAILED(hr))
        {
            LOG_ERROR("[DX12CommandContext] Failed to close Command List (HRESULT: 0x%08X)", hr);
            return false;
        }

        return true;
    }

} // namespace Graphics