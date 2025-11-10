#include "pch.h"
#include "Graphics/DX12/DX12VertexBuffer.h"
#include "Graphics/DX12/DX12CommandContext.h"
#include "Graphics/DX12/DX12CommandQueue.h"

namespace Graphics
{
	DX12VertexBuffer::~DX12VertexBuffer()
	{
		Shutdown();
	}

	bool DX12VertexBuffer::Initialize(
		ID3D12Device* device,
		DX12CommandQueue* commandQueue,
		DX12CommandContext* commandContext,
		const void* vertexData,
		size_t vertexCount,
		size_t vertexStride
	)
	{
		if (!device || !commandQueue || !commandContext || !vertexData)
		{
			LOG_ERROR("[DX12VertexBuffer] Invalid parameters (null pointer)");
			return false;
		}

		LOG_INFO("[DX12VertexBuffer] Initializing Vertex Buffer...");

		mVertexCount = vertexCount;
		mVertexStride = vertexStride;

		const size_t bufferSize = vertexCount * vertexStride;

		// Default Heap에 버텍스 버퍼 생성 (GPU 전용 메모리)
		if (!CreateVertexBuffer(device, bufferSize))
		{
			LOG_ERROR("[DX12VertexBuffer] Failed to create vertex buffer");
			return false;
		}

		// Upload Heap에 임시 업로드 버퍼 생성
		if (!CreateUploadBuffer(device, bufferSize))
		{
			LOG_ERROR("[DX12VertexBuffer] Failed to create upload buffer");
			return false;
		}

		// CPU 데이터를 Upload Buffer에 복사
		void* mappedData = nullptr;
		CD3DX12_RANGE readRange(0, 0); // CPU 읽기 없음

		HRESULT hr = mUploadBuffer->Map(0, &readRange, &mappedData);
		if (FAILED(hr))
		{
			LOG_ERROR("[DX12VertexBuffer] Failed to map upload buffer (HRESULT: 0x%08X)", hr);
			return false;
		}

		memcpy(mappedData, vertexData, bufferSize);
		mUploadBuffer->Unmap(0, nullptr);

		// GPU 복사 명령 기록
		ID3D12GraphicsCommandList* commandList = commandContext->GetCommandList();
		commandList->Reset(commandContext->GetAllocator(), nullptr);

		commandList->CopyBufferRegion(
			mVertexBuffer.Get(), 0,
			mUploadBuffer.Get(), 0,
			static_cast<UINT64>(bufferSize)
		);

		// COPY_DEST - VERTEX_AND_CONSTANT_BUFFER 상태 전이
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			mVertexBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
		);
		commandList->ResourceBarrier(1, &barrier);


		// 커맨드 리스트 Close 및 제출
		commandList->Close();
		ID3D12CommandList* commandLists[] = { commandList };
		commandQueue->ExecuteCommandLists(commandLists, 1);
		commandQueue->WaitForIdle();

		// GPU 복사 완료 - Upload Buffer 해제
		mUploadBuffer.Reset();

		// 렌더링에 사용할 View 초기화
		mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
		mVertexBufferView.SizeInBytes = static_cast<UINT>(bufferSize);
		mVertexBufferView.StrideInBytes = static_cast<UINT>(mVertexStride);

		LOG_INFO(
			"[DX12VertexBuffer] Initialized successfully (Count: %u, Stride: %u, GPU Address: 0x%llX)",
			vertexCount,
			vertexStride,
			mVertexBufferView.BufferLocation
		);

		// TODO: [Future Optimization] DX12ResourceUploader 패턴으로 리팩토링
		// 현재: 각 VertexBuffer가 개별 Upload Buffer 소유
		// 목표: 공유 Upload Buffer를 통한 배치 업로드
		// 
		// 개선 방향:
		// 1. DX12ResourceUploader 클래스 구현
		//    - 큰 Upload Buffer 하나를 여러 리소스가 공유
		//    - Ring Buffer 패턴으로 메모리 재사용
		//    - FlushUploads()로 일괄 제출
		// 
		// 2. VertexBuffer 단순화
		//    - Upload Buffer 멤버 제거
		//    - Initialize()에서 uploader->UploadVertexBuffer() 호출
		// 
		// 3. 사용 예시:
		//    DX12ResourceUploader uploader;
		//    uploader.Initialize(device, 256MB);
		//    for (auto& vb : vertexBuffers)
		//        vb.Initialize(device, &uploader, ...);
		//    uploader.FlushUploads(cmdList, queue);
		// 
		// 장점:
		// - 메모리 효율 증가 (Upload Buffer 재사용)
		// - 배치 업로드로 성능 향상
		// - 명확한 책임 분리
		// 
		// 적용 시점:
		// - 리소스 로딩이 복잡해질 때
		// - 메모리 관리가 중요해질 때
		// - 로딩 성능 최적화가 필요할 때

		return true;
	}

	void DX12VertexBuffer::Shutdown()
	{
		if (!IsInitialized())
		{
			return;
		}

		LOG_INFO("[DX12VertexBuffer] Shutting down Vertex Buffer...");

		mVertexBuffer.Reset();
		mVertexBufferView = {};
		mVertexCount = 0;
		mVertexStride = 0;

		LOG_INFO("[DX12VertexBuffer] Vertex Buffer shut down successfully");
	}

	bool DX12VertexBuffer::CreateVertexBuffer(ID3D12Device* device, size_t bufferSize)
	{
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

		HRESULT hr = device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,     // 초기 상태: 복사 대상
			nullptr,
			IID_PPV_ARGS(&mVertexBuffer)
		);

		if (FAILED(hr))
		{
			LOG_ERROR("[DX12VertexBuffer] Failed to create vertex buffer resource (HRESULT: 0x%08X)", hr);
			return false;
		}

		return true;
	}

	bool DX12VertexBuffer::CreateUploadBuffer(ID3D12Device* device, size_t bufferSize)
	{
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

		HRESULT hr = device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,  // Upload Heap 기본 상태
			nullptr,
			IID_PPV_ARGS(&mUploadBuffer)
		);

		if (FAILED(hr))
		{
			LOG_ERROR("[DX12VertexBuffer] Failed to create upload vertex buffer resource (HRESULT: 0x%08X)", hr);
			return false;
		}

		return true;
	}

} // namespace Graphics
