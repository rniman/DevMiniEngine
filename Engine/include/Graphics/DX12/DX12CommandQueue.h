#pragma once
#include "Graphics/GraphicsTypes.h"

namespace Graphics
{
	/**
	 * @brief DirectX 12 Command Queue 관리 클래스
	 *
	 * GPU에 명령을 제출하는 Queue를 관리합니다.
	 * Graphics, Compute, Copy 타입을 지원하며, 기본적으로 Graphics Queue를 사용합니다.
	 */
	class DX12CommandQueue
	{
	public:
		DX12CommandQueue() = default;
		~DX12CommandQueue();

		// Non-copyable
		DX12CommandQueue(const DX12CommandQueue&) = delete;
		DX12CommandQueue& operator=(const DX12CommandQueue&) = delete;

		/**
		 * @brief Command Queue 초기화
		 *
		 * @param device D3D12 Device
		 * @param type Command Queue 타입 (기본: Direct)
		 * @return 초기화 성공 여부
		 */
		bool Initialize(
			ID3D12Device* device,
			D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT
		);

		void Shutdown();

		/**
		 * @brief Command List 실행 및 Fence 시그널
		 *
		 * GPU에 작업 목록을 제출하고, 이 작업이 완료되었음을 식별하기 위한
		 * 새로운 Fence 값을 큐에 삽입(Signal)합니다.
		 *
		 * @param commandLists 실행할 Command List 배열
		 * @param numCommandLists Command List 개수
		 *
		 * @return (Core::uint64) 이 작업 배치를 식별하는 고유한 Fence 값 (작업 영수증)
		 *
		 * @note 호출자(예: Renderer)는 이 반환된 Fence 값을
		 * 반드시 FrameResource 같은 곳에 저장해야 합니다.
		 *
		 * @note 이 값은 나중에 이 프레임에서 사용한 리소스(예: Command Allocator)를
		 * 안전하게 재사용하기 전에, GPU 작업 완료를 기다리는 데 사용됩니다.
		 */
		Core::uint64 ExecuteCommandLists(
			ID3D12CommandList* const* commandLists,
			Core::uint32 numCommandLists
		);

		/**
		 * @brief GPU 작업 완료 대기 (블로킹)
		 *
		 * @return 대기 성공 여부
		 *
		 * @warning CPU 블로킹 발생 - 성능에 민감한 경우 사용 주의
		 * @note Queue에 제출된 모든 작업이 완료될 때까지 대기합니다
		 */
		bool WaitForIdle();

		/**
		 * @brief 특정 Fence 값에 도달할 때까지 GPU를 기다립니다.
		 *
		 * WaitForIdle()과 달리, 큐의 모든 작업이 아닌
		 * 'valueToWaitFor'로 식별되는 특정 작업까지만 기다립니다.
		 *
		 * @param valueToWaitFor 기다릴 대상 Fence 값 (ExecuteCommandLists가 반환한 값).
		 * @return 성공 여부
		 *
		 * @note 이 함수는 CPU-GPU 파이프라인 동기화의 핵심이며,
		 * CPU가 GPU를 추월하지 않도록 보장합니다.
		 */
		bool WaitForFenceValue(Core::uint64 valueToWaitFor);

		/**
		 * @brief Fence 값 획득 (현재 완료된 GPU 작업)
		 *
		 * @return GPU가 완료한 마지막 Fence 값
		 *
		 * @note 이 값과 GetNextFenceValue()를 비교하여 GPU 진행 상태를 확인할 수 있습니다
		 */
		Core::uint64 GetCompletedFenceValue() const;

		// Getters
		ID3D12CommandQueue* GetQueue() const { return mCommandQueue.Get(); }
		bool IsInitialized() const { return mCommandQueue != nullptr; }
		D3D12_COMMAND_LIST_TYPE GetType() const { return mType; }
		Core::uint64 GetNextFenceValue() const { return mNextFenceValue; }

	private:
		bool CreateFence(ID3D12Device* device);
		const char* GetCommandListTypeString(D3D12_COMMAND_LIST_TYPE type) const;

		// DirectX Resources
		ComPtr<ID3D12CommandQueue> mCommandQueue;
		ComPtr<ID3D12Fence> mFence;
		HANDLE mFenceEvent = nullptr;

		// Synchronization State
		Core::uint64 mNextFenceValue = 0;
		D3D12_COMMAND_LIST_TYPE mType = D3D12_COMMAND_LIST_TYPE_DIRECT;
	};

} // namespace Graphics
