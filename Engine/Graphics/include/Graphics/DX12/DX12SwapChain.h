#pragma once
#include "Graphics/GraphicsTypes.h"

namespace Graphics
{
	/**
	 * @brief DirectX 12 SwapChain 관리 클래스
	 *
	 * 화면 출력을 위한 SwapChain과 Back Buffer들을 관리합니다.
	 * Double buffering을 기본으로 사용합니다.
	 */
	class DX12SwapChain
	{
	public:
		DX12SwapChain() = default;
		~DX12SwapChain();

		// Non-copyable
		DX12SwapChain(const DX12SwapChain&) = delete;
		DX12SwapChain& operator=(const DX12SwapChain&) = delete;

		/**
		 * @brief SwapChain 초기화
		 *
		 * @param factory DXGI Factory
		 * @param commandQueue Command Queue
		 * @param hwnd Window handle
		 * @param width Back buffer 너비
		 * @param height Back buffer 높이
		 * @param bufferCount Back buffer 개수 (기본: 2)
		 * @return 초기화 성공 여부
		 */
		bool Initialize(
			IDXGIFactory4* factory,
			ID3D12CommandQueue* commandQueue,
			HWND hwnd,
			Core::uint32 width,
			Core::uint32 height,
			Core::uint32 bufferCount = FRAME_BUFFER_COUNT
		);

		void Shutdown();
		void Present(bool vSync = true);

		/**
		 * @brief 다음 프레임을 위해 Back Buffer 인덱스 업데이트
		 *
		 * @note Present() 호출 후 반드시 호출해야 합니다
		 */
		void MoveToNextFrame();

		/**
		 * @brief SwapChain 리사이즈
		 *
		 * @warning 호출 전 GPU 작업 완료 대기 필요
		 * @note Back Buffer 리소스가 재생성됩니다
		 */
		bool Resize(Core::uint32 width, Core::uint32 height);

		// Getters
		IDXGISwapChain3* GetSwapChain() const { return mSwapChain.Get(); }
		ID3D12Resource* GetCurrentBackBuffer() const;
		ID3D12Resource* GetBackBuffer(Core::uint32 index) const;
		Core::uint32 GetCurrentBackBufferIndex() const { return mCurrentBackBufferIndex; }
		Core::uint32 GetBufferCount() const { return mBufferCount; }
		Core::uint32 GetWidth() const { return mWidth; }
		Core::uint32 GetHeight() const { return mHeight; }
		DXGI_FORMAT GetFormat() const { return mFormat; }
		bool IsInitialized() const { return mSwapChain != nullptr; }

	private:
		bool CreateSwapChain(
			IDXGIFactory4* factory,
			ID3D12CommandQueue* commandQueue,
			HWND hwnd
		);
		bool GetBackBufferResources();
		void ReleaseBackBuffers();

		// 멤버 변수

		// DirectX Resources
		ComPtr<IDXGISwapChain3> mSwapChain;
		std::vector<ComPtr<ID3D12Resource>> mBackBuffers;

		// SwapChain Properties
		Core::uint32 mCurrentBackBufferIndex;
		Core::uint32 mBufferCount;
		Core::uint32 mWidth;
		Core::uint32 mHeight;
		DXGI_FORMAT mFormat;
	};

} // namespace Graphics