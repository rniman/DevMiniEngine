#include "pch.h"
#include "Graphics/DX12/DX12DepthStencilBuffer.h"
#include "Core/Logging/LogMacros.h"

namespace Graphics
{

    DX12DepthStencilBuffer::DX12DepthStencilBuffer()
        : mFormat(DXGI_FORMAT_UNKNOWN)
        , mCurrentState(D3D12_RESOURCE_STATE_COMMON)
        , mWidth(0)
        , mHeight(0)
        , mDSVHandle{}
    {
    }

    DX12DepthStencilBuffer::~DX12DepthStencilBuffer()
    {
        Shutdown();
    }

    bool DX12DepthStencilBuffer::Initialize(
        ID3D12Device* device,
        uint32 width,
        uint32 height,
        DXGI_FORMAT format
    )
    {
        // DepthStencilBufferDesc 생성하여 위임
        DepthStencilBufferDesc desc;
        desc.width = width;
        desc.height = height;
        desc.format = format;
        desc.enableShaderResource = false;
        desc.sampleCount = 1;
        desc.sampleQuality = 0;

        return Initialize(device, desc);
    }

    bool DX12DepthStencilBuffer::Initialize(
        ID3D12Device* device,
        const DepthStencilBufferDesc& desc
    )
    {
        // 파라미터 검증
        if (!device || desc.width == 0 || desc.height == 0)
        {
            LOG_ERROR(
                "Invalid parameters: device=%p, width=%u, height=%u",
                device,
                desc.width,
                desc.height
            );
            return false;
        }

        if (!IsValidDepthStencilFormat(desc.format))
        {
            LOG_ERROR("Invalid Depth-Stencil format: %d", desc.format);
            return false;
        }

        // 현재 단계에서는 기본 기능만 지원
        if (desc.enableShaderResource)
        {
            LOG_ERROR("Shader Resource View (SRV) not yet supported");
            return false;
        }

        if (desc.sampleCount > 1)
        {
            LOG_ERROR("MSAA (sampleCount > 1) not yet supported");
            return false;
        }

        // 속성 저장
        mWidth = desc.width;
        mHeight = desc.height;
        mFormat = desc.format;

        CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

        CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            desc.format,
            static_cast<UINT64>(desc.width),
            static_cast<UINT>(desc.height),
            1,  // 텍스처 배열 크기
            1,  // Mipmap 레벨
            static_cast<UINT>(desc.sampleCount),
            static_cast<UINT>(desc.sampleQuality)
        );
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        // Depth는 1.0 (최대 거리), Stencil은 0으로 초기화
        D3D12_CLEAR_VALUE clearValue;
        clearValue.Format = desc.format;
        clearValue.DepthStencil.Depth = 1.0f;
        clearValue.DepthStencil.Stencil = 0;

        // Depth-Stencil Buffer 리소스 생성
        HRESULT hr = device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &clearValue,
            IID_PPV_ARGS(&mDepthStencilBuffer)
        );

        if (FAILED(hr))
        {
            LOG_ERROR(
                "Failed to create Depth-Stencil Buffer (HRESULT: 0x%08X, Size: %ux%u, Format: %d)",
                hr,
                desc.width,
                desc.height,
                desc.format
            );
            return false;
        }

        // 초기 상태 저장
        mCurrentState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

#ifdef _DEBUG
        mDepthStencilBuffer->SetName(L"DepthStencilBuffer");
#endif

        // DSV Descriptor Heap 생성
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        dsvHeapDesc.NodeMask = 0;

        hr = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&mDSVHeap));

        if (FAILED(hr))
        {
            LOG_ERROR("Failed to create DSV Descriptor Heap (HRESULT: 0x%08X)", hr);
            mDepthStencilBuffer.Reset();
            return false;
        }

#ifdef _DEBUG
        mDSVHeap->SetName(L"DSV_DescriptorHeap");
#endif

        // DSV 생성
        mDSVHandle = mDSVHeap->GetCPUDescriptorHandleForHeapStart();

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Format = desc.format;

        // MSAA 지원 시 ViewDimension 변경
        if (desc.sampleCount > 1)
        {
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
        }
        else
        {
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice = 0;
        }

        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

        device->CreateDepthStencilView(
            mDepthStencilBuffer.Get(),
            &dsvDesc,
            mDSVHandle
        );

        LOG_INFO(
            "Depth-Stencil Buffer created successfully (Size: %ux%u, Format: %d, Samples: %u)",
            desc.width,
            desc.height,
            desc.format,
            desc.sampleCount
        );

        return true;
    }

    void DX12DepthStencilBuffer::Shutdown()
    {
        if (!IsInitialized())
        {
            return;
        }

        LOG_INFO("Shutting down Depth-Stencil Buffer (%ux%u)", mWidth, mHeight);

        // DirectX 리소스 해제
        mDepthStencilBuffer.Reset();
        mDSVHeap.Reset();

        // 상태 초기화
        mDSVHandle.ptr = 0;
        mCurrentState = D3D12_RESOURCE_STATE_COMMON;
        mFormat = DXGI_FORMAT_UNKNOWN;
        mWidth = 0;
        mHeight = 0;
    }

//#ifdef _DEBUG
//    void DX12DepthStencilBuffer::SetDebugName(const wchar_t* name)
//    {
//        if (!name)
//        {
//            LOG_ERROR("Debug name cannot be null");
//            return;
//        }
//
//        if (!IsInitialized())
//        {
//            LOG_ERROR("Cannot set debug name on uninitialized buffer");
//            return;
//        }
//
//        // Depth-Stencil Buffer 리소스에 이름 설정
//        HRESULT hr = mDepthStencilBuffer->SetName(name);
//        if (FAILED(hr))
//        {
//            LOG_ERROR("Failed to set debug name for Depth-Stencil Buffer (HRESULT: 0x%08X)", hr);
//            return;
//        }
//
//        // DSV Heap에도 이름 설정 (구분을 위해 접미사 추가)
//        std::wstring heapName = std::wstring(name) + L"_DSVHeap";
//        hr = mDSVHeap->SetName(heapName.c_str());
//        if (FAILED(hr))
//        {
//            LOG_ERROR("Failed to set debug name for DSV Heap (HRESULT: 0x%08X)", hr);
//            return;
//        }
//
//        LOG_INFO("Debug name set successfully");
//    }
//#endif

    bool DX12DepthStencilBuffer::IsValidDepthStencilFormat(DXGI_FORMAT format)
    {
        switch (format)
        {
        case DXGI_FORMAT_D32_FLOAT:
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        case DXGI_FORMAT_D16_UNORM:
            return true;
        default:
            return false;
        }
    }

    bool DX12DepthStencilBuffer::Resize(
        ID3D12Device* device,
        uint32 width,
        uint32 height
    )
    {
        if (width == mWidth && height == mHeight)
        {
            LOG_INFO("Depth-Stencil Buffer size unchanged, skipping resize");
            return true;
        }

        LOG_INFO(
            "Resizing Depth-Stencil Buffer: %ux%u -> %ux%u",
            mWidth,
            mHeight,
            width,
            height
        );

        // 기존 리소스 해제
        mDepthStencilBuffer.Reset();
        mDSVHeap.Reset();

        // 동일한 포맷으로 재생성
        return Initialize(device, width, height, mFormat);
    }

} // namespace Graphics