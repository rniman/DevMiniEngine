#pragma once

#include "Graphics/GraphicsTypes.h"

namespace Graphics
{
    /**
     * @brief DirectX 12 Device 관리 클래스
     *
     * GPU와의 인터페이스를 담당하며, Factory, Adapter, Device를 관리합니다.
     */
    class DX12Device
    {
    public:
        DX12Device() = default;
        ~DX12Device();

        // Non-copyable
        DX12Device(const DX12Device&) = delete;
        DX12Device& operator=(const DX12Device&) = delete;

        /**
         * @brief Device 초기화
         * @param enableDebugLayer Debug Layer 활성화 여부 (Debug 빌드에서만 사용)
         * @return 초기화 성공 여부
         */
        bool Initialize(bool enableDebugLayer = true);

        /**
         * @brief Device 종료 및 리소스 해제
         */
        void Shutdown();

        IDXGIFactory4* GetFactory() const { return mFactory.Get(); }
        IDXGIAdapter1* GetAdapter() const { return mAdapter.Get(); }
        ID3D12Device* GetDevice() const { return mDevice.Get(); }
        bool IsInitialized() const { return mDevice != nullptr; }
        D3D_FEATURE_LEVEL GetFeatureLevel() const { return mFeatureLevel; }

    private:
        /**
         * @brief Debug Layer 활성화
         */
        bool EnableDebugLayer();

        /**
         * @brief DXGI Factory 생성
         */
        bool CreateFactory();

        /**
         * @brief 최적의 GPU Adapter 선택
         * @return 선택 성공 여부
         */
        bool SelectAdapter();

        /**
         * @brief D3D12 Device 생성
         * @return 생성 성공 여부
         */
        bool CreateDevice();

        /**
         * @brief Feature Level을 문자열로 변환
         */
        const char* GetFeatureLevelString(D3D_FEATURE_LEVEL featureLevel) const;

    private:
        ComPtr<IDXGIFactory4> mFactory;         ///< DXGI Factory
        ComPtr<IDXGIAdapter1> mAdapter;         ///< GPU Adapter
        ComPtr<ID3D12Device> mDevice;           ///< D3D12 Device
        D3D_FEATURE_LEVEL mFeatureLevel = D3D_FEATURE_LEVEL_11_0; ///< 지원되는 Feature Level
        bool mDebugLayerEnabled = false;        ///< Debug Layer 활성화 여부
    };

} // namespace Graphics