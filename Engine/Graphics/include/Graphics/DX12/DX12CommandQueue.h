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
         * @param device D3D12 Device
         * @param type Command Queue 타입 (기본: Direct)
         * @return 초기화 성공 여부
         */
        bool Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);

        /**
         * @brief Command Queue 종료 및 리소스 해제
         */
        void Shutdown();

        ID3D12CommandQueue* GetQueue() const { return mCommandQueue.Get(); }
        bool IsInitialized() const { return mCommandQueue != nullptr; }
        D3D12_COMMAND_LIST_TYPE GetType() const { return mType; }

        /**
         * @brief Command List 실행
         * @param ppCommandLists 실행할 Command List 배열
         * @param numCommandLists Command List 개수
         */
        void ExecuteCommandLists(ID3D12CommandList* const* ppCommandLists, Core::uint32 numCommandLists);

        /**
         * @brief GPU 작업 완료 대기
         * @return 대기 성공 여부
         */
        bool WaitForIdle();

        /**
         * @brief Fence 값 획득 (현재 완료된 GPU 작업)
         */
        Core::uint64 GetCompletedFenceValue() const;

        /**
         * @brief 다음 Fence 값 획득 (다음 Signal할 값)
         */
        Core::uint64 GetNextFenceValue() const { return mNextFenceValue; }

    private:
        /**
         * @brief Fence 초기화
         */
        bool CreateFence(ID3D12Device* device);

        /**
         * @brief Command Queue 타입을 문자열로 변환
         */
        const char* GetCommandListTypeString(D3D12_COMMAND_LIST_TYPE type) const;

    private:
        ComPtr<ID3D12CommandQueue> mCommandQueue;   ///< Command Queue
        ComPtr<ID3D12Fence> mFence;                 ///< Fence (GPU 동기화)
        HANDLE mFenceEvent;                         ///< Fence 이벤트 핸들
        Core::uint64 mNextFenceValue;               ///< 다음 Fence 값
        D3D12_COMMAND_LIST_TYPE mType;              ///< Command Queue 타입
    };

} // namespace Graphics