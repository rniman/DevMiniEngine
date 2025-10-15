#include "Graphics/DX12/DX12DescriptorHeap.h"
#include "Core/Logging/LogMacros.h"

namespace Graphics
{
    //=============================================================================
    // Constructor / Destructor
    //=============================================================================

    DX12DescriptorHeap::~DX12DescriptorHeap()
    {
        Shutdown();
    }

    //=============================================================================
    // Public Methods
    //=============================================================================

    bool DX12DescriptorHeap::Initialize(
        ID3D12Device* device,
        D3D12_DESCRIPTOR_HEAP_TYPE type,
        Core::uint32 numDescriptors,
        bool shaderVisible)
    {
        if (!device)
        {
            LOG_ERROR("[DX12DescriptorHeap] Device is null");
            return false;
        }

        if (numDescriptors == 0)
        {
            LOG_ERROR("[DX12DescriptorHeap] Invalid descriptor count: 0");
            return false;
        }

        LOG_INFO("[DX12DescriptorHeap] Initializing Descriptor Heap (%s, %u descriptors)...",
            GetDescriptorHeapTypeString(type), numDescriptors);

        mType = type;
        mNumDescriptors = numDescriptors;

        // Create descriptor heap
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.Type = type;
        heapDesc.NumDescriptors = numDescriptors;
        heapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
            : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        heapDesc.NodeMask = 0;

        HRESULT hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mHeap));
        if (FAILED(hr))
        {
            LOG_ERROR("[DX12DescriptorHeap] Failed to create Descriptor Heap (HRESULT: 0x%08X)", hr);
            return false;
        }

        // Get descriptor size
        mDescriptorSize = device->GetDescriptorHandleIncrementSize(type);

        // Get start handles
        mCPUStart = mHeap->GetCPUDescriptorHandleForHeapStart();
        if (shaderVisible)
        {
            mGPUStart = mHeap->GetGPUDescriptorHandleForHeapStart();
        }

        LOG_INFO("[DX12DescriptorHeap] Descriptor Heap created successfully");
        LOG_INFO("[DX12DescriptorHeap]   Descriptor Size: %u bytes", mDescriptorSize);
        LOG_INFO("[DX12DescriptorHeap]   Shader Visible: %s", shaderVisible ? "Yes" : "No");

        return true;
    }

    void DX12DescriptorHeap::Shutdown()
    {
        if (!IsInitialized())
        {
            return;
        }

        LOG_INFO("[DX12DescriptorHeap] Shutting down Descriptor Heap...");

        mHeap.Reset();
        mCPUStart = {};
        mGPUStart = {};

        LOG_INFO("[DX12DescriptorHeap] Descriptor Heap shut down successfully");
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DX12DescriptorHeap::GetCPUHandle(Core::uint32 index) const
    {
        if (!IsInitialized())
        {
            LOG_ERROR("[DX12DescriptorHeap] Descriptor Heap not initialized");
            return {};
        }

        if (index >= mNumDescriptors)
        {
            LOG_ERROR("[DX12DescriptorHeap] Index out of range: %u (max: %u)", index, mNumDescriptors - 1);
            return {};
        }

        D3D12_CPU_DESCRIPTOR_HANDLE handle = mCPUStart;
        handle.ptr += static_cast<SIZE_T>(index) * mDescriptorSize;
        return handle;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DX12DescriptorHeap::GetGPUHandle(Core::uint32 index) const
    {
        if (!IsInitialized())
        {
            LOG_ERROR("[DX12DescriptorHeap] Descriptor Heap not initialized");
            return {};
        }

        if (index >= mNumDescriptors)
        {
            LOG_ERROR("[DX12DescriptorHeap] Index out of range: %u (max: %u)", index, mNumDescriptors - 1);
            return {};
        }

        D3D12_GPU_DESCRIPTOR_HANDLE handle = mGPUStart;
        handle.ptr += static_cast<UINT64>(index) * mDescriptorSize;
        return handle;
    }

    //=============================================================================
    // Private Methods
    //=============================================================================

    const char* DX12DescriptorHeap::GetDescriptorHeapTypeString(D3D12_DESCRIPTOR_HEAP_TYPE type) const
    {
        switch (type)
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV: return "CBV/SRV/UAV";
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:     return "Sampler";
        case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:         return "RTV";
        case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:         return "DSV";
        default: return "Unknown";
        }
    }

} // namespace Graphics