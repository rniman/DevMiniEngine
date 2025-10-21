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
		 * @brief Command List 실행
		 *
		 * @param commandLists 실행할 Command List 배열
		 * @param numCommandLists Command List 개수
		 *
		 * @note 즉시 GPU에 제출되며, Fence를 통한 동기화가 필요합니다
		 */
		void ExecuteCommandLists(
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

		// 멤버 변수

		// DirectX Resources
		ComPtr<ID3D12CommandQueue> mCommandQueue;
		ComPtr<ID3D12Fence> mFence;
		HANDLE mFenceEvent;

		// Synchronization State
		Core::uint64 mNextFenceValue;
		D3D12_COMMAND_LIST_TYPE mType;
	};

} // namespace Graphics