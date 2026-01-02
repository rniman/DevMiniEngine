#pragma once
#include "Graphics/GraphicsTypes.h"
#include "Graphics/TextureType.h"

namespace Graphics
{
	class DX12CommandQueue;
	class DX12CommandContext;
	class DX12DescriptorHeap;

	/**
	 * @brief DirectX 12 텍스처 리소스 관리 클래스
	 *
	 * WICTextureLoader와 DDSTextureLoader를 사용하여 다양한 포맷의
	 * 텍스처 파일을 로드하고 GPU 리소스로 관리합니다.
	 *
	 * 지원 포맷:
	 * - WIC: PNG, JPG, BMP, GIF, TIFF, WMP
	 * - DDS: 압축 텍스처(BC1-BC7), 밉맵, 큐브맵 등
	 * - Memory: 절차적 생성, 폴백 텍스처 등
	 *
	 * @note Phase 4.3: sRGB/Linear 색공간 자동 처리 지원
	 */
	class Texture
	{
	public:
		Texture() = default;
		~Texture();

		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

		//=========================================================================
		// 파일 로딩 (색공간 자동 적용)
		//=========================================================================

		/**
		 * @brief WIC를 사용하여 이미지 파일 로드 (색공간 자동 적용)
		 *
		 * TextureType에 따라 sRGB/Linear 색공간을 자동으로 선택합니다.
		 * - Albedo, Emissive: sRGB (WIC_LOADER_FORCE_SRGB)
		 * - Normal, Roughness 등: Linear (WIC_LOADER_IGNORE_SRGB)
		 *
		 * @param device DirectX 12 디바이스
		 * @param commandQueue 커맨드 큐 (GPU 동기화)
		 * @param commandContext 커맨드 컨텍스트
		 * @param filename 로드할 파일 경로 (와이드 문자열)
		 * @param textureType 텍스처 용도 (색공간 결정에 사용)
		 * @return 성공 시 true, 실패 시 false
		 *
		 * @note Phase 4.3: 색공간 처리를 위한 권장 API
		 */
		bool LoadFromFile(
			ID3D12Device* device,
			DX12CommandQueue* commandQueue,
			DX12CommandContext* commandContext,
			const wchar_t* filename,
			TextureType textureType
		);

		/**
		 * @brief DDS 파일 로드
		 *
		 * 압축 텍스처(BC1-BC7), 밉맵 체인을 포함한 DDS 파일을 로드합니다.
		 * 런타임 성능이 중요한 경우 권장됩니다.
		 *
		 * @param device DirectX 12 디바이스
		 * @param commandQueue 커맨드 큐
		 * @param commandContext 커맨드 컨텍스트
		 * @param filename 로드할 DDS 파일 경로
		 * @return 성공 시 true, 실패 시 false
		 *
		 * @note DDS 파일은 이미 색공간 정보를 포함하므로 별도 처리 불필요
		 */
		bool LoadFromDDS(
			ID3D12Device* device,
			DX12CommandQueue* commandQueue,
			DX12CommandContext* commandContext,
			const wchar_t* filename
		);

		//=========================================================================
		// 메모리 로딩
		//=========================================================================

		/**
		 * @brief 원시 픽셀 데이터로부터 텍스처 생성
		 *
		 * 픽셀 데이터 배열로부터 직접 텍스처를 생성합니다.
		 * 폴백 텍스처, 절차적 텍스처 생성에 사용됩니다.
		 *
		 * @param device DirectX 12 디바이스
		 * @param commandQueue 커맨드 큐
		 * @param commandContext 커맨드 컨텍스트
		 * @param data 픽셀 데이터 (RGBA 순서, 행 우선)
		 * @param width 텍스처 너비
		 * @param height 텍스처 높이
		 * @param format 픽셀 포맷 (기본: DXGI_FORMAT_R8G8B8A8_UNORM)
		 * @return 성공 시 true, 실패 시 false
		 *
		 * @note Phase 4.2: 폴백 텍스처 지원을 위해 추가
		 */
		bool CreateFromMemory(
			ID3D12Device* device,
			DX12CommandQueue* commandQueue,
			DX12CommandContext* commandContext,
			const void* data,
			uint32 width,
			uint32 height,
			DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM
		);

