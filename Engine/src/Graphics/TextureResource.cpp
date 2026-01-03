#include "pch.h" 
#include "Graphics/TextureResource.h"
#include "Graphics/DX12/DX12CommandContext.h"
#include "Graphics/DX12/DX12CommandQueue.h"
#include "Graphics/DX12/DX12DescriptorHeap.h"

// WIC/DDS TextureResource Loader (DirectXTK12)
#include "WICTextureLoader12.h"
#include "DDSTextureLoader12.h"

#include <filesystem>

using namespace std;

namespace
{
	/**
	 * @brief TextureType에 따른 WIC 로더 플래그 반환
	 *
	 * @param textureType 텍스처 용도
	 * @return WIC_LOADER_FLAGS 값
	 */
	DirectX::WIC_LOADER_FLAGS GetWICLoaderFlags(Graphics::TextureType textureType)
	{
		if (Graphics::IsSRGBTexture(textureType))
		{
			return DirectX::WIC_LOADER_FORCE_SRGB;
		}
		else
		{
			return DirectX::WIC_LOADER_IGNORE_SRGB;
		}
	}

} // anonymous namespace

namespace Graphics
{
	TextureResource::~TextureResource()
	{
		Shutdown();
	}

	//=========================================================================
	// 파일 로딩
	//=========================================================================

	bool TextureResource::LoadFromFile(
		ID3D12Device* device,
		DX12CommandQueue* commandQueue,
		DX12CommandContext* commandContext,
		const wchar_t* filename,
		TextureType textureType
	)
	{
		CORE_ASSERT(device != nullptr, "[TextureResource] Device is null");
		CORE_ASSERT(commandQueue != nullptr, "[TextureResource] CommandQueue is null");
		CORE_ASSERT(commandContext != nullptr, "[TextureResource] CommandContext is null");
		CORE_ASSERT(filename != nullptr, "[TextureResource] Filename is null");

		const bool isSRGB = IsSRGBTexture(textureType);

		LOG_INFO(
			"[TextureResource] Loading WIC texture: %s (Type: %s, sRGB: %s)",
			filesystem::path(filename).filename().string().c_str(),
			TextureTypeToString(textureType),
			isSRGB ? "Yes" : "No"
		);

		if (mInitialized)
		{
			LOG_WARN("[TextureResource] TextureResource already initialized. Shutting down first.");
			Shutdown();
		}

		// 텍스처 데이터와 서브리소스 정보를 담을 컨테이너
		unique_ptr<uint8_t[]> wicData;
		D3D12_SUBRESOURCE_DATA subresource;

		// 텍스처 타입에 따른 WIC 플래그 적용
		DirectX::WIC_LOADER_FLAGS loadFlags = GetWICLoaderFlags(textureType);

		// WICTextureLoader Extended 버전으로 텍스처 데이터 로드
		HRESULT hr = DirectX::LoadWICTextureFromFileEx(
			device,
			filename,
			0,                          // maxsize (0 = 제한 없음)
			D3D12_RESOURCE_FLAG_NONE,   // resFlags
			loadFlags,                  // sRGB/Linear 플래그
			mTexture.GetAddressOf(),
			wicData,
			subresource
		);

		if (FAILED(hr))
		{
			LOG_ERROR("[TextureResource] Failed to load WIC texture (HRESULT: 0x%08X)", hr);
			return false;
		}

		// GPU로 텍스처 데이터 업로드 (서브리소스 1개)
		if (!UploadTextureData(device, commandContext, commandQueue, &subresource, 1))
		{
			LOG_ERROR("[TextureResource] Failed to upload WIC texture data to GPU");
			mTexture.Reset();
			return false;
		}

		// 텍스처 정보 가져오기
		D3D12_RESOURCE_DESC desc = mTexture->GetDesc();
		mWidth = static_cast<uint32>(desc.Width);
		mHeight = desc.Height;
		mFormat = desc.Format;
		mIsSRGB = isSRGB;

		mInitialized = true;
		LOG_INFO(
			"[TextureResource] WIC texture loaded (%ux%u, Format: %d, sRGB: %s)",
			mWidth,
			mHeight,
			static_cast<int>(mFormat),
			mIsSRGB ? "Yes" : "No"
		);

		return true;
	}

