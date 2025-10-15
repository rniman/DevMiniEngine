#include "Graphics/DX12/DX12SwapChain.h"
#include "Core/Logging/LogMacros.h"


namespace Graphics
{
    //=============================================================================
    // Constructor / Destructor
    //=============================================================================

    DX12SwapChain::~DX12SwapChain()
    {
        Shutdown();
    }

    //=============================================================================
    // Public Methods
    //=============================================================================

    bool DX12SwapChain::Initialize(
        IDXGIFactory4* factory,
        ID3D12CommandQueue* commandQueue,
        HWND hwnd,
        Core::uint32 width,
        Core::uint32 height,
        Core::uint32 bufferCount
    )
    {
        if (!factory || !commandQueue || !hwnd)
        {
            LOG_ERROR("[DX12SwapChain] Invalid parameters");
            return false;
        }

        if (width == 0 || height == 0)
        {
            LOG_ERROR("[DX12SwapChain] Invalid dimensions (%u x %u)", width, height);
            return false;
        }

        LOG_INFO("[DX12SwapChain] Initializing SwapChain (%u x %u, %u buffers)...", width, height, bufferCount);

        mWidth = width;
        mHeight = height;
        mBufferCount = bufferCount;
        mFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        mCurrentBackBufferIndex = 0;

        // Step 1: Create SwapChain
        if (!CreateSwapChain(factory, commandQueue, hwnd))
        {
            LOG_ERROR("[DX12SwapChain] Failed to create SwapChain");
            return false;
        }

        // Step 2: Get Back Buffer resources
        if (!GetBackBufferResources())
        {
            LOG_ERROR("[DX12SwapChain] Failed to get Back Buffer resources");
            return false;
        }

        LOG_INFO("[DX12SwapChain] SwapChain initialized successfully");
        return true;
    }

    void DX12SwapChain::Shutdown()
    {
        if (!IsInitialized())
        {
            return;
        }

        LOG_INFO("[DX12SwapChain] Shutting down SwapChain...");

        // Release back buffers
        ReleaseBackBuffers();

        // Release swap chain
        mSwapChain.Reset();

        LOG_INFO("[DX12SwapChain] SwapChain shut down successfully");
    }

    void DX12SwapChain::Present(bool vSync)
    {
        if (!IsInitialized())
        {
            LOG_ERROR("[DX12SwapChain] SwapChain not initialized");
            return;
        }

        // Present: vSync ? 1 : 0
        // Flags: Allow tearing if vSync is disabled
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
        UINT syncInterval = vSync ? 1 : 0;
        UINT presentFlags = (!vSync && mSwapChain->GetDesc1(&swapChainDesc) == S_OK) ? DXGI_PRESENT_ALLOW_TEARING : 0;

        HRESULT hr = mSwapChain->Present(syncInterval, presentFlags);
        if (FAILED(hr))
        {
            LOG_ERROR("[DX12SwapChain] Present failed (HRESULT: 0x%08X)", hr);
        }
    }

    void DX12SwapChain::MoveToNextFrame()
    {
        if (!IsInitialized())
        {
            return;
        }

        mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
    }

    ID3D12Resource* DX12SwapChain::GetCurrentBackBuffer() const
    {
        if (!IsInitialized() || mCurrentBackBufferIndex >= mBackBuffers.size())
        {
            return nullptr;
        }

        return mBackBuffers[mCurrentBackBufferIndex].Get();
    }

    ID3D12Resource* DX12SwapChain::GetBackBuffer(Core::uint32 index) const
    {
        if (!IsInitialized() || index >= mBackBuffers.size())
        {
            return nullptr;
        }

        return mBackBuffers[index].Get();
    }

    bool DX12SwapChain::Resize(Core::uint32 width, Core::uint32 height)
    {
        if (!IsInitialized())
        {
            LOG_ERROR("[DX12SwapChain] SwapChain not initialized");
            return false;
        }

        if (width == 0 || height == 0)
        {
            LOG_ERROR("[DX12SwapChain] Invalid dimensions (%u x %u)", width, height);
            return false;
        }

        if (width == mWidth && height == mHeight)
        {
            // No resize needed
            return true;
        }

        LOG_INFO("[DX12SwapChain] Resizing SwapChain from %u x %u to %u x %u",
            mWidth, mHeight, width, height);

        // Release back buffers
        ReleaseBackBuffers();

        // Resize swap chain buffers
        HRESULT hr = mSwapChain->ResizeBuffers(
            mBufferCount,
            width,
            height,
            mFormat,
            DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
        );

        if (FAILED(hr))
        {
            LOG_ERROR("[DX12SwapChain] Failed to resize SwapChain (HRESULT: 0x%08X)", hr);
            return false;
        }

        mWidth = width;
        mHeight = height;

        // Get new back buffer resources
        if (!GetBackBufferResources())
        {
            LOG_ERROR("[DX12SwapChain] Failed to get Back Buffer resources after resize");
            return false;
        }

        LOG_INFO("[DX12SwapChain] SwapChain resized successfully");
        return true;
    }

    //=============================================================================
    // Private Methods
    //=============================================================================

    bool DX12SwapChain::CreateSwapChain(
        IDXGIFactory4* factory,
        ID3D12CommandQueue* commandQueue,
        HWND hwnd
    )
    {
        LOG_INFO("[DX12SwapChain] Creating SwapChain...");

        // SwapChain description
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = mWidth;
        swapChainDesc.Height = mHeight;
        swapChainDesc.Format = mFormat;
        swapChainDesc.Stereo = FALSE;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = mBufferCount;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

        // Create swap chain
        ComPtr<IDXGISwapChain1> swapChain1;
        HRESULT hr = factory->CreateSwapChainForHwnd(
            commandQueue,
            hwnd,
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain1
        );

        if (FAILED(hr))
        {
            LOG_ERROR("[DX12SwapChain] Failed to create SwapChain (HRESULT: 0x%08X)", hr);
            return false;
        }

        // Disable Alt+Enter full screen toggle
        factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

        // Query IDXGISwapChain3 interface
        hr = swapChain1.As(&mSwapChain);
        if (FAILED(hr))
        {
            LOG_ERROR("[DX12SwapChain] Failed to query IDXGISwapChain3 (HRESULT: 0x%08X)", hr);
            return false;
        }

        // Get initial back buffer index
        mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

        LOG_INFO("[DX12SwapChain] SwapChain created successfully");
        return true;
    }

    bool DX12SwapChain::GetBackBufferResources()
    {
        LOG_INFO("[DX12SwapChain] Getting Back Buffer resources...");

        mBackBuffers.resize(mBufferCount);

        for (Core::uint32 i = 0; i < mBufferCount; ++i)
        {
            HRESULT hr = mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mBackBuffers[i]));
            if (FAILED(hr))
            {
                LOG_ERROR("[DX12SwapChain] Failed to get Back Buffer %u (HRESULT: 0x%08X)", i, hr);
                return false;
            }

            // Set debug name
            wchar_t name[32];
            swprintf_s(name, L"BackBuffer[%u]", i);
            mBackBuffers[i]->SetName(name);
        }

        LOG_INFO("[DX12SwapChain] Back Buffer resources acquired (%u buffers)", mBufferCount);
        return true;
    }

    void DX12SwapChain::ReleaseBackBuffers()
    {
        LOG_INFO("[DX12SwapChain] Releasing Back Buffer resources...");

        for (auto& backBuffer : mBackBuffers)
        {
            backBuffer.Reset();
        }

        mBackBuffers.clear();
    }

} // namespace Graphics