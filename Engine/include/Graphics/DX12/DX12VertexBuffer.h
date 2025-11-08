#pragma once
#include "Graphics/GraphicsTypes.h"
#include <d3d12.h> 

namespace Graphics
{
	class DX12CommandQueue;
	class DX12CommandContext;

	/**
	 * @brief DirectX 12 버텍스 버퍼 관리 클래스
	 *
	 * GPU 메모리에 버텍스 데이터를 업로드하고 렌더링 시 사용할 수 있도록 관리합니다.
	 * Default Heap (GPU 전용 메모리)를 사용하여 최적의 렌더링 성능을 제공합니다.
	 */
	class DX12VertexBuffer
	{
	public:
		DX12VertexBuffer() = default;
		~DX12VertexBuffer();

		DX12VertexBuffer(const DX12VertexBuffer&) = delete;
		DX12VertexBuffer& operator=(const DX12VertexBuffer&) = delete;

		/**
		 * @brief 버텍스 버퍼를 초기화하고 GPU로 데이터를 업로드합니다
		 *
		 * Default Heap에 버텍스 버퍼를 생성하고, Upload Heap을 통해 CPU 데이터를
		 * GPU로 복사합니다. 복사 완료 후 Upload Buffer는 즉시 해제됩니다.
		 *
		 * @param device DirectX 12 디바이스
		 * @param commandQueue 커맨드 큐 (GPU 작업 제출 및 동기화)
		 * @param commandContext 커맨드 컨텍스트 (커맨드 리스트 및 할당자)
		 * @param vertexData 업로드할 버텍스 데이터 (CPU 메모리)
		 * @param vertexCount 버텍스 개수
		 * @param vertexStride 버텍스 하나의 크기 (바이트)
		 * @return 성공 시 true, 실패 시 false
		 *
		 * @note 이 함수는 내부적으로 GPU 작업 완료를 대기합니다 (WaitForIdle)
		 */
		bool Initialize(
			ID3D12Device* device,
			DX12CommandQueue* commandQueue,
			DX12CommandContext* commandContext,
			const void* vertexData,
			size_t vertexCount,
			size_t vertexStride
		);

		void Shutdown();

		/**
		 * @brief 렌더링에 사용할 Vertex Buffer View를 반환합니다
		 *
		 * @return D3D12_VERTEX_BUFFER_VIEW 구조체
		 */
		D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const { return mVertexBufferView; }
		size_t GetVertexCount() const { return mVertexCount; }
		bool IsInitialized() const { return mVertexBuffer != nullptr; }

	private:
		/**
		 * @brief Default Heap에 버텍스 버퍼를 생성합니다
		 *
		 * @param device DirectX 12 디바이스
		 * @param bufferSize 버퍼 크기 (바이트)
		 * @return 성공 시 true, 실패 시 false
		 */
		bool CreateVertexBuffer(ID3D12Device* device, size_t bufferSize);

		/**
		 * @brief Upload Heap에 임시 업로드 버퍼를 생성합니다
		 *
		 * @param device DirectX 12 디바이스
		 * @param bufferSize 버퍼 크기 (바이트)
		 * @return 성공 시 true, 실패 시 false
		 */
		bool CreateUploadBuffer(ID3D12Device* device, size_t bufferSize);

		ComPtr<ID3D12Resource> mVertexBuffer;				// GPU 전용 버텍스 버퍼 (Default Heap)
		ComPtr<ID3D12Resource> mUploadBuffer;				// CPU-GPU 업로드 버퍼 (Upload Heap)
		D3D12_VERTEX_BUFFER_VIEW mVertexBufferView = {};	// 렌더링에 사용할 View
		size_t mVertexCount = 0;							// 버텍스 개수
		size_t mVertexStride = 0;							// 버텍스 하나의 크기 (바이트)
	};

} // namespace Graphics
