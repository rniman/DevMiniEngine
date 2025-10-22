#pragma once
#include "Graphics/GraphicsTypes.h"

namespace Graphics
{
    /**
     * @brief DirectX 12 Command 실행 컨텍스트
     *
     * Command Allocator와 Command List를 관리합니다.
     *
     * @note 단일 Allocator를 사용하므로 GPU 동기화 후에만 Reset 가능합니다
     * @warning 프레임 버퍼링을 위해서는 프레임 수만큼 인스턴스를 생성해야 합니다
     */
    class DX12CommandContext
    {
    public:
        DX12CommandContext() = default;
        ~DX12CommandContext();

        // Non-copyable
        DX12CommandContext(const DX12CommandContext&) = delete;
        DX12CommandContext& operator=(const DX12CommandContext&) = delete;

        /**
         * @brief Command Context 초기화
         *
         * @param device D3D12 Device
         * @param type Command List 타입
         * @return 초기화 성공 여부
         */
        bool Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type);

        void Shutdown();

        /**
         * @brief Command List 리셋 (새 프레임 시작)
         *
         * @return 리셋 성공 여부
         *
         * @note 프레임 시작 시 호출하여 새로운 명령 기록을 시작합니다
         * @warning Allocator가 GPU에서 사용 중일 때 호출하면 실패합니다
         */
        bool Reset();

        /**
         * @brief Command List 닫기 (기록 종료)
         *
         * @return 닫기 성공 여부
         *
         * @note ExecuteCommandLists() 호출 전에 반드시 Close()해야 합니다
         * @warning 이미 닫힌 Command List를 다시 닫으면 실패합니다
         */
        bool Close();

        // Getters
        ID3D12CommandAllocator* GetAllocator() const { return mCommandAllocator.Get(); }
        ID3D12GraphicsCommandList* GetCommandList() const { return mCommandList.Get(); }
        bool IsInitialized() const { return mCommandList != nullptr; }

    private:
        // DirectX Resources
        ComPtr<ID3D12CommandAllocator> mCommandAllocator;
        ComPtr<ID3D12GraphicsCommandList> mCommandList;

        // Properties
        D3D12_COMMAND_LIST_TYPE mType = D3D12_COMMAND_LIST_TYPE_DIRECT;
    };

} // namespace Graphics