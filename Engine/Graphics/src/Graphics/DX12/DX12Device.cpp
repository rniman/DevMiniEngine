#include "Graphics/DX12/DX12Device.h"
#include "Graphics/DX12/DX12CommandQueue.h"
#include "Graphics/DX12/DX12SwapChain.h"
#include "Graphics/DX12/DX12DescriptorHeap.h" 
#include "Graphics/DX12/DX12CommandContext.h"
#include "Core/Logging/LogMacros.h"

namespace Graphics
{
    DX12Device::~DX12Device()
    {
        Shutdown();
    }

    bool DX12Device::Initialize(bool enableDebugLayer)
    {
        LOG_INFO("[DX12Device] Initializing DirectX 12 Device...");

        // 1단계: Debug Layer 활성화 (Debug 빌드에서만)
#if defined(_DEBUG)
        if (enableDebugLayer)
        {
            if (!EnableDebugLayer())
            {
                LOG_WARN("[DX12Device] Failed to enable Debug Layer (continuing without it)");
            }
        }
#endif

        // 2단계: DXGI Factory 생성
        if (!CreateFactory())
        {
            LOG_ERROR("[DX12Device] Failed to create DXGI Factory");
            return false;
        }

        // 3단계: GPU Adapter 선택
        if (!SelectAdapter())
        {
            LOG_ERROR("[DX12Device] Failed to select GPU Adapter");
            return false;
        }

        // 4단계: D3D12 Device 생성
        if (!CreateDevice())
        {
            LOG_ERROR("[DX12Device] Failed to create D3D12 Device");
            return false;
        }

        // 5단계: Command Queue 생성
        if (!CreateCommandQueues())
        {
            LOG_ERROR("[DX12Device] Failed to create Command Queues");
            return false;
        }

        LOG_INFO("[DX12Device] DirectX 12 Device initialized successfully");
        LOG_INFO("[DX12Device] Feature Level: %s", GetFeatureLevelString(mFeatureLevel));

        return true;
    }

    void DX12Device::Shutdown()
    {
        if (!IsInitialized())
        {
            return;
        }

        LOG_INFO("[DX12Device] Shutting down DirectX 12 Device...");

        // GPU 작업 완료 대기
        if (mGraphicsQueue)
        {
            mGraphicsQueue->WaitForIdle();
        }

        // 역순으로 리소스 해제
        // Command Context
        for (auto& context : mCommandContexts)
        {
            if (context)
            {
                context->Shutdown();
                context.reset();
            }
        }

        // RTV Heap
        if (mRTVHeap)
        {
            mRTVHeap->Shutdown();
            mRTVHeap.reset();
        }

        // SwapChain
        if (mSwapChain)
        {
            mSwapChain->Shutdown();
            mSwapChain.reset();
        }

        // Command Queue
        if (mGraphicsQueue)
        {
            mGraphicsQueue->Shutdown();
            mGraphicsQueue.reset();
        }

        // DirectX 객체
        mDevice.Reset();
        mAdapter.Reset();
        mFactory.Reset();

        LOG_INFO("[DX12Device] DirectX 12 Device shut down successfully");
    }

    DX12CommandContext* DX12Device::GetCommandContext(Core::uint32 index) const
    {
        if (index >= FRAME_BUFFER_COUNT)
        {
            LOG_ERROR("[DX12Device] Command Context index out of range: %u", index);
            return nullptr;
        }

        return mCommandContexts[index].get();
    }

    bool DX12Device::CreateSwapChain(HWND hwnd, Core::uint32 width, Core::uint32 height)
    {
        if (!IsInitialized())
        {
            LOG_ERROR("[DX12Device] Device not initialized");
            return false;
        }

        if (!mGraphicsQueue || !mGraphicsQueue->IsInitialized())
        {
            LOG_ERROR("[DX12Device] Graphics Command Queue not initialized");
            return false;
        }

        LOG_INFO("[DX12Device] Creating SwapChain...");

        // 1단계: SwapChain 생성
        mSwapChain = std::make_unique<DX12SwapChain>();
        if (!mSwapChain->Initialize(
            mFactory.Get(),
            mGraphicsQueue->GetQueue(),
            hwnd,
            width,
            height,
            FRAME_BUFFER_COUNT
        ))
        {
            LOG_ERROR("[DX12Device] Failed to create SwapChain");
            mSwapChain.reset();
            return false;
        }

        // 2단계: Descriptor Heap 생성
        if (!CreateDescriptorHeaps())
        {
            LOG_ERROR("[DX12Device] Failed to create Descriptor Heaps");
            return false;
        }

        // 3단계: Render Target View 생성
        if (!CreateRenderTargetViews())
        {
            LOG_ERROR("[DX12Device] Failed to create Render Target Views");
            return false;
        }

        // 4단계: Command Context 생성
        if (!CreateCommandContexts())
        {
            LOG_ERROR("[DX12Device] Failed to create Command Contexts");
            return false;
        }

        LOG_INFO("[DX12Device] SwapChain created successfully");
        return true;
    }