		/**
		 * @brief 압축된 이미지 데이터로부터 텍스처 로드 (색공간 자동 적용)
		 *
		 * WIC를 사용하여 메모리 상의 압축 이미지를 디코딩합니다.
		 * glb 파일의 임베디드 텍스처 로딩에 사용됩니다.
		 *
		 * @param device DirectX 12 디바이스
		 * @param commandQueue 커맨드 큐
		 * @param commandContext 커맨드 컨텍스트
		 * @param data 압축된 이미지 데이터 (PNG, JPG 바이너리)
		 * @param dataSize 데이터 크기 (바이트)
		 * @param textureType 텍스처 용도 (색공간 결정에 사용)
		 * @return 성공 시 true, 실패 시 false
		 *
		 * @note Phase 4.3: 색공간 처리를 위한 권장 API
		 */
		bool LoadFromMemory(
			ID3D12Device* device,
			DX12CommandQueue* commandQueue,
			DX12CommandContext* commandContext,
			const void* data,
			uint32 dataSize,
			TextureType textureType
		);

		//=========================================================================
		// SRV 생성
		//=========================================================================

		/**
		 * @brief SRV(Shader Resource View) 생성
		 *
		 * 텍스처를 셰이더에서 사용할 수 있도록 SRV를 생성합니다.
		 * LoadFromFile/LoadFromDDS 후에 호출해야 합니다.
		 *
		 * @param device DirectX 12 디바이스
		 * @param descriptorHeap SRV를 할당할 디스크립터 힙
		 * @param descriptorIndex 디스크립터 힙 내 인덱스
		 * @return 성공 시 true, 실패 시 false
		 */
		bool CreateSRV(
			ID3D12Device* device,
			const DX12DescriptorHeap* descriptorHeap,
			uint32 descriptorIndex
		);

		void Shutdown();

		//=========================================================================
		// Getters
		//=========================================================================

		ID3D12Resource* GetResource() const { return mTexture.Get(); }
		D3D12_GPU_DESCRIPTOR_HANDLE GetSRVHandle() const { return mSRVGPUHandle; }
		D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUHandle() const { return mSRVCPUHandle; }
		bool IsInitialized() const { return mInitialized; }
		bool HasSRV() const { return mSRVCPUHandle.ptr != 0; }

		// 텍스처 정보
		uint32 GetWidth() const { return mWidth; }
		uint32 GetHeight() const { return mHeight; }
		DXGI_FORMAT GetFormat() const { return mFormat; }

		/**
		 * @brief 텍스처가 sRGB 색공간인지 여부
		 * @return sRGB 포맷이면 true, Linear 포맷이면 false
		 * @note Phase 4.3: 색공간 처리 지원
		 */
		bool IsSRGB() const { return mIsSRGB; }

	private:
		/**
		 * @brief 텍스처 데이터를 GPU로 업로드
		 *
		 * Upload Heap을 사용하여 서브리소스 데이터를 Default Heap으로 복사합니다.
		 * COPY_DEST 상태에서 PIXEL_SHADER_RESOURCE 상태로 전이를 수행합니다.
		 *
		 * @param device DirectX 12 디바이스
		 * @param commandQueue 커맨드 큐
		 * @param commandContext 커맨드 컨텍스트
		 * @param subresources 업로드할 서브리소스 데이터 배열
		 * @param numSubresources 서브리소스 개수
		 * @return 성공 시 true, 실패 시 false
		 */
		bool UploadTextureData(
			ID3D12Device* device,
			DX12CommandContext* commandContext,
			DX12CommandQueue* commandQueue,
			D3D12_SUBRESOURCE_DATA* subresources,
			uint32 numSubresources
		);

		ComPtr<ID3D12Resource> mTexture;                // 텍스처 리소스
		D3D12_GPU_DESCRIPTOR_HANDLE mSRVGPUHandle = {}; // GPU용 SRV 핸들
		D3D12_CPU_DESCRIPTOR_HANDLE mSRVCPUHandle = {}; // CPU용 SRV 핸들

		uint32 mWidth = 0;                              // 텍스처 가로 크기
		uint32 mHeight = 0;                             // 텍스처 세로 크기
		DXGI_FORMAT mFormat = DXGI_FORMAT_UNKNOWN;      // 픽셀 포맷

		bool mInitialized = false;                      // 초기화 여부
		bool mIsSRGB = false;                           // sRGB 색공간 여부 (Phase 4.3)
	};

} // namespace Graphics
