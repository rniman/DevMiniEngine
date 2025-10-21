#pragma once
#include "Graphics/GraphicsTypes.h"

namespace Graphics
{
	/**
	 * @brief DirectX 12 Descriptor Heap 관리 클래스
	 *
	 * GPU 리소스에 대한 Descriptor들을 관리하는 Heap입니다.
	 * RTV, DSV, CBV/SRV/UAV, Sampler 타입을 지원합니다.
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
			Core::uint32 numDescriptors,
			bool shaderVisible = false
		);

		void Shutdown();

		/**
		 * @brief CPU Descriptor Handle 획득 (인덱스 기반)
		 *
		 * @param index Descriptor 인덱스 (0 ~ numDescriptors-1)
		 * @return CPU Descriptor Handle
		 *
		 * @warning 인덱스 범위 검증을 수행하지 않습니다
		 */
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(Core::uint32 index) const;

		/**
		 * @brief GPU Descriptor Handle 획득 (인덱스 기반)
		 *
		 * @param index Descriptor 인덱스 (0 ~ numDescriptors-1)
		 * @return GPU Descriptor Handle
		 *
		 * @warning 인덱스 범위 검증을 수행하지 않습니다
		 * @note Shader Visible Heap에서만 유효합니다
		 */
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(Core::uint32 index) const;

		// Getters
		ID3D12DescriptorHeap* GetHeap() const { return mHeap.Get(); }
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUStart() const { return mCPUStart; }
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUStart() const { return mGPUStart; }
		Core::uint32 GetDescriptorSize() const { return mDescriptorSize; }
		Core::uint32 GetNumDescriptors() const { return mNumDescriptors; }
		D3D12_DESCRIPTOR_HEAP_TYPE GetType() const { return mType; }
		bool IsInitialized() const { return mHeap != nullptr; }

	private:
		const char* GetDescriptorHeapTypeString(D3D12_DESCRIPTOR_HEAP_TYPE type) const;

		// 멤버 변수
		
		// Descriptor Heap Resources
		ComPtr<ID3D12DescriptorHeap> mHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE mCPUStart;
		D3D12_GPU_DESCRIPTOR_HANDLE mGPUStart;

		// Heap Properties
		Core::uint32 mDescriptorSize;
		Core::uint32 mNumDescriptors;
		D3D12_DESCRIPTOR_HEAP_TYPE mType;
	};

} // namespace Graphics