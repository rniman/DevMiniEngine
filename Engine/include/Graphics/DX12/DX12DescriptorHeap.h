#pragma once
#include "Graphics/GraphicsTypes.h"
#include <queue>

namespace Graphics
{
	// 추가: 유효하지 않은 인덱스 상수
	constexpr uint32 INVALID_DESCRIPTOR_INDEX = static_cast<uint32>(-1);

	/**
	 * @brief DirectX 12 Descriptor Heap 관리 클래스
	 *
	 * GPU 리소스에 대한 Descriptor들을 관리하는 Heap입니다.
	 * RTV, DSV, CBV/SRV/UAV, Sampler 타입을 지원합니다.
	 *
	 * 연속된 Descriptor 블록 할당을 지원하여
	 * Descriptor Table 사용 시 필요한 연속 메모리를 보장합니다.
	 */
	class DX12DescriptorHeap
	{
	public:
		DX12DescriptorHeap() = default;
		~DX12DescriptorHeap();

		// Non-copyable
		DX12DescriptorHeap(const DX12DescriptorHeap&) = delete;
		DX12DescriptorHeap& operator=(const DX12DescriptorHeap&) = delete;

		/**
		 * @brief Descriptor Heap 초기화
		 *
		 * @param device D3D12 Device
		 * @param type Descriptor Heap 타입
		 * @param numDescriptors Descriptor 개수
		 * @param shaderVisible Shader에서 접근 가능 여부
		 * @return 초기화 성공 여부
		 */
		bool Initialize(
			ID3D12Device* device,
			D3D12_DESCRIPTOR_HEAP_TYPE type,
			uint32 numDescriptors,
			bool shaderVisible = false
		);

		void Shutdown();

		/**
		 * @brief 연속된 Descriptor 블록 할당
		 *
		 * Descriptor Table을 사용할 때 필요한 연속된 Descriptor 블록을 할당합니다.
		 * 예: Material의 모든 텍스처를 위한 7개의 연속 슬롯
		 *
		 * @param count 필요한 연속 Descriptor 개수
		 * @return 블록 시작 인덱스 (실패 시 INVALID_DESCRIPTOR_INDEX)
		 *
		 * @note 현재는 간단한 순차 할당 방식 사용
		 * @warning 할당된 블록은 FreeBlock으로 해제해야 합니다
		 */
		uint32 AllocateBlock(uint32 count);

		/**
		 * @brief Descriptor 블록 해제
		 *
		 * AllocateBlock으로 할당한 블록을 해제하여 재사용 가능하게 합니다.
		 *
		 * @param startIndex 블록 시작 인덱스
		 * @param count 블록 크기
		 *
		 * @note 현재는 Free List에 추가만 하고 실제 재사용은 미구현
		 * @warning 잘못된 인덱스나 크기를 전달하면 Heap 손상 가능
		 */
		void FreeBlock(uint32 startIndex, uint32 count);

		/**
		 * @brief CPU Descriptor Handle 획득 (인덱스 기반)
		 *
		 * @param index Descriptor 인덱스 (0 ~ numDescriptors-1)
		 * @return CPU Descriptor Handle
		 *
		 * @warning 인덱스 범위 검증을 수행하지 않습니다
		 */
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32 index) const;

		/**
		 * @brief GPU Descriptor Handle 획득 (인덱스 기반)
		 *
		 * @param index Descriptor 인덱스 (0 ~ numDescriptors-1)
		 * @return GPU Descriptor Handle
		 *
		 * @warning 인덱스 범위 검증을 수행하지 않습니다
		 * @note Shader Visible Heap에서만 유효합니다
		 */
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32 index) const;

		// Getters
		ID3D12DescriptorHeap* GetHeap() const { return mHeap.Get(); }
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUStart() const { return mCPUStart; }
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUStart() const { return mGPUStart; }
		uint32 GetDescriptorSize() const { return mDescriptorSize; }
		uint32 GetNumDescriptors() const { return mNumDescriptors; }
		D3D12_DESCRIPTOR_HEAP_TYPE GetType() const { return mType; }
		bool IsInitialized() const { return mHeap != nullptr; }

		/**
		 * @brief 할당 통계 조회
		 */
		uint32 GetAllocatedCount() const { return mAllocatedCount; }
		uint32 GetAvailableCount() const { return mNumDescriptors - mAllocatedCount; }

	private:
		const char* GetDescriptorHeapTypeString(D3D12_DESCRIPTOR_HEAP_TYPE type) const;

		// 멤버 변수

		// Descriptor Heap Resources
		ComPtr<ID3D12DescriptorHeap> mHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE mCPUStart{};
		D3D12_GPU_DESCRIPTOR_HANDLE mGPUStart{};

		// Heap Properties
		uint32 mDescriptorSize = 0;
		uint32 mNumDescriptors = 0;
		D3D12_DESCRIPTOR_HEAP_TYPE mType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

		// Allocation Management
		uint32 mNextFreeIndex = 0;        // 다음 할당 가능한 인덱스
		uint32 mAllocatedCount = 0;       // 현재 할당된 Descriptor 개수

		// 해제된 블록을 추적하는 구조체
		struct DescriptorBlock
		{
			uint32 startIndex;
			uint32 count;
		};
		std::queue<DescriptorBlock> mFreeBlocks;  // 타입 이름 변경
	};

} // namespace Graphics
