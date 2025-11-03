#pragma once
#include "Graphics/GraphicsTypes.h"

namespace Graphics
{
	class DX12CommandQueue;
	class DX12CommandContext;

	/**
	 * @brief DirectX 12 인덱스 버퍼 관리 클래스
	 *
	 * GPU 메모리에 인덱스 데이터를 업로드하고 렌더링 시 사용할 수 있도록 관리합니다.
	 * Default Heap (GPU 전용 메모리)를 사용하여 최적의 렌더링 성능을 제공합니다.
	 * 인덱스 버퍼를 사용하면 버텍스 재사용으로 메모리를 절약할 수 있습니다.
	 */
	class DX12IndexBuffer
	{
	public:
		DX12IndexBuffer() = default;
		~DX12IndexBuffer();

		DX12IndexBuffer(const DX12IndexBuffer&) = delete;
		DX12IndexBuffer& operator=(const DX12IndexBuffer&) = delete;

		/**
		 * @brief 인덱스 버퍼를 초기화하고 GPU로 데이터를 업로드합니다
		 *
		 * Default Heap에 인덱스 버퍼를 생성하고, Upload Heap을 통해 CPU 데이터를
		 * GPU로 복사합니다. 복사 완료 후 Upload Buffer는 즉시 해제됩니다.
		 *
		 * @param device DirectX 12 디바이스
		 * @param commandQueue 커맨드 큐 (GPU 작업 제출 및 동기화)
		 * @param commandContext 커맨드 컨텍스트 (커맨드 리스트 및 할당자)
		 * @param indexData 업로드할 인덱스 데이터 (CPU 메모리)
		 * @param indexCount 인덱스 개수
		 * @param indexFormat 인덱스 포맷 (DXGI_FORMAT_R16_UINT 또는 DXGI_FORMAT_R32_UINT)
		 * @return 성공 시 true, 실패 시 false
		 *
		 * @note 이 함수는 내부적으로 GPU 작업 완료를 대기합니다 (WaitForIdle)
		 */
		bool Initialize(
			ID3D12Device* device,
			DX12CommandQueue* commandQueue,
			DX12CommandContext* commandContext,
			const void* indexData,
			size_t indexCount,
			DXGI_FORMAT indexFormat = DXGI_FORMAT_R16_UINT
		);

		void Shutdown();

		/**
		 * @brief 렌더링에 사용할 Index Buffer View를 반환합니다
		 *
		 * @return D3D12_INDEX_BUFFER_VIEW 구조체
		 */
		D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const { return mIndexBufferView; }
		size_t GetIndexCount() const { return mIndexCount; }
		bool IsInitialized() const { return mIndexBuffer != nullptr; }

	private:
		/**
		 * @brief Default Heap에 인덱스 버퍼를 생성합니다
		 *
		 * @param device DirectX 12 디바이스
		 * @param bufferSize 버퍼 크기 (바이트)
		 * @return 성공 시 true, 실패 시 false
		 */
		bool CreateIndexBuffer(ID3D12Device* device, size_t bufferSize);

		/**
		 * @brief Upload Heap에 임시 업로드 버퍼를 생성합니다
		 *
		 * @param device DirectX 12 디바이스
		 * @param bufferSize 버퍼 크기 (바이트)
		 * @return 성공 시 true, 실패 시 false
		 */
		bool CreateUploadBuffer(ID3D12Device* device, size_t bufferSize);

		ComPtr<ID3D12Resource> mIndexBuffer;              // GPU 전용 인덱스 버퍼 (Default Heap)
		ComPtr<ID3D12Resource> mUploadBuffer;             // CPU→GPU 업로드 버퍼 (Upload Heap)
		D3D12_INDEX_BUFFER_VIEW mIndexBufferView = {};   // 렌더링에 사용할 View
		size_t mIndexCount = 0;                    // 인덱스 개수
		DXGI_FORMAT mIndexFormat = DXGI_FORMAT_UNKNOWN;  // 인덱스 포맷 (R16_UINT 또는 R32_UINT)
	};

} // namespace Graphics