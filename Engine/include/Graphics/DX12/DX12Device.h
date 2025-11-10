#pragma once
#include "Graphics/GraphicsTypes.h"
#include <array>
#include <memory>

namespace Graphics
{
	// Forward declarations
	class DX12CommandQueue;
	class DX12SwapChain;
	class DX12CommandContext;

	/**
	 * @brief DirectX 12 Device 관리 클래스
	 *
	 * GPU와의 인터페이스를 담당하며 모든 DirectX 12 리소스의 생성을 관리합니다.
	 *
	 * 초기화 순서:
	 * 1. Debug Layer 활성화 (선택적)
	 * 2. DXGI Factory 생성
	 * 3. GPU Adapter 선택 (최대 VRAM 기준)
	 * 4. D3D12 Device 생성
	 * 5. Command Queue 생성
	 * 6. Descriptor Heap 생성
	 * 7. Command Context 생성
	 *
	 * @note SwapChain은 별도로 CreateSwapChain()을 호출하여 생성
	 */
	class DX12Device
	{
	public:
		DX12Device();
		~DX12Device();

		// Non-copyable
		DX12Device(const DX12Device&) = delete;
		DX12Device& operator=(const DX12Device&) = delete;

		/**
		 * @brief Device 초기화
		 *
		 * @param enableDebugLayer Debug Layer 활성화 여부 (Debug 빌드에서만 유효)
		 * @return 초기화 성공 여부
		 *
		 * @note Debug Layer는 성능 오버헤드가 있으므로 Release 빌드에서는 비활성화 권장
		 */
		bool Initialize(bool enableDebugLayer = true);

		/**
		 * @brief Device 종료 및 모든 리소스 해제
		 *
		 * 리소스는 역순으로 해제됩니다:
		 * 1. Command Context
		 * 2. SwapChain
		 * 3. Descriptor Heap
		 * 4. Command Queue
		 * 5. Device, Adapter, Factory
		 *
		 * @note GPU 유휴 대기 후 호출하는 것을 권장합니다
		 */
		void Shutdown();

		/**
		 * @brief 스왑 체인 생성 및 초기화
		 *
		 * @param hwnd 렌더링 대상 윈도우 핸들
		 * @param width 백 버퍼 너비
		 * @param height 백 버퍼 높이
		 * @return 생성 성공 여부
		 *
		 * @note Initialize() 호출 후에만 사용 가능
		 * @note RTV(Render Target View)도 함께 생성됩니다
		 * @note 윈도우 리사이징, 전체 화면 전환을 대비하여 Initialize와 분리하여 사용합니다
		 * @warning 기존 스왑 체인이 있으면 실패합니다
		 */
		bool CreateSwapChain(HWND hwnd, Core::uint32 width, Core::uint32 height);

		// Getters
		IDXGIFactory4* GetFactory() const { return mFactory.Get(); }
		IDXGIAdapter1* GetAdapter() const { return mAdapter.Get(); }
		ID3D12Device* GetDevice() const { return mDevice.Get(); }
		D3D_FEATURE_LEVEL GetFeatureLevel() const { return mFeatureLevel; }
		DX12CommandQueue* GetGraphicsQueue() const { return mGraphicsQueue.get(); }
		DX12SwapChain* GetSwapChain() const { return mSwapChain.get(); }
		DX12CommandContext* GetCommandContext(Core::uint32 index) const;
		bool IsInitialized() const { return mDevice != nullptr; }

	private:
		// 초기화 헬퍼 (호출 순서대로 정렬)
		bool EnableDebugLayer();
		bool CreateFactory();
		bool SelectAdapter();
		bool CreateDevice();
		bool CreateCommandQueues();
		bool CreateCommandContexts();

		// 유틸리티 함수
		const char* GetFeatureLevelString(D3D_FEATURE_LEVEL featureLevel) const;

		// 멤버 변수
		
		// --- 코어 DX12 객체 ---
		// DX12 호환성을 위한 "베이스라인" 인터페이스를 멤버로 저장합니다.
		// 
		// TODO: [엔진 확장 지점]
		// 레이 트레이싱(ID3D12Device5), 메시 셰이더(ID3D12Device6) 등
		// 더 높은 버전의 기능이 필요할 경우, mDevice.As(&device5)처럼
		// 함수 내에서 일시적으로 쿼리하여 사용합니다.
		// 확장 시기에는 다양한 COM 객체의 버젼을 확인 해야합니다.
		// 멤버 변수 자체의 타입은 변경할 필요가 없습니다.
		ComPtr<IDXGIFactory4> mFactory;
		ComPtr<IDXGIAdapter1> mAdapter;
		ComPtr<ID3D12Device> mDevice;

		// Graphics Pipeline Components
		std::unique_ptr<DX12CommandQueue> mGraphicsQueue;   // D3D12_COMMAND_LIST_TYPE_DIRECT
		std::unique_ptr<DX12SwapChain> mSwapChain;
		std::array<std::unique_ptr<DX12CommandContext>, FRAME_BUFFER_COUNT> mCommandContexts;

		// Properties
		D3D_FEATURE_LEVEL mFeatureLevel = D3D_FEATURE_LEVEL_11_0;

		// Flags
		bool mDebugLayerEnabled = false;
		bool mTearingAllowed = false;       // Factory5의 쿼리된 기능 플래그
	};

} // namespace Graphics