	bool TextureResource::LoadFromDDS(
		ID3D12Device* device,
		DX12CommandQueue* commandQueue,
		DX12CommandContext* commandContext,
		const wchar_t* filename
	)
	{
		CORE_ASSERT(device != nullptr, "[TextureResource] Device is null");
		CORE_ASSERT(commandQueue != nullptr, "[TextureResource] CommandQueue is null");
		CORE_ASSERT(commandContext != nullptr, "[TextureResource] CommandContext is null");
		CORE_ASSERT(filename != nullptr, "[TextureResource] Filename is null");

		LOG_INFO(
			"[TextureResource] Loading DDS texture from file: %s",
			filesystem::path(filename).filename().string().c_str()
		);

		if (mInitialized)
		{
			LOG_WARN("[TextureResource] TextureResource already initialized. Shutting down first.");
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
			LOG_ERROR("[TextureResource] Failed to load DDS texture (HRESULT: 0x%08X)", hr);
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
			LOG_ERROR("[TextureResource] Failed to upload DDS texture data to GPU");
			mTexture.Reset();
			return false;
		}

		// 텍스처 정보 가져오기
		D3D12_RESOURCE_DESC desc = mTexture->GetDesc();
		mWidth = static_cast<uint32>(desc.Width);
		mHeight = desc.Height;
		mFormat = desc.Format;

		// DDS 포맷에서 sRGB 여부 확인
		switch (mFormat)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			mIsSRGB = true;
			break;
		default:
			mIsSRGB = false;
			break;
		}

		mInitialized = true;
		LOG_INFO(
			"[TextureResource] DDS texture loaded (%ux%u, Format: %d, MipLevels: %u, sRGB: %s)",
			mWidth,
			mHeight,
			static_cast<int>(mFormat),
			desc.MipLevels,
			mIsSRGB ? "Yes" : "No"
		);

