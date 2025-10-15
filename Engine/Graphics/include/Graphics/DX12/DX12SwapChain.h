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

        /**
         * @brief SwapChain 종료 및 리소스 해제
         */
        void Shutdown();

        /**
         * @brief 화면에 렌더링 결과 출력
         * @param vSync VSync 활성화 여부
         */
        void Present(bool vSync = true);

        /**
         * @brief 다음 프레임을 위해 Back Buffer 인덱스 업데이트
         */
        void MoveToNextFrame();

        IDXGISwapChain3* GetSwapChain() const { return mSwapChain.Get(); }

        /**
         * @brief 현재 Back Buffer 리소스 획득
         */
        ID3D12Resource* GetCurrentBackBuffer() const;

        /**
         * @brief 특정 인덱스의 Back Buffer 리소스 획득
         */
        ID3D12Resource* GetBackBuffer(Core::uint32 index) const;

        Core::uint32 GetCurrentBackBufferIndex() const { return mCurrentBackBufferIndex; }
        Core::uint32 GetBufferCount() const { return mBufferCount; }
        Core::uint32 GetWidth() const { return mWidth; }
        Core::uint32 GetHeight() const { return mHeight; }
        DXGI_FORMAT GetFormat() const { return mFormat; }
        bool IsInitialized() const { return mSwapChain != nullptr; }

        /**
         * @brief SwapChain 리사이즈
         * @param width 새로운 너비
         * @param height 새로운 높이
         * @return 리사이즈 성공 여부
         */
        bool Resize(Core::uint32 width, Core::uint32 height);

    private:
        /**
         * @brief SwapChain 생성
         */
        bool CreateSwapChain(
            IDXGIFactory4* factory,
            ID3D12CommandQueue* commandQueue,
            HWND hwnd
        );

        /**
         * @brief Back Buffer 리소스 획득
         */
        bool GetBackBufferResources();

        /**
         * @brief Back Buffer 리소스 해제
         */
        void ReleaseBackBuffers();

    private:
        ComPtr<IDXGISwapChain3> mSwapChain;                 ///< SwapChain
        std::vector<ComPtr<ID3D12Resource>> mBackBuffers;   ///< Back Buffer 리소스들
        Core::uint32 mCurrentBackBufferIndex;               ///< 현재 Back Buffer 인덱스
        Core::uint32 mBufferCount;                          ///< Back Buffer 개수
        Core::uint32 mWidth;                                ///< Back Buffer 너비
        Core::uint32 mHeight;                               ///< Back Buffer 높이
        DXGI_FORMAT mFormat;                                ///< Back Buffer 포맷
    };

} // namespace Graphics