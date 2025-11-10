#include "pch.h"
#include "Graphics/DX12/DX12DescriptorHeap.h"

namespace Graphics
{
	DX12DescriptorHeap::~DX12DescriptorHeap()
	{
		Shutdown();
	}

	bool DX12DescriptorHeap::Initialize(
		ID3D12Device* device,
		D3D12_DESCRIPTOR_HEAP_TYPE type,
		uint32 numDescriptors,
		bool shaderVisible
	)
	{
		if (!device)
		{
			LOG_ERROR("[DX12DescriptorHeap] Device is null");
			return false;
		}

		if (numDescriptors == 0)
		{
			LOG_ERROR("[DX12DescriptorHeap] Invalid descriptor count: 0");
			return false;
		}

		LOG_INFO(
			"[DX12DescriptorHeap] Initializing Descriptor Heap (%s, %u descriptors)...",
			GetDescriptorHeapTypeString(type),
			numDescriptors
		);

		mType = type;
		mNumDescriptors = numDescriptors;

		// Descriptor Heap 생성
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type = type;
		heapDesc.NumDescriptors = static_cast<UINT>(numDescriptors);
		heapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		heapDesc.NodeMask = 0;

		HRESULT hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mHeap));
		if (FAILED(hr))
		{
			LOG_ERROR("[DX12DescriptorHeap] Failed to create Descriptor Heap (HRESULT: 0x%08X)", hr);
			return false;
		}

		// Descriptor 크기 가져오기
		mDescriptorSize = device->GetDescriptorHandleIncrementSize(type);

		// 시작 핸들 가져오기
		mCPUStart = mHeap->GetCPUDescriptorHandleForHeapStart();
		if (shaderVisible)
		{
			mGPUStart = mHeap->GetGPUDescriptorHandleForHeapStart();
		}

		// 할당 상태 초기화
		mNextFreeIndex = 0;
		mAllocatedCount = 0;

		LOG_INFO("[DX12DescriptorHeap] Descriptor Heap created successfully");
		LOG_INFO("[DX12DescriptorHeap]   Descriptor Size: %u bytes", mDescriptorSize);
		LOG_INFO("[DX12DescriptorHeap]   Shader Visible: %s", shaderVisible ? "Yes" : "No");

		return true;
	}

	void DX12DescriptorHeap::Shutdown()
	{
		if (!IsInitialized())
		{
			return;
		}

		LOG_INFO("[DX12DescriptorHeap] Shutting down Descriptor Heap...");
		LOG_INFO(
			"[DX12DescriptorHeap]   Final allocation: %u / %u descriptors used",
			mAllocatedCount,
			mNumDescriptors
		);

		mHeap.Reset();
		mCPUStart = {};
		mGPUStart = {};
		mNextFreeIndex = 0;
		mAllocatedCount = 0;

		// Free List 비우기
		while (!mFreeBlocks.empty())
		{
			mFreeBlocks.pop();
		}

		LOG_INFO("[DX12DescriptorHeap] Descriptor Heap shut down successfully");
	}

	uint32 DX12DescriptorHeap::AllocateBlock(uint32 count)
	{
		if (!IsInitialized())
		{
			LOG_ERROR("[DX12DescriptorHeap] Cannot allocate: Heap not initialized");
			return INVALID_DESCRIPTOR_INDEX;
		}

		if (count == 0)
		{
			LOG_ERROR("[DX12DescriptorHeap] Cannot allocate block of size 0");
			return INVALID_DESCRIPTOR_INDEX;
		}

		// TODO: Free List에서 재사용 가능한 블록 찾기 (향후 구현)
		// 현재는 간단한 순차 할당만 지원

		// 공간이 충분한지 확인
		if (mNextFreeIndex + count > mNumDescriptors)
		{
			LOG_ERROR(
				"[DX12DescriptorHeap] Out of descriptor space: requested %u, available %u",
				count,
				mNumDescriptors - mNextFreeIndex
			);
			return INVALID_DESCRIPTOR_INDEX;
		}

		// 블록 할당
		uint32 startIndex = mNextFreeIndex;
		mNextFreeIndex += count;
		mAllocatedCount += count;

		LOG_TRACE(
			"[DX12DescriptorHeap] Allocated block [%u ~ %u] (%u descriptors)",
			startIndex,
			startIndex + count - 1,
			count
		);

		return startIndex;
	}

	void DX12DescriptorHeap::FreeBlock(uint32 startIndex, uint32 count)
	{
		if (!IsInitialized())
		{
			LOG_ERROR("[DX12DescriptorHeap] Cannot free: Heap not initialized");
			return;
		}

		if (count == 0)
		{
			LOG_WARN("[DX12DescriptorHeap] Attempting to free block of size 0");
			return;
		}

		if (startIndex + count > mNumDescriptors)
		{
			LOG_ERROR(
				"[DX12DescriptorHeap] Invalid free range [%u ~ %u] exceeds heap size %u",
				startIndex,
				startIndex + count - 1,
				mNumDescriptors
			);
			return;
		}

		LOG_TRACE(
			"[DX12DescriptorHeap] Freed block [%u ~ %u] (%u descriptors)",
			startIndex,
			startIndex + count - 1,
			count
		);

		// Free List에 추가
		mFreeBlocks.push({ startIndex, count });

		if (mAllocatedCount >= count)
		{
			mAllocatedCount -= count;
		}
		else
		{
			LOG_WARN("[DX12DescriptorHeap] Allocation count underflow during free");
			mAllocatedCount = 0;
		}

		// TODO: Free List 병합 및 재사용 로직 구현
		// 현재는 단순히 Free List에 추가만 함
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DX12DescriptorHeap::GetCPUHandle(uint32 index) const
	{
		if (!IsInitialized())
		{
			LOG_ERROR("[DX12DescriptorHeap] Descriptor Heap not initialized");
			return {};
		}

		if (index >= mNumDescriptors)
		{
			LOG_ERROR("[DX12DescriptorHeap] Index out of range: %u (max: %u)", index, mNumDescriptors - 1);
			return {};
		}

		D3D12_CPU_DESCRIPTOR_HANDLE handle = mCPUStart;
		handle.ptr += static_cast<SIZE_T>(index) * mDescriptorSize;
		return handle;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE DX12DescriptorHeap::GetGPUHandle(uint32 index) const
	{
		if (!IsInitialized())
		{
			LOG_ERROR("[DX12DescriptorHeap] Descriptor Heap not initialized");
			return {};
		}

		if (index >= mNumDescriptors)
		{
			LOG_ERROR("[DX12DescriptorHeap] Index out of range: %u (max: %u)", index, mNumDescriptors - 1);
			return {};
		}

		D3D12_GPU_DESCRIPTOR_HANDLE handle = mGPUStart;
		handle.ptr += static_cast<UINT64>(index) * mDescriptorSize;
		return handle;
	}

	const char* DX12DescriptorHeap::GetDescriptorHeapTypeString(D3D12_DESCRIPTOR_HEAP_TYPE type) const
	{
		switch (type)
		{
		case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
			return "CBV/SRV/UAV";
		case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
			return "Sampler";
		case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
			return "RTV";
		case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
			return "DSV";
		default:
			return "Unknown";
		}
	}

} // namespace Graphics