		return true;
	}

	//=========================================================================
	// 메모리 로딩
	//=========================================================================

	bool TextureResource::CreateFromMemory(
		ID3D12Device* device,
		DX12CommandQueue* commandQueue,
		DX12CommandContext* commandContext,
		const void* data,
		uint32 width,
		uint32 height,
		DXGI_FORMAT format
	)
	{
		CORE_ASSERT(device != nullptr, "[TextureResource] Device is null");
		CORE_ASSERT(commandQueue != nullptr, "[TextureResource] CommandQueue is null");
		CORE_ASSERT(commandContext != nullptr, "[TextureResource] CommandContext is null");
		CORE_ASSERT(data != nullptr, "[TextureResource] Data is null");
		CORE_ASSERT(width > 0 && height > 0, "[TextureResource] Invalid dimensions");

		LOG_INFO("[TextureResource] Creating texture from raw memory (%ux%u)", width, height);

		if (mInitialized)
		{
			LOG_WARN("[TextureResource] TextureResource already initialized. Shutting down first.");
			Shutdown();
		}

		// 포맷별 픽셀 크기 계산
		uint32 bytesPerPixel = 4;  // R8G8B8A8 기본값
		switch (format)
		{
		case DXGI_FORMAT_R8_UNORM:
			bytesPerPixel = 1;
			break;
		case DXGI_FORMAT_R8G8_UNORM:
			bytesPerPixel = 2;
			break;
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			bytesPerPixel = 4;
			break;
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			bytesPerPixel = 8;
			break;
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			bytesPerPixel = 16;
			break;
		default:
			LOG_WARN("[TextureResource] Unknown format, assuming 4 bytes per pixel");
			bytesPerPixel = 4;
			break;
		}

		// 텍스처 리소스 생성 (Default Heap, COPY_DEST 상태)
		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Alignment = 0;
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = format;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);

		HRESULT hr = device->CreateCommittedResource(
			&defaultHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(mTexture.GetAddressOf())
		);

		if (FAILED(hr))
		{
			LOG_ERROR("[TextureResource] Failed to create texture resource (HRESULT: 0x%08X)", hr);
			return false;
		}

		// 서브리소스 데이터 구성
		D3D12_SUBRESOURCE_DATA subresource = {};
		subresource.pData = data;
		subresource.RowPitch = static_cast<LONG_PTR>(width * bytesPerPixel);
		subresource.SlicePitch = subresource.RowPitch * height;

		// GPU로 업로드 (기존 함수 재사용)
		if (!UploadTextureData(device, commandContext, commandQueue, &subresource, 1))
		{
			LOG_ERROR("[TextureResource] Failed to upload texture data to GPU");
			mTexture.Reset();
			return false;
		}

		// 멤버 변수 설정
		mWidth = width;
		mHeight = height;
		mFormat = format;

		// 포맷에서 sRGB 여부 확인
		switch (format)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			mIsSRGB = true;
			break;
		default:
			mIsSRGB = false;
			break;
		}

		mInitialized = true;

		LOG_INFO(
			"[TextureResource] TextureResource created from raw memory (%ux%u, Format: %d, sRGB: %s)",
			mWidth,
			mHeight,
			static_cast<int>(mFormat),
			mIsSRGB ? "Yes" : "No"
		);

		return true;
	}

	bool TextureResource::LoadFromMemory(
		ID3D12Device* device,
		DX12CommandQueue* commandQueue,
		DX12CommandContext* commandContext,
		const void* data,
		uint32 dataSize,
		TextureType textureType
	)
	{
		CORE_ASSERT(device != nullptr, "[TextureResource] Device is null");
		CORE_ASSERT(commandQueue != nullptr, "[TextureResource] CommandQueue is null");
		CORE_ASSERT(commandContext != nullptr, "[TextureResource] CommandContext is null");
		CORE_ASSERT(data != nullptr, "[TextureResource] Data is null");
		CORE_ASSERT(dataSize > 0, "[TextureResource] Invalid data size");

		const bool isSRGB = IsSRGBTexture(textureType);

		LOG_INFO(
			"[TextureResource] Loading texture from memory (%u bytes, Type: %s, sRGB: %s)",
			dataSize,
			TextureTypeToString(textureType),
			isSRGB ? "Yes" : "No"
		);

		if (mInitialized)
		{
			LOG_WARN("[TextureResource] TextureResource already initialized. Shutting down first.");
			Shutdown();
		}

		// 텍스처 데이터와 서브리소스 정보를 담을 컨테이너
		unique_ptr<uint8_t[]> wicData;
		D3D12_SUBRESOURCE_DATA subresource;

		// 텍스처 타입에 따른 WIC 플래그 적용
		DirectX::WIC_LOADER_FLAGS loadFlags = GetWICLoaderFlags(textureType);

		// WICTextureLoader Extended 버전으로 메모리에서 텍스처 데이터 로드
		HRESULT hr = DirectX::LoadWICTextureFromMemoryEx(
			device,
			static_cast<const uint8_t*>(data),
			static_cast<size_t>(dataSize),
			0,                          // maxsize
			D3D12_RESOURCE_FLAG_NONE,   // resFlags
			loadFlags,                  // sRGB/Linear 플래그
			mTexture.GetAddressOf(),
			wicData,
			subresource
		);

		if (FAILED(hr))
		{
			LOG_ERROR("[TextureResource] Failed to load WIC texture from memory (HRESULT: 0x%08X)", hr);
			return false;
		}

		// GPU로 텍스처 데이터 업로드
		if (!UploadTextureData(device, commandContext, commandQueue, &subresource, 1))
		{
			LOG_ERROR("[TextureResource] Failed to upload texture data to GPU");
			mTexture.Reset();
			return false;
		}

		// 텍스처 정보 가져오기
		D3D12_RESOURCE_DESC desc = mTexture->GetDesc();
		mWidth = static_cast<uint32>(desc.Width);
		mHeight = desc.Height;
		mFormat = desc.Format;
		mIsSRGB = isSRGB;

		mInitialized = true;
		LOG_INFO(
			"[TextureResource] TextureResource loaded from memory (%ux%u, Format: %d, sRGB: %s)",
			mWidth,
			mHeight,
			static_cast<int>(mFormat),
			mIsSRGB ? "Yes" : "No"
		);

		return true;
	}

	//=========================================================================
	// GPU 업로드
	//=========================================================================

	bool TextureResource::UploadTextureData(
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
			LOG_ERROR("[TextureResource] No subresources to upload");
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
				"[TextureResource] Failed to create upload buffer: HRESULT = 0x%08X",
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
		commandQueue->ExecuteCommandLists(1, commandLists);
		commandQueue->WaitForIdle();

		// Upload Buffer는 자동으로 해제됨 (ComPtr)
		LOG_TRACE(
			"[TextureResource] TextureResource data uploaded to GPU (%u subresource%s)",
			numSubresources, numSubresources > 1 ? "s" : ""
		);

		return true;
	}

	//=========================================================================
	// SRV 생성
	//=========================================================================

	bool TextureResource::CreateSRV(
		ID3D12Device* device,
		const DX12DescriptorHeap* descriptorHeap,
		uint32 descriptorIndex
	)
	{
		CORE_ASSERT(mInitialized, "[TextureResource] TextureResource not initialized");
		CORE_ASSERT(device != nullptr, "[TextureResource] Device is null");
		CORE_ASSERT(descriptorHeap != nullptr, "[TextureResource] Descriptor heap is null");

		LOG_INFO("[TextureResource] Creating SRV for texture...");

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
			"[TextureResource] SRV created (MipLevels: %u, sRGB: %s)",
			srvDesc.Texture2D.MipLevels,
			mIsSRGB ? "Yes" : "No"
		);

		return true;
	}

	//=========================================================================
	// 정리
	//=========================================================================

	void TextureResource::Shutdown()
	{
		if (!mInitialized)
		{
			return;
		}

		LOG_TRACE("[TextureResource] Shutting down texture (%ux%u, sRGB: %s)", mWidth, mHeight, mIsSRGB ? "Yes" : "No");

		mTexture.Reset();
		mSRVGPUHandle = {};
		mSRVCPUHandle = {};
		mIsSRGB = false;
	}

} // namespace Graphics
