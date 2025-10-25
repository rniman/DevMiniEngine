// Graphics/include/Graphics/DX12/DX12GraphicsRootSignature.h
#pragma once
#include "Graphics/GraphicsTypes.h"

namespace Graphics
{
	/**
	 * @brief DirectX 12 그래픽스 파이프라인용 루트 시그니처
	 *
	 * 셰이더가 접근할 수 있는 리소스 레이아웃을 정의합니다.
	 * 현재는 빈 루트 시그니처를 생성하며, 향후 빌더 패턴으로 확장 예정입니다.
	 *
	 */
	class DX12GraphicsRootSignature
	{
	public:
		/**
		 * @brief 빈 루트 시그니처를 생성합니다
		 *
		 * @param device DirectX 12 디바이스
		 * @return 성공 시 true, 실패 시 false
		 *
		 * @note 실패 시 에러 로그가 자동으로 출력됩니다
		 */
		bool Initialize(ID3D12Device* device);

		ID3D12RootSignature* Get() const { return mRootSignature.Get(); }

	private:
		ComPtr<ID3D12RootSignature> mRootSignature;
	};

} // namespace Graphics