    bool DX12Device::EnableDebugLayer()
    {
#if defined(_DEBUG)
        LOG_INFO("[DX12Device] Enabling Debug Layer...");

        ComPtr<ID3D12Debug> debugController;
        HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));

        if (SUCCEEDED(hr))
        {
            debugController->EnableDebugLayer();
            mDebugLayerEnabled = true;
            LOG_INFO("[DX12Device] Debug Layer enabled successfully");
            return true;
        }
        else
        {
            LOG_WARN("[DX12Device] Failed to enable Debug Layer (HRESULT: 0x%08X)", hr);
            return false;
        }
#else
        return false;
#endif
    }

    bool DX12Device::CreateFactory()
    {
        LOG_INFO("[DX12Device] Creating DXGI Factory...");

        UINT factoryFlags = 0;

        // Debug 빌드에서 Debug 플래그 추가
#if defined(_DEBUG)
        if (mDebugLayerEnabled)
        {
            factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
#endif

        HRESULT hr = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&mFactory));

        if (FAILED(hr))
        {
            LOG_ERROR("[DX12Device] Failed to create DXGI Factory (HRESULT: 0x%08X)", hr);
            return false;
        }

        LOG_INFO("[DX12Device] DXGI Factory created successfully");
        return true;
    }

    bool DX12Device::SelectAdapter()
    {
        LOG_INFO("[DX12Device] Selecting GPU Adapter...");

        ComPtr<IDXGIAdapter1> adapter;
        SIZE_T maxDedicatedVideoMemory = 0;
        UINT adapterIndex = 0;

        // 최대 VRAM을 가진 Adapter 선택
        while (mFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            // Software Adapter 제외
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                adapterIndex++;
                continue;
            }

            // Device 생성 가능 여부 및 VRAM 크기 확인
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
            {
                if (desc.DedicatedVideoMemory > maxDedicatedVideoMemory)
                {
                    maxDedicatedVideoMemory = desc.DedicatedVideoMemory;
                    mAdapter = adapter;

                    // Adapter 정보 출력
                    char adapterName[128];
                    size_t convertedChars = 0;
                    wcstombs_s(&convertedChars, adapterName, sizeof(adapterName), desc.Description, _TRUNCATE);

                    LOG_INFO("[DX12Device] Found compatible adapter: %s", adapterName);
                    LOG_INFO("[DX12Device]   Dedicated Video Memory: %.2f GB", desc.DedicatedVideoMemory / (1024.0 * 1024.0 * 1024.0));
                    LOG_INFO("[DX12Device]   Dedicated System Memory: %.2f MB", desc.DedicatedSystemMemory / (1024.0 * 1024.0));
                    LOG_INFO("[DX12Device]   Shared System Memory: %.2f MB", desc.SharedSystemMemory / (1024.0 * 1024.0));
                }
            }

            adapterIndex++;
        }

        if (!mAdapter)
        {
            LOG_ERROR("[DX12Device] No compatible GPU adapter found");
            return false;
        }

        LOG_INFO("[DX12Device] GPU Adapter selected successfully");
        return true;
    }

    bool DX12Device::CreateDevice()
    {
        LOG_INFO("[DX12Device] Creating D3D12 Device...");

        // Feature Level 목록 (높은 순서)
        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0
        };

        HRESULT hr = E_FAIL;

        // 최대 Feature Level 감지
        for (D3D_FEATURE_LEVEL featureLevel : featureLevels)
        {
            hr = D3D12CreateDevice(
                mAdapter.Get(),
                featureLevel,
                IID_PPV_ARGS(&mDevice)
            );

            if (SUCCEEDED(hr))
            {
                mFeatureLevel = featureLevel;
                LOG_INFO("[DX12Device] D3D12 Device created with Feature Level: %s", GetFeatureLevelString(featureLevel));
                break;
            }
        }

        if (FAILED(hr))
        {
            LOG_ERROR("[DX12Device] Failed to create D3D12 Device (HRESULT: 0x%08X)", hr);
            return false;
        }

        // Debug 빌드 전용: Info Queue 설정
#if defined(_DEBUG)
        if (mDebugLayerEnabled)
        {
            ComPtr<ID3D12InfoQueue> infoQueue;
            if (SUCCEEDED(mDevice.As(&infoQueue)))
            {
                // 심각한 에러 발생 시 중단
                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);

                LOG_INFO("[DX12Device] Debug Info Queue configured");
            }
        }
#endif

        LOG_INFO("[DX12Device] D3D12 Device created successfully");
        return true;
    }

    bool DX12Device::CreateCommandQueues()
    {
        LOG_INFO("[DX12Device] Creating Command Queues...");

        // Graphics Command Queue 생성
        mGraphicsQueue = std::make_unique<DX12CommandQueue>();
        if (!mGraphicsQueue->Initialize(mDevice.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT))
        {
            LOG_ERROR("[DX12Device] Failed to create Graphics Command Queue");
            return false;
        }

        LOG_INFO("[DX12Device] Command Queues created successfully");
        return true;
    }

    bool DX12Device::CreateDescriptorHeaps()
    {
        LOG_INFO("[DX12Device] Creating Descriptor Heaps...");

        // RTV Descriptor Heap 생성 (Shader Visible 불필요)
        mRTVHeap = std::make_unique<DX12DescriptorHeap>();
        if (!mRTVHeap->Initialize(
            mDevice.Get(),
            D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            FRAME_BUFFER_COUNT,
            false
        ))
        {
            LOG_ERROR("[DX12Device] Failed to create RTV Descriptor Heap");
            return false;
        }

        LOG_INFO("[DX12Device] Descriptor Heaps created successfully");
        return true;
    }

    bool DX12Device::CreateRenderTargetViews()
    {
        if (!mSwapChain || !mRTVHeap)
        {
            LOG_ERROR("[DX12Device] SwapChain or RTV Heap not initialized");
            return false;
        }

        LOG_INFO("[DX12Device] Creating Render Target Views...");

        // 백 버퍼를 위한 RTV 생성
        for (Core::uint32 i = 0; i < FRAME_BUFFER_COUNT; ++i)
        {
            ID3D12Resource* backBuffer = mSwapChain->GetBackBuffer(i);
            if (!backBuffer)
            {
                LOG_ERROR("[DX12Device] Failed to get Back Buffer %u", i);
                return false;
            }

            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mRTVHeap->GetCPUHandle(i);
            mDevice->CreateRenderTargetView(backBuffer, nullptr, rtvHandle);

            LOG_INFO("[DX12Device] RTV created for Back Buffer %u", i);
        }

        LOG_INFO("[DX12Device] Render Target Views created successfully");
        return true;
    }

    bool DX12Device::CreateCommandContexts()
    {
        LOG_INFO("[DX12Device] Creating Command Contexts...");

        for (Core::uint32 i = 0; i < FRAME_BUFFER_COUNT; ++i)
        {
            mCommandContexts[i] = std::make_unique<DX12CommandContext>();
            if (!mCommandContexts[i]->Initialize(mDevice.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT))
            {
                LOG_ERROR("[DX12Device] Failed to create Command Context %u", i);
                return false;
            }

            LOG_INFO("[DX12Device] Command Context %u created", i);
        }

        LOG_INFO("[DX12Device] Command Contexts created successfully");
        return true;
    }

    const char* DX12Device::GetFeatureLevelString(D3D_FEATURE_LEVEL featureLevel) const
    {
        switch (featureLevel)
        {
        case D3D_FEATURE_LEVEL_12_1:
            return "12.1";
        case D3D_FEATURE_LEVEL_12_0:
            return "12.0";
        case D3D_FEATURE_LEVEL_11_1:
            return "11.1";
        case D3D_FEATURE_LEVEL_11_0:
            return "11.0";
        default:
            return "Unknown";
        }
    }

} // namespace Graphics