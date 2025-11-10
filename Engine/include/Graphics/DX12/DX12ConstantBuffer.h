#pragma once
#include "Graphics/GraphicsTypes.h"

namespace Graphics
{
	/**
	 * @brief 프레임별로 독립된 Constant Buffer를 관리하는 클래스
	 *
	 * Upload Heap을 사용하여 CPU에서 직접 업데이트 가능한 Constant Buffer를 제공합니다.
	 * 각 프레임마다 독립된 메모리 영역을 사용하여 CPU-GPU 동기화 문제를 방지합니다.
	 *
	 * @note Upload Heap은 초기화 시 Map되어 프로그램 종료까지 유지됩니다
	 * @warning 모든 Constant Buffer는 256바이트 정렬이 필수입니다
	 *
	 * @code
	 * // 사용 예시
	 * DX12ConstantBuffer mvpBuffer;
	 * mvpBuffer.Initialize(device, sizeof(MVPMatrix), 2);
	 *
	 * // 매 프레임 업데이트
	 * MVPMatrix mvp = CalculateMVP();
	 * mvpBuffer.Update(currentFrameIndex, &mvp, sizeof(MVPMatrix));
	 *
	 * // GPU 주소 바인딩
	 * cmdList->SetGraphicsRootConstantBufferView(
	 *     0,
	 *     mvpBuffer.GetGPUAddress(currentFrameIndex)
	 * );
	 * @endcode
	 */
	class DX12ConstantBuffer
	{
	public:
		DX12ConstantBuffer() = default;
		~DX12ConstantBuffer();

		DX12ConstantBuffer(const DX12ConstantBuffer&) = delete;
		DX12ConstantBuffer& operator=(const DX12ConstantBuffer&) = delete;

		/**
		 * @brief Constant Buffer 초기화
		 *
		 * @param device DirectX 12 Device
		 * @param bufferSize 버퍼 크기 (바이트 단위, 자동으로 256바이트 정렬)
		 * @param frameCount 프레임 버퍼 개수 (기본값: FRAME_BUFFER_COUNT)
		 * @return 초기화 성공 여부
		 */
		bool Initialize(
			ID3D12Device* device,
			size_t bufferSize,
			Core::uint32 frameCount = FRAME_BUFFER_COUNT
		);

		void Shutdown();

		/**
		 * @brief 타입 안전 업데이트 헬퍼 함수
		 *
		 * @tparam T Trivially copyable 타입
		 * @param frameIndex 프레임 인덱스
		 * @param data 업데이트할 데이터
		 *
		 * @note 단일 구조체 업데이트에 권장
		 * @note 배열이나 가변 크기는 Update() 사용
		 */
		template<typename T>
		void UpdateTyped(Core::uint32 frameIndex, const T& data)
		{
			static_assert(
				std::is_trivially_copyable_v<T>,
				"Type must be trivially copyable for GPU upload"
				);

			Update(frameIndex, &data, sizeof(T));
		}

		/**
		 * @brief 특정 프레임의 Constant Buffer 데이터 업데이트
		 *
		 * @param frameIndex 업데이트할 프레임 인덱스
		 * @param data 복사할 데이터 포인터
		 * @param dataSize 데이터 크기 (바이트 단위)
		 *
		 * @note 내부적으로 memcpy를 사용하여 업데이트합니다
		 * @warning dataSize는 초기화 시 지정한 bufferSize 이하여야 합니다
		 * @warning frameIndex가 frameCount 이상이면 에러 로그 출력 후 무시됩니다
		 *
		 * @note Upload Heap의 Write-Combined 메모리 특성상 memcpy 후 자동으로 GPU에 전달됩니다
		 * @note CPU와 GPU의 동기화는 호출자가 Fence로 관리해야 합니다
		 */
		void Update(
			Core::uint32 frameIndex,
			const void* data,
			size_t dataSize
		);

		/**
		 * @brief 특정 프레임의 Constant Buffer GPU 주소 반환
		 *
		 * @param frameIndex GPU 주소를 가져올 프레임 인덱스
		 * @return GPU 가상 주소 (SetGraphicsRootConstantBufferView용)
		 */
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(Core::uint32 frameIndex) const;

		size_t GetAlignedBufferSize() const { return mAlignedBufferSize; }
		Core::uint32 GetFrameCount() const { return mFrameCount; }

	private:
		/**
		 * @brief 크기를 256바이트로 정렬
		 *
		 * DirectX 12 Constant Buffer는 256바이트 정렬이 필수
		 */
		static size_t AlignSize(size_t size);

		ComPtr<ID3D12Resource> mConstantBuffer;  // Upload Heap 리소스
		Core::uint8* mMappedData = nullptr;      // 지속적으로 Map된 CPU 포인터
		size_t mAlignedBufferSize = 0;           // 256바이트 정렬된 단일 프레임 버퍼 크기
		Core::uint32 mFrameCount = 0;            // 프레임 버퍼 개수
	};

} // namespace Graphics
