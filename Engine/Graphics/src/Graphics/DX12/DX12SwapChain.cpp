#include "Graphics/DX12/DX12SwapChain.h"
#include "Core/Logging/LogMacros.h"

namespace Graphics
{
    DX12SwapChain::~DX12SwapChain()
    {
        Shutdown();
    }

    bool DX12SwapChain::Initialize(
        ID3D12Device* device,
        IDXGIFactory4* factory,
        ID3D12CommandQueue* commandQueue,
        HWND hwnd,
        Core::uint32 width,
        Core::uint32 height,
        Core::uint32 bufferCount,
        bool tearingAllowed
    )
    {
        if (!device || !factory || !commandQueue || !hwnd)
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
        mTearingAllowed = tearingAllowed;

        // 1단계: SwapChain 생성
        if (!CreateSwapChain(factory, commandQueue, hwnd))
        {
            LOG_ERROR("[DX12SwapChain] Failed to create SwapChain");
            return false;
        }

        // 2단계: Back Buffer 리소스 획득
        if (!GetBackBufferResources())
        {
            LOG_ERROR("[DX12SwapChain] Failed to get Back Buffer resources");
            return false;
        }

        // 3단계: Descriptor Heap 생성
        if (!CreateDescriptorHeaps(device))
        {
            LOG_ERROR("[DX12SwapChain] Failed to create Descriptor Heaps");
            return false;
        }

        // 4단계: Render Target View 생성
        if (!CreateRenderTargetViews(device))
        {
            LOG_ERROR("[DX12SwapChain] Failed to create Render Target Views");
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

        // RTV Heap
        if (mRTVHeap)
        {
            mRTVHeap->Shutdown();
            mRTVHeap.reset();
        }

        // Back Buffer 해제
        ReleaseBackBuffers();

        // SwapChain 해제
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

        // VSync 간격 설정
        UINT syncInterval = vSync ? 1 : 0;
        // Tearing 플래그 설정
        UINT presentFlags = 0;
        
        if (!vSync && mTearingAllowed) // mIsTearingAllowed는 멤버 변수
        {
            presentFlags = DXGI_PRESENT_ALLOW_TEARING;
        }

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
            // 리사이즈 불필요
            return true;
        }

        LOG_INFO("[DX12SwapChain] Resizing SwapChain from %u x %u to %u x %u",
            mWidth, mHeight, width, height);

        // Back Buffer 해제
        ReleaseBackBuffers();

        // SwapChain 버퍼 리사이즈
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

        // 새로운 Back Buffer 리소스 획득
        if (!GetBackBufferResources())
        {
            LOG_ERROR("[DX12SwapChain] Failed to get Back Buffer resources after resize");
            return false;
        }

        LOG_INFO("[DX12SwapChain] SwapChain resized successfully");
        return true;
    }

    bool DX12SwapChain::CreateSwapChain(
        IDXGIFactory4* factory,
        ID3D12CommandQueue* commandQueue,
        HWND hwnd
    )
    {
        LOG_INFO("[DX12SwapChain] Creating SwapChain...");

        // SwapChain 설정
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
        if (mTearingAllowed)
        {
            swapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
        }

        // SwapChain 생성
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

        // Alt+Enter 전체화면 전환 비활성화
        factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

        // IDXGISwapChain3 인터페이스 쿼리
        hr = swapChain1.As(&mSwapChain);
        if (FAILED(hr))
        {
            LOG_ERROR("[DX12SwapChain] Failed to query IDXGISwapChain3 (HRESULT: 0x%08X)", hr);
            return false;
        }

        // 초기 Back Buffer 인덱스 가져오기
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

            // 디버그 이름 설정
            wchar_t name[32];
            swprintf_s(name, L"BackBuffer[%u]", i);
            mBackBuffers[i]->SetName(name);
        }

        LOG_INFO("[DX12SwapChain] Back Buffer resources acquired (%u buffers)", mBufferCount);
        return true;
    }

    bool DX12SwapChain::CreateDescriptorHeaps(ID3D12Device* device)
    {
        LOG_INFO("[DX12SwapChain] Creating Descriptor Heaps...");

        // RTV Descriptor Heap 생성 (Shader Visible 불필요)
        mRTVHeap = std::make_unique<DX12DescriptorHeap>();
        if (!mRTVHeap->Initialize(
            device,
            D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            FRAME_BUFFER_COUNT,
            false
        ))
        {
            LOG_ERROR("[DX12SwapChain] Failed to create RTV Descriptor Heap");
            return false;
        }

        LOG_INFO("[DX12SwapChain] Descriptor Heaps created successfully");
        return true;
    }

    bool DX12SwapChain::CreateRenderTargetViews(ID3D12Device* device)
    {
        if (!mSwapChain || !mRTVHeap)
        {
            LOG_ERROR("[DX12SwapChain] SwapChain or RTV Heap not initialized");
            return false;
        }

        LOG_INFO("[DX12SwapChain] Creating Render Target Views...");

        // 백 버퍼를 위한 RTV 생성
        for (Core::uint32 i = 0; i < FRAME_BUFFER_COUNT; ++i)
        {
            ID3D12Resource* backBuffer = GetBackBuffer(i);
            if (!backBuffer)
            {
                LOG_ERROR("[DX12SwapChain] Failed to get Back Buffer %u", i);
                return false;
            }

            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mRTVHeap->GetCPUHandle(i);
            device->CreateRenderTargetView(backBuffer, nullptr, rtvHandle);

            LOG_INFO("[DX12SwapChain] RTV created for Back Buffer %u", i);
        }

        LOG_INFO("[DX12SwapChain] Render Target Views created successfully");
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