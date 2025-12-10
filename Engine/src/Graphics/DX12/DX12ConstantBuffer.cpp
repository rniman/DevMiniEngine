#include "pch.h"
#include "Graphics/DX12/DX12ConstantBuffer.h"


using namespace Core;

namespace Graphics
{
	DX12ConstantBuffer::~DX12ConstantBuffer()
	{
		Shutdown();
	}

	bool DX12ConstantBuffer::Initialize(
		ID3D12Device* device,
		size_t bufferSize,
		uint32 frameCount
	)
	{
		if (!device)
		{
			LOG_ERROR("Device is nullptr");
			return false;
		}

		if (bufferSize == 0)
		{
			LOG_ERROR("Buffer size cannot be zero");
			return false;
		}

		if (frameCount == 0 || frameCount > FRAME_BUFFER_COUNT)
		{
			LOG_ERROR("Invalid frame count: %u (must be 1-%u)", frameCount, FRAME_BUFFER_COUNT);
			return false;
		}

		// 256바이트 정렬
		mAlignedBufferSize = AlignSize(bufferSize);
		mFrameCount = frameCount;

		// 전체 버퍼 크기 계산 (프레임별 독립 메모리)
		size_t totalBufferSize = mAlignedBufferSize * mFrameCount;

		// Upload Heap 속성 설정
		D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

		// 버퍼 리소스 설정
		D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(static_cast<UINT64>(totalBufferSize));

		// Upload Heap에 Constant Buffer 생성
		HRESULT hr = device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,  // Upload Heap의 초기 상태
			nullptr,
			IID_PPV_ARGS(&mConstantBuffer)
		);

		if (FAILED(hr))
		{
			LOG_ERROR("Failed to create Constant Buffer (HRESULT: 0x%08X)", hr);
			return false;
		}

		// Upload Heap을 지속적으로 Map (프로그램 종료까지 유지)
		CD3DX12_RANGE readRange(0, 0);  // CPU에서 읽지 않음
		hr = mConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mMappedData));

		if (FAILED(hr))
		{
			LOG_ERROR("Failed to map Constant Buffer (HRESULT: 0x%08X)", hr);
			mConstantBuffer.Reset();
			return false;
		}

		LOG_INFO(
			"DX12ConstantBuffer initialized (Original: %zu bytes, Aligned: %zu bytes, Frames: %u, Total: %zu bytes, GPU: 0x%llX)",
			bufferSize,
			mAlignedBufferSize,
			mFrameCount,
			totalBufferSize,
			mConstantBuffer->GetGPUVirtualAddress()
		);

		return true;
	}

	void DX12ConstantBuffer::Shutdown()
	{
		// Upload Heap은 Map된 상태로 유지되므로 Unmap 필요
		if (mConstantBuffer && mMappedData)
		{
			mConstantBuffer->Unmap(0, nullptr);
			mMappedData = nullptr;
		}

		mConstantBuffer.Reset();
		mAlignedBufferSize = 0;
		mFrameCount = 0;

		LOG_INFO("DX12ConstantBuffer shutdown completed");
	}

	void DX12ConstantBuffer::Update(
		uint32 frameIndex,
		const void* data,
		size_t dataSize
	)
	{
		if (!mMappedData)
		{
			CORE_ASSERT(false, "Constant Buffer is not initialized or mapped");
			LOG_ERROR("Constant Buffer is not initialized or mapped");
			return;
		}

		if (frameIndex >= mFrameCount)
		{
			CORE_ASSERT(false, "Invalid frame index: %u (max: %u)", frameIndex, mFrameCount - 1);
			LOG_ERROR("Invalid frame index: %u (max: %u)", frameIndex, mFrameCount - 1);
			return;
		}

		if (dataSize > mAlignedBufferSize)
		{
			LOG_ERROR(
				"Data size (%zu bytes) exceeds buffer size (%zu bytes)",
				dataSize,
				mAlignedBufferSize
			);
			return;
		}

		if (!data)
		{
			LOG_ERROR("Data pointer is nullptr");
			return;
		}

		// 해당 프레임의 버퍼 오프셋 계산
		size_t offset = mAlignedBufferSize * frameIndex;

		// 데이터 복사 (GPU가 직접 읽을 수 있는 Upload Heap 메모리)
		memcpy(mMappedData + offset, data, dataSize);

		// NOTE: Upload Heap은 Write-Combined 메모리이므로
		// memcpy 후 자동으로 GPU에 전달됨 (Unmap 불필요)
	}

	void DX12ConstantBuffer::UpdateAtOffset(
		Core::uint32 frameIndex,
		Core::uint32 slotIndex,
		const void* data,
		size_t dataSize,
		size_t slotSize
	)
	{
		if (!mMappedData)
		{
			LOG_ERROR("Constant Buffer is not initialized or mapped");
			return;
		}

		if (frameIndex >= mFrameCount)
		{
			LOG_ERROR("Invalid frame index: %u (max: %u)", frameIndex, mFrameCount - 1);
			return;
		}

		if (!data)
		{
			LOG_ERROR("Data pointer is nullptr");
			return;
		}

		if (dataSize > slotSize)
		{
			LOG_ERROR("Data size (%zu) exceeds slot size (%zu)", dataSize, slotSize);
			return;
		}

		// 현재 프레임의 시작 위치 + 슬롯 오프셋
		size_t frameOffset = mAlignedBufferSize * frameIndex;
		size_t objectOffset = slotSize * slotIndex;
		size_t totalOffset = frameOffset + objectOffset;

		// 범위 체크 (안전성)
		size_t totalBufferSize = mAlignedBufferSize * mFrameCount;
		if (totalOffset + dataSize > totalBufferSize)
		{
			LOG_ERROR(
				"Buffer overflow: offset(%zu) + size(%zu) > total(%zu)",
				totalOffset, dataSize, totalBufferSize
			);
			return;
		}

		// 데이터 복사
		memcpy(mMappedData + totalOffset, data, dataSize);
	}

	D3D12_GPU_VIRTUAL_ADDRESS DX12ConstantBuffer::GetGPUAddress(uint32 frameIndex) const
	{
		if (!mConstantBuffer)
		{
			LOG_ERROR("Constant Buffer is not initialized");
			return 0;
		}

		if (frameIndex >= mFrameCount)
		{
			LOG_ERROR("Invalid frame index: %u (max: %u)", frameIndex, mFrameCount - 1);
			return 0;
		}

		// 기본 GPU 주소 + 프레임별 오프셋
		D3D12_GPU_VIRTUAL_ADDRESS baseAddress = mConstantBuffer->GetGPUVirtualAddress();
		size_t offset = mAlignedBufferSize * frameIndex;

		return baseAddress + offset;
	}

	size_t DX12ConstantBuffer::AlignSize(size_t size)
	{
		// DirectX 12 Constant Buffer는 256바이트 정렬 필수
		constexpr size_t ALIGNMENT = 256;
		return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
	}

} // namespace Graphics
