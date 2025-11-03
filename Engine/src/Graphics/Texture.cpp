#include "pch.h" 
#include "Graphics/Texture.h"
#include "Graphics/DX12/DX12CommandQueue.h"
#include "Graphics/DX12/DX12CommandContext.h"
#include "Graphics/DX12/DX12DescriptorHeap.h"
#include "Core/Logging/LogMacros.h"

#include <filesystem>

// WIC/DDS Texture Loader (DirectXTK12)
#include "WICTextureLoader12.h"
#include "DDSTextureLoader12.h"

using namespace std;

namespace Graphics
{
	Texture::~Texture()
	{ 
		Shutdown();
	}

	bool Texture::LoadFromFile(
		ID3D12Device* device,
		DX12CommandQueue* commandQueue,
		DX12CommandContext* commandContext,
		const wchar_t* filename
	)
	{
		CORE_ASSERT(device != nullptr, "[Texture] Device is null");
		CORE_ASSERT(commandQueue != nullptr, "[Texture] CommandQueue is null");
		CORE_ASSERT(commandContext != nullptr, "[Texture] CommandContext is null");
		CORE_ASSERT(filename != nullptr, "[Texture] Filename is null");

		LOG_INFO(
			"[Texture] Loading WIC texture from file: %s",
			filesystem::path(filename).filename().string().c_str()
		);

		if (mInitialized)
		{
			LOG_WARN("[Texture] Texture already initialized. Shutting down first.");
			Shutdown();
		}

		// 텍스처 데이터와 서브리소스 정보를 담을 컨테이너
		unique_ptr<uint8_t[]> wicData;
		D3D12_SUBRESOURCE_DATA subresource;

		// WICTextureLoader를 사용하여 텍스처 데이터 로드
		HRESULT hr = DirectX::LoadWICTextureFromFile(
			device,
			filename,
			mTexture.GetAddressOf(),
			wicData,
			subresource
		);

		if (FAILED(hr))
		{
			LOG_ERROR("[Texture] Failed to load WIC texture (HRESULT: 0x%08X)", hr);
			return false;
		}

		// GPU로 텍스처 데이터 업로드 (서브리소스 1개)
		if (!UploadTextureData(device, commandContext, commandQueue, &subresource, 1))
		{
			LOG_ERROR("[Texture] Failed to upload WIC texture data to GPU");
			mTexture.Reset();
			return false;
		}

		// 텍스처 정보 가져오기
		D3D12_RESOURCE_DESC desc = mTexture->GetDesc();
		mWidth = static_cast<uint32>(desc.Width);
		mHeight = desc.Height;
		mFormat = desc.Format;

		mInitialized = true;
		LOG_INFO(
			"[Texture] WIC texture loaded successfully (%ux%u, Format: %d)",
			mWidth,
			mHeight,
			static_cast<int>(mFormat)
		);

		return true;
	}

	bool Texture::LoadFromDDS(
		ID3D12Device* device,
		DX12CommandQueue* commandQueue,
		DX12CommandContext* commandContext,
		const wchar_t* filename
	)
	{
		CORE_ASSERT(device != nullptr, "[Texture] Device is null");
		CORE_ASSERT(commandQueue != nullptr, "[Texture] CommandQueue is null");
		CORE_ASSERT(commandContext != nullptr, "[Texture] CommandContext is null");
		CORE_ASSERT(filename != nullptr, "[Texture] Filename is null");

		LOG_INFO(
			"[Texture] Loading DDS texture from file: %s",
			filesystem::path(filename).filename().string().c_str()
		);

		if (mInitialized)
		{
			LOG_WARN("[Texture] Texture already initialized. Shutting down first.");
			Shutdown();
		}

		// 텍스처 데이터와 서브리소스 정보를 담을 컨테이너
		unique_ptr<uint8_t[]> ddsData;
		vector<D3D12_SUBRESOURCE_DATA> subresources;

		// DDSTextureLoader를 사용하여 텍스처 데이터 로드
		HRESULT hr = DirectX::LoadDDSTextureFromFile(
			device,
			filename,
			mTexture.GetAddressOf(),
			ddsData,
			subresources
		);

		if (FAILED(hr))
		{
			LOG_ERROR("[Texture] Failed to load DDS texture (HRESULT: 0x%08X)", hr);
			return false;
		}

		// GPU로 텍스처 데이터 업로드 (서브리소스 N개)
		if (!UploadTextureData(
			device,
			commandContext,
			commandQueue,
			subresources.data(),
			static_cast<UINT>(subresources.size())
		))
		{
			LOG_ERROR("[Texture] Failed to upload DDS texture data to GPU");
			mTexture.Reset();
			return false;
		}

		// 텍스처 정보 가져오기
		D3D12_RESOURCE_DESC desc = mTexture->GetDesc();
		mWidth = static_cast<uint32>(desc.Width);
		mHeight = desc.Height;
		mFormat = desc.Format;

		mInitialized = true;
		LOG_INFO(
			"[Texture] DDS texture loaded successfully (%ux%u, Format: %d, MipLevels: %u)",
			mWidth,
			mHeight,
			static_cast<int>(mFormat),
			desc.MipLevels);

		return true;
	}

