#pragma once
#include "Graphics/GraphicsTypes.h"
#include "d3d12.h"

namespace Graphics
{
	/**
	 * @brief Depth-Stencil Buffer 생성 옵션
	 */
	struct DepthStencilBufferDesc
	{
		uint32 width = 0;
		uint32 height = 0;
		DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		bool enableShaderResource = false;  // 향후 Shadow Mapping 대비
		uint32 sampleCount = 1;             // MSAA 대비
		uint32 sampleQuality = 0;
	};

	/**
	 * @brief Depth-Stencil Buffer와 Descriptor 관리 클래스
	 *
	 * Depth Test와 Stencil Test를 위한 리소스를 생성하고 관리합니다.
	 *
	 * @note 리소스는 Default Heap에 생성되며 DEPTH_WRITE 상태로 시작
	 */
	class DX12DepthStencilBuffer
	{
	public:
		DX12DepthStencilBuffer();
		~DX12DepthStencilBuffer();

		// 복사/이동 금지
		DX12DepthStencilBuffer(const DX12DepthStencilBuffer&) = delete;
		DX12DepthStencilBuffer& operator=(const DX12DepthStencilBuffer&) = delete;

		/**
		 * @brief Depth-Stencil Buffer 초기화
		 *
		 * @param device 유효한 ID3D12Device 포인터 (nullptr 불가)
		 * @param width 버퍼 너비 (1 이상)
		 * @param height 버퍼 높이 (1 이상)
		 * @param format Depth-Stencil 포맷 (D24_UNORM_S8_UINT, D32_FLOAT 등)
		 * @return 초기화 성공 시 true, 실패 시 false
		 *
		 * @note 실패 원인: 잘못된 파라미터, 지원되지 않는 포맷, 리소스/Heap 생성 실패
		 */
		bool Initialize(
			ID3D12Device* device,
			uint32 width,
			uint32 height,
			DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT
		);

		/**
		 * @brief Depth-Stencil Buffer 초기화 (확장 버전)
		 *
		 * @param device 유효한 ID3D12Device 포인터
		 * @param desc 버퍼 생성 옵션
		 * @return 초기화 성공 시 true
		 */
		bool Initialize(ID3D12Device* device, const DepthStencilBufferDesc& desc);;

		/**
		 * @brief 버퍼 크기 변경
		 *
		 * @param device D3D12 Device
		 * @param width 새로운 너비
		 * @param height 새로운 높이
		 * @return 성공 시 true
		 *
		 * @note 기존 리소스를 해제하고 동일한 포맷으로 재생성
		 * @note 크기가 동일하면 재생성하지 않음
		 */
		bool Resize(ID3D12Device* device, uint32 width, uint32 height);

		/**
		 * @brief 리소스 해제
		 */
		void Shutdown();

		// Getters
		ID3D12Resource* GetResource() const { return mDepthStencilBuffer.Get(); }
		D3D12_CPU_DESCRIPTOR_HANDLE GetDSVHandle() const { return mDSVHandle; }
		D3D12_RESOURCE_STATES GetCurrentState() const { return mCurrentState; }
		DXGI_FORMAT GetFormat() const { return mFormat; }
		uint32 GetWidth() const { return mWidth; }
		uint32 GetHeight() const { return mHeight; }
		bool IsInitialized() const { return mDepthStencilBuffer != nullptr; }

		// TODO: Phase 2 완료 후 전체 리소스 클래스에 디버그 이름 기능 일괄 추가
		// #ifdef _DEBUG
		//     void SetDebugName(const wchar_t* name);
		// #endif

	private:
		/**
		 * @brief Depth-Stencil 포맷 유효성 검증
		 */
		static bool IsValidDepthStencilFormat(DXGI_FORMAT format);

		// DirectX 리소스
		ComPtr<ID3D12Resource> mDepthStencilBuffer;
		ComPtr<ID3D12DescriptorHeap> mDSVHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE mDSVHandle;

		// 버퍼 속성
		D3D12_RESOURCE_STATES mCurrentState;
		DXGI_FORMAT mFormat;
		uint32 mWidth;
		uint32 mHeight;
	};

} // namespace Graphics