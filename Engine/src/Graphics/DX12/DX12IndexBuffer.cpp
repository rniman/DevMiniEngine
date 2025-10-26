#include "Graphics/DX12/DX12IndexBuffer.h"
#include "Graphics/DX12/d3dx12.h"
#include "Graphics/DX12/DX12CommandQueue.h"
#include "Graphics/DX12/DX12CommandContext.h"
#include <Core/Logging/LogMacros.h>

namespace Graphics
{
	DX12IndexBuffer::~DX12IndexBuffer()
	{
		Shutdown();
	}

	bool DX12IndexBuffer::Initialize(
		ID3D12Device* device,
		DX12CommandQueue* commandQueue,
		DX12CommandContext* commandContext,
		const void* indexData,
		Core::uint32 indexCount,
		DXGI_FORMAT indexFormat
	)
	{
		if (!device || !commandQueue || !commandContext || !indexData)
		{
			LOG_ERROR("[DX12IndexBuffer] Invalid parameters (null pointer)");
			return false;
		}

		if (indexCount == 0)
		{
			LOG_ERROR("[DX12IndexBuffer] Invalid index count: 0");
			return false;
		}

		// 인덱스 포맷 검증
		if (indexFormat != DXGI_FORMAT_R16_UINT && indexFormat != DXGI_FORMAT_R32_UINT)
		{
			LOG_ERROR("[DX12IndexBuffer] Invalid index format (must be R16_UINT or R32_UINT)");
			return false;
		}

		LOG_INFO("[DX12IndexBuffer] Initializing Index Buffer...");

		mIndexCount = indexCount;
		mIndexFormat = indexFormat;

		// 인덱스 크기 계산
		const UINT indexSize = (indexFormat == DXGI_FORMAT_R16_UINT) ? sizeof(Core::uint16) : sizeof(Core::uint32);
		const UINT bufferSize = indexCount * indexSize;

		// Default Heap에 GPU 전용 버퍼 생성
		if (!CreateIndexBuffer(device, bufferSize))
		{
			LOG_ERROR("[DX12IndexBuffer] Failed to create index buffer");
			return false;
		}

		// Upload Heap에 임시 업로드 버퍼 생성
		if (!CreateUploadBuffer(device, bufferSize))
		{
			LOG_ERROR("[DX12IndexBuffer] Failed to create upload buffer");
			return false;
		}

		// CPU 데이터를 Upload Buffer에 복사
		void* mappedData = nullptr;
		CD3DX12_RANGE readRange(0, 0);  // CPU 읽기 없음

		HRESULT hr = mUploadBuffer->Map(0, &readRange, &mappedData);
		if (FAILED(hr))
		{
			LOG_ERROR("[DX12IndexBuffer] Failed to map upload buffer (HRESULT: 0x%08X)", hr);
			return false;
		}

		memcpy(mappedData, indexData, bufferSize);
		mUploadBuffer->Unmap(0, nullptr);

		// GPU 복사 명령 기록
		ID3D12GraphicsCommandList* commandList = commandContext->GetCommandList();
		commandList->Reset(commandContext->GetAllocator(), nullptr);

		commandList->CopyBufferRegion(
			mIndexBuffer.Get(), 0,
			mUploadBuffer.Get(), 0,
			bufferSize
		);

		// COPY_DEST → INDEX_BUFFER 상태 전이
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			mIndexBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_INDEX_BUFFER
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
		mIndexBufferView.BufferLocation = mIndexBuffer->GetGPUVirtualAddress();
		mIndexBufferView.SizeInBytes = bufferSize;
		mIndexBufferView.Format = mIndexFormat;

		LOG_INFO(
			"[DX12IndexBuffer] Initialized successfully (Count: %u, Format: %s, GPU Address: 0x%llX)",
			indexCount,
			(indexFormat == DXGI_FORMAT_R16_UINT) ? "R16_UINT" : "R32_UINT",
			mIndexBufferView.BufferLocation
		);

		// TODO: [Future Optimization] DX12ResourceUploader 패턴으로 리팩토링
		// VertexBuffer와 동일한 개선 방향 적용

		return true;
	}

	void DX12IndexBuffer::Shutdown()
	{
		if (!IsInitialized())
		{
			return;
		}

		LOG_INFO("[DX12IndexBuffer] Shutting down Index Buffer...");

		mIndexBuffer.Reset();
		mIndexBufferView = {};
		mIndexCount = 0;
		mIndexFormat = DXGI_FORMAT_UNKNOWN;

		LOG_INFO("[DX12IndexBuffer] Index Buffer shut down successfully");
	}

	bool DX12IndexBuffer::CreateIndexBuffer(ID3D12Device* device, UINT bufferSize)
	{
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

		HRESULT hr = device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,  // 초기 상태: 복사 대상
			nullptr,
			IID_PPV_ARGS(&mIndexBuffer)
		);

		if (FAILED(hr))
		{
			LOG_ERROR("[DX12IndexBuffer] Failed to create index buffer resource (HRESULT: 0x%08X)", hr);
			return false;
		}

		return true;
	}

	bool DX12IndexBuffer::CreateUploadBuffer(ID3D12Device* device, UINT bufferSize)
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
			LOG_ERROR("[DX12IndexBuffer] Failed to create upload buffer resource (HRESULT: 0x%08X)", hr);
			return false;
		}

		return true;
	}

} // namespace Graphics