	bool Texture::UploadTextureData(
		ID3D12Device* device,
		DX12CommandContext* commandContext,
		DX12CommandQueue* commandQueue,
		D3D12_SUBRESOURCE_DATA* subresources,
		uint32 numSubresources
	)
	{
		ID3D12GraphicsCommandList* commandList = commandContext->GetCommandList();
		commandList->Reset(commandContext->GetAllocator(), nullptr);

		// 서브리소스가 없으면 실패
		if (numSubresources == 0 || subresources == nullptr)
		{
			LOG_ERROR("[Texture] No subresources to upload");
			return false;
		}

		// Command List 가져오기
		ID3D12GraphicsCommandList* cmdList = commandContext->GetCommandList();

		// 업로드 버퍼 크기 계산
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(
			mTexture.Get(),
			0,
			static_cast<UINT>(numSubresources)
		);

		// Upload Heap 생성
		ComPtr<ID3D12Resource> uploadBuffer;
		CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

		HRESULT hr = device->CreateCommittedResource(
			&uploadHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&uploadBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadBuffer.GetAddressOf())
		);

		if (FAILED(hr))
		{
			LOG_ERROR(
				"[Texture] Failed to create upload buffer: HRESULT = 0x%08X",
				static_cast<uint32>(hr)
			);
			return false;
		}

		// 텍스처 데이터를 Upload Buffer로 복사 후 GPU로 전송 (d3dx12.h 헬퍼)
		UpdateSubresources(
			cmdList,
			mTexture.Get(),
			uploadBuffer.Get(),
			0,
			0,
			static_cast<UINT>(numSubresources),
			subresources
		);

		// 리소스 상태 전이: COPY_DEST -> PIXEL_SHADER_RESOURCE (d3dx12.h 헬퍼)
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			mTexture.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		);
		cmdList->ResourceBarrier(1, &barrier);

		// 커맨드 리스트 Close 및 제출
		commandList->Close();
		ID3D12CommandList* commandLists[] = { commandList };
		commandQueue->ExecuteCommandLists(commandLists, 1);
		commandQueue->WaitForIdle();

		// Upload Buffer는 자동으로 해제됨 (ComPtr)
		LOG_TRACE(
			"[Texture] Texture data uploaded to GPU (%u subresource%s)",
			numSubresources, numSubresources > 1 ? "s" : ""
		);

		return true;
	}

	bool Texture::CreateSRV(
		ID3D12Device* device,
		const DX12DescriptorHeap* descriptorHeap,
		uint32 descriptorIndex
	)
	{
		CORE_ASSERT(mInitialized, "[Texture] Texture not initialized");
		CORE_ASSERT(device != nullptr, "[Texture] Device is null");
		CORE_ASSERT(descriptorHeap != nullptr, "[Texture] Descriptor heap is null");

		LOG_INFO("[Texture] Creating SRV for texture...");

		// 디스크립터 힙에서 SRV용 핸들 할당
		// TODO: DX12DescriptorHeap에 AllocateSRV 메서드 구현 필요
		// 임시로 수동 할당 (향후 디스크립터 할당 시스템 구축 예정)
		mSRVCPUHandle = descriptorHeap->GetCPUHandle(descriptorIndex);
		mSRVGPUHandle = descriptorHeap->GetGPUHandle(descriptorIndex);

		// SRV 생성
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = mFormat;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = mTexture->GetDesc().MipLevels;

		device->CreateShaderResourceView(
			mTexture.Get(),
			&srvDesc,
			mSRVCPUHandle
		);

		LOG_INFO(
			"[Texture] SRV created successfully (MipLevels: %u)",
			srvDesc.Texture2D.MipLevels
		);

		return true;
	}

	void Texture::Shutdown()
	{
		if (!mInitialized)
		{
			return;
		}

		LOG_TRACE("[Texture] Shutting down texture (%ux%u)", mWidth, mHeight);

		mTexture.Reset();
		mWidth = 0;
		mHeight = 0;
		mFormat = DXGI_FORMAT_UNKNOWN;
		mInitialized = false;
	}

} // namespace Graphics
