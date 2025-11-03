#pragma once
#include "Graphics/GraphicsTypes.h"

namespace Graphics
{
	/**
	 * @brief DirectX 12 Root Signature를 관리하는 클래스
	 *
	 * Root Signature는 셰이더가 접근할 리소스의 레이아웃을 정의합니다.
	 * GPU 파이프라인과 셰이더 간의 "인터페이스 계약" 역할을 하며,
	 * Constant Buffer, Texture, Sampler 등의 바인딩 구조를 명시합니다.
	 *
	 * 지원하는 Root Parameter 타입:
	 * - Descriptor Table: CBV, SRV, UAV, Sampler의 연속된 범위
	 * - Root Constants: 32bit 값을 직접 전달 (최대 64개)
	 * - Root Descriptor: 단일 CBV/SRV/UAV 직접 바인딩
	 *
	 * DirectX 12에서 Root Signature 변경은 파이프라인 상태 변경을 유발하므로
	 * 가능한 적게 변경하고 재사용하는 것이 성능상 유리합니다.
	 *
	 * @note PSO(Pipeline State Object) 생성 시 Root Signature 필요
	 * @note Draw 호출 전 CommandList에 바인딩 필수
	 * @warning Root Signature 변경은 GPU 파이프라인 플러시를 유발
	 */
	class DX12RootSignature
	{
	public:
		DX12RootSignature() = default;
		~DX12RootSignature();

		DX12RootSignature(const DX12RootSignature&) = delete;
		DX12RootSignature& operator=(const DX12RootSignature&) = delete;

		/**
		 * @brief Root Signature 초기화 (Version 1.1)
		 *
		 * DirectX 12.1부터 지원되는 Version 1.1을 사용합니다.
		 * Version 1.1은 Descriptor의 정적/동적 속성을 명시하여
		 * 드라이버 최적화 기회를 제공합니다.
		 *
		 * @param device DirectX 12 Device
		 * @param numParameters Root Parameter 개수
		 * @param parameters Root Parameter 배열 (CD3DX12_ROOT_PARAMETER1)
		 * @param numStaticSamplers Static Sampler 개수
		 * @param staticSamplers Static Sampler 배열
		 * @param flags Root Signature 플래그
		 * @return 초기화 성공 여부
		 *
		 * @note 내부적으로 D3DX12SerializeVersionedRootSignature 사용
		 * @warning parameters 배열은 이 함수가 반환될 때까지 유효해야 합니다
		 */
		bool Initialize(
			ID3D12Device* device,
			Core::uint32 numParameters,
			const D3D12_ROOT_PARAMETER1* parameters,
			Core::uint32 numStaticSamplers = 0,
			const D3D12_STATIC_SAMPLER_DESC* staticSamplers = nullptr,
			D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		);

		/**
		 * @brief Empty Root Signature 초기화 (파라미터 없음)
		 *
		 * 리소스 바인딩이 필요 없는 단순한 셰이더를 위한 Root Signature입니다.
		 * Vertex Input만 사용하는 경우 적합합니다.
		 *
		 * @param device DirectX 12 Device
		 * @return 초기화 성공 여부
		 */
		bool InitializeEmpty(ID3D12Device* device);

		void Shutdown();

		// Getters
		ID3D12RootSignature* GetRootSignature() const { return mRootSignature.Get(); }
		bool IsInitialized() const { return mRootSignature != nullptr; }

	private:
		ComPtr<ID3D12RootSignature> mRootSignature;
	};

} // namespace Graphics