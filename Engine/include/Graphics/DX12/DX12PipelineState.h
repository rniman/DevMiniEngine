// Graphics/include/Graphics/DX12/DX12PipelineState.h
#pragma once
#include "Graphics/GraphicsTypes.h"

namespace Graphics
{
	/**
	 * @brief DirectX 12 Pipeline State Object 관리 클래스
	 *
	 * D3D12_GRAPHICS_PIPELINE_STATE_DESC를 직접 사용하여
	 * DirectX 12 문서와 1:1 대응되도록 설계되었습니다.
	 */
	class DX12PipelineState
	{
	public:
		DX12PipelineState() = default;
		~DX12PipelineState() = default;

		DX12PipelineState(const DX12PipelineState&) = delete;
		DX12PipelineState& operator=(const DX12PipelineState&) = delete;

		/**
		 * @brief Graphics Pipeline State Object를 생성합니다
		 *
		 * @param device DirectX 12 디바이스
		 * @param desc D3D12 표준 PSO Descriptor
		 * @return 성공 시 true, 실패 시 false
		 */
		bool Initialize(
			ID3D12Device* device,
			const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc
		);

		void Shutdown();

		ID3D12PipelineState* Get() const { return mPipelineState.Get(); }
		bool IsInitialized() const { return mPipelineState != nullptr; }

		/**
		 * @brief 기본 Graphics PSO Descriptor를 생성하는 헬퍼 함수
		 *
		 * 첫 삼각형 렌더링에 적합한 기본 설정을 제공합니다:
		 * - Solid fill, back-face culling
		 * - No blending
		 * - Depth test disabled
		 *
		 * @param rootSignature 루트 시그니처
		 * @param vs 버텍스 셰이더 바이트코드
		 * @param ps 픽셀 셰이더 바이트코드
		 * @param inputLayout Input Layout Descriptor
		 * @return 기본값이 설정된 PSO Descriptor
		 */
		static D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateDefaultDesc(
			ID3D12RootSignature* rootSignature,
			ID3DBlob* vs,
			ID3DBlob* ps,
			const D3D12_INPUT_LAYOUT_DESC& inputLayout
		);

	private:
		ComPtr<ID3D12PipelineState> mPipelineState;
	};

} // namespace Graphics