#pragma once
#include "Graphics/GraphicsTypes.h"

namespace Graphics
{
	/**
	 * @brief DirectX 12 기반 렌더러 클래스
	 *
	 * CPU-GPU 파이프라이닝을 관리하고 매 프레임 렌더링을 수행합니다.
	 * FRAME_BUFFER_COUNT 개의 프레임 리소스를 순환하며 사용합니다.
	 */
	class DX12Renderer
	{
	public:
		DX12Renderer() = default;
		~DX12Renderer() = default;

		DX12Renderer(const DX12Renderer&) = delete;
		DX12Renderer& operator=(const DX12Renderer&) = delete;

		// TODO: Game 클래스가 매 프레임 호출할 유일한 public 함수
		// bool Render();

		/**
		 * @brief 다음 프레임으로 인덱스 이동 (Ring Buffer 방식)
		 */
		void MoveFrameIndex()
		{
			mCurrentFrameIndex = (mCurrentFrameIndex + 1) % FRAME_BUFFER_COUNT;
		}

		/**
		 * @brief 특정 프레임의 Fence 값 조회
		 * @param index 프레임 인덱스 (0 ~ FRAME_BUFFER_COUNT-1)
		 */
		Core::uint64 GetFrameFenceValue(Core::uint32 index) const
		{
			GRAPHICS_ASSERT(index < FRAME_BUFFER_COUNT, "Frame index out of range");
			return mFrameFenceValues[index];
		}

		/**
		 * @brief 현재 프레임의 Fence 값 조회
		 */
		Core::uint64 GetCurrentFrameFenceValue() const
		{
			return mFrameFenceValues[mCurrentFrameIndex];
		}

		/**
		 * @brief 현재 프레임 인덱스 조회
		 */
		Core::uint32 GetCurrentFrameIndex() const
		{
			return mCurrentFrameIndex;
		}

		/**
		 * @brief 특정 프레임의 Fence 값 설정
		 * @param index 프레임 인덱스 (0 ~ FRAME_BUFFER_COUNT-1)
		 * @param value 설정할 Fence 값
		 */
		void SetFrameFenceValue(Core::uint32 index, Core::uint64 value)
		{
			GRAPHICS_ASSERT(index < FRAME_BUFFER_COUNT, "Frame index out of range");
			mFrameFenceValues[index] = value;
		}

		/**
		 * @brief 현재 프레임의 Fence 값 설정
		 * @param value 설정할 Fence 값
		 */
		void SetCurrentFrameFenceValue(Core::uint64 value)
		{
			mFrameFenceValues[mCurrentFrameIndex] = value;
		}

	private:
		// TODO: [Future] 렌더링에 필요한 참조 (소유하지 않음)
		// DX12Device* mDevice = nullptr;
		// DX12CommandQueue* mCommandQueue = nullptr;
		// DX12SwapChain* mSwapChain = nullptr;

		// TODO: [Future Enhancement] FrameResource 구조체로 리팩토링
		// 현재: 프레임별 Fence 값만 추적
		// 미래: 프레임별 리소스(CommandAllocator, ConstantBuffer 등)를
		//       FrameResource 구조체로 캡슐화
		//
		// struct FrameResource
		// {
		//     Core::uint64 FenceValue = 0;
		//     ComPtr<ID3D12CommandAllocator> CommandAllocator;
		//     std::unique_ptr<DX12ConstantBuffer<PassConstants>> PassCB;
		// };
		// std::array<FrameResource, FRAME_BUFFER_COUNT> mFrameResources;

		/**
		 * @brief CPU-GPU 동기화를 위한 프레임별 Fence 값
		 *
		 * mFrameFenceValues[i]: i번째 프레임의 GPU 작업이 완료되는 시점을 나타내는 Fence 값
		 * CPU는 i번째 프레임 리소스를 재사용하기 전에 해당 Fence 값에 도달했는지 확인해야 함
		 */
		std::array<Core::uint64, FRAME_BUFFER_COUNT> mFrameFenceValues = { 0 };

		/**
		 * @brief 현재 CPU가 작업 중인 프레임 인덱스 (0 ~ FRAME_BUFFER_COUNT-1)
		 *
		 * Ring Buffer 방식으로 순환: 0 → 1 → 2 → 0 → ...
		 */
		Core::uint32 mCurrentFrameIndex = 0;
	};

} // namespace Graphics