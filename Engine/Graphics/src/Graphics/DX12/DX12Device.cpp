#include "Graphics/DX12/DX12Device.h"
#include "Core/Logging/LogMacros.h"

namespace Graphics
{
    //=============================================================================
    // Constructor / Destructor
    //=============================================================================

    DX12Device::~DX12Device()
    {
        Shutdown();
    }

    //=============================================================================
    // Public Methods
    //=============================================================================

    bool DX12Device::Initialize(bool enableDebugLayer)
    {
        LOG_INFO("[DX12Device] Initializing DirectX 12 Device...");

        // Step 1: Enable Debug Layer (Debug 빌드에서만)
#if defined(_DEBUG)
        if (enableDebugLayer)
        {
            if (!EnableDebugLayer())
            {
                LOG_WARN("[DX12Device] Failed to enable Debug Layer (continuing without it)");
            }
        }
#endif

        // Step 2: Create DXGI Factory
        if (!CreateFactory())
        {
            LOG_ERROR("[DX12Device] Failed to create DXGI Factory");
            return false;
        }

        // Step 3: Select GPU Adapter
        if (!SelectAdapter())
        {
            LOG_ERROR("[DX12Device] Failed to select GPU Adapter");
            return false;
        }

        // Step 4: Create D3D12 Device
        if (!CreateDevice())
        {
            LOG_ERROR("[DX12Device] Failed to create D3D12 Device");
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

        // Release resources in reverse order
        mDevice.Reset();
        mAdapter.Reset();
        mFactory.Reset();

        LOG_INFO("[DX12Device] DirectX 12 Device shut down successfully");
    }

    //=============================================================================
    // Private Methods - Initialization Steps
    //=============================================================================

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

        // Debug 빌드에서 Debug Layer가 활성화된 경우 Factory에도 Debug 플래그 추가
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

        // 모든 Adapter를 순회하며 가장 큰 전용 비디오 메모리를 가진 Adapter 선택
        while (mFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            // Software Adapter는 제외
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                adapterIndex++;
                continue;
            }

            // D3D12 Device 생성 가능 여부 확인
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
            {
                // 더 큰 전용 비디오 메모리를 가진 Adapter 선택
                if (desc.DedicatedVideoMemory > maxDedicatedVideoMemory)
                {
                    maxDedicatedVideoMemory = desc.DedicatedVideoMemory;
                    mAdapter = adapter;

                    // Adapter 정보 로그 출력
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

        // 지원 가능한 Feature Level 목록 (높은 것부터 시도)
        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0
        };

        HRESULT hr = E_FAIL;

        // 가장 높은 Feature Level부터 시도
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

        // Debug 빌드에서 추가 디버그 정보 설정
#if defined(_DEBUG)
        if (mDebugLayerEnabled)
        {
            ComPtr<ID3D12InfoQueue> infoQueue;
            if (SUCCEEDED(mDevice.As(&infoQueue)))
            {
                // Break on severe errors
                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);

                LOG_INFO("[DX12Device] Debug Info Queue configured");
            }
        }
#endif

        LOG_INFO("[DX12Device] D3D12 Device created successfully");
        return true;
    }

    //=============================================================================
    // Helper Methods
    //=============================================================================

    const char* DX12Device::GetFeatureLevelString(D3D_FEATURE_LEVEL featureLevel) const
    {
        switch (featureLevel)
        {
        case D3D_FEATURE_LEVEL_12_1: return "12.1";
        case D3D_FEATURE_LEVEL_12_0: return "12.0";
        case D3D_FEATURE_LEVEL_11_1: return "11.1";
        case D3D_FEATURE_LEVEL_11_0: return "11.0";
        default: return "Unknown";
        }
    }

} // namespace Graphics