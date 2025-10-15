#pragma once

#include "Graphics/GraphicsTypes.h"

namespace Graphics
{
    /**
     * @brief DirectX 12 Command 실행 컨텍스트
     *
     * Command Allocator와 Command List를 관리합니다.
     * 프레임별로 하나씩 생성하여 사용합니다.
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
         * @param device D3D12 Device
         * @param type Command List 타입
         * @return 초기화 성공 여부
         */
        bool Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type);

        /**
         * @brief Command Context 종료 및 리소스 해제
         */
        void Shutdown();

        /**
         * @brief Command List 리셋 (새 프레임 시작)
         */
        bool Reset();

        /**
         * @brief Command List 닫기 (기록 종료)
         */
        bool Close();

        ID3D12CommandAllocator* GetAllocator() const { return mCommandAllocator.Get(); }
        ID3D12GraphicsCommandList* GetCommandList() const { return mCommandList.Get(); }
        bool IsInitialized() const { return mCommandList != nullptr; }

    private:
        ComPtr<ID3D12CommandAllocator>      mCommandAllocator;  ///< Command Allocator
        ComPtr<ID3D12GraphicsCommandList>   mCommandList;       ///< Command List
        D3D12_COMMAND_LIST_TYPE             mType;              ///< Command List 타입
    };

} // namespace Graphics