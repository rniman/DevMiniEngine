#pragma once

#include "Graphics/GraphicsTypes.h"
#include <unordered_map>
#include <memory>

namespace Graphics
{
	class Material;
	class DX12ShaderCompiler;

	/**
	 * @brief PSO 생성 및 캐싱을 관리하는 클래스
	 *
	 * Material 기반으로 Pipeline State Object를 생성하고,
	 * 동일한 설정의 PSO 재생성을 방지합니다.
	 */
	class DX12PipelineStateCache
	{
	public:
		DX12PipelineStateCache() = default;
		~DX12PipelineStateCache();

		DX12PipelineStateCache(const DX12PipelineStateCache&) = delete;
		DX12PipelineStateCache& operator=(const DX12PipelineStateCache&) = delete;

		/**
		 * @brief DX12PipelineStateCache 초기화
		 *
		 * @param device D3D12 Device
		 * @param shaderCompiler 셰이더 컴파일러
		 * @return 성공 시 true
		 */
		bool Initialize(ID3D12Device* device, DX12ShaderCompiler* shaderCompiler);

		/**
		 * @brief Material 기반 PSO 가져오기 또는 생성
		 *
		 * 캐시에 동일한 설정의 PSO가 있으면 반환하고, 없으면 새로 생성합니다.
		 *
		 * @param material 렌더링 설정이 담긴 Material
		 * @param rootSignature Root Signature
		 * @param inputLayout Vertex Input Layout
		 * @return 생성되거나 캐싱된 PSO (실패 시 nullptr)
		 */
		ID3D12PipelineState* GetOrCreatePipelineState(
			const Material& material,
			ID3D12RootSignature* rootSignature,
			const D3D12_INPUT_LAYOUT_DESC& inputLayout
		);

		/**
		 * @brief 모든 PSO 캐시 정리
		 */
		void Clear();

		
		void Shutdown();

		/**
		 * @brief 캐시된 PSO 개수 반환
		 */
		size_t GetCachedPSOCount() const { return mPSOCache.size(); }

	private:
		/**
		 * @brief PSO 캐시 키 구조체
		 *
		 * Material, InputLayout, RootSignature 조합으로 PSO를 고유하게 식별합니다.
		 */
		struct PSOKey
		{
			size_t materialHash = 0;
			size_t inputLayoutHash = 0;
			ID3D12RootSignature* rootSignature = nullptr;

			bool operator==(const PSOKey& other) const
			{
				return materialHash == other.materialHash &&
					inputLayoutHash == other.inputLayoutHash &&
					rootSignature == other.rootSignature;
			}
		};

		/**
		 * @brief PSOKey 해시 계산 함수 객체
		 */
		struct PSOKeyHasher
		{
			size_t operator()(const PSOKey& key) const
			{
				size_t hash = key.materialHash;
				hash ^= key.inputLayoutHash + 0x9e3779b9 + (hash << 6) + (hash >> 2);
				hash ^= std::hash<void*>()(key.rootSignature) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
				return hash;
			}
		};

		/**
		 * @brief PSO 생성 헬퍼 함수
		 */
		ComPtr<ID3D12PipelineState> CreatePSO(
			const Material& material,
			ID3D12RootSignature* rootSignature,
			const D3D12_INPUT_LAYOUT_DESC& inputLayout
		);

		/**
		 * @brief 셰이더 컴파일 및 PSO Desc 구성
		 */
		bool CompileShadersAndFillDesc(
			const Material& material,
			D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc,
			ComPtr<ID3DBlob>& vsBlob,
			ComPtr<ID3DBlob>& psBlob
		);

		/**
		 * @brief Input Layout 해시 계산
		 */
		static size_t HashInputLayout(const D3D12_INPUT_LAYOUT_DESC& inputLayout);

		// PSO 캐시 (Key -> PSO)
		std::unordered_map<PSOKey, ComPtr<ID3D12PipelineState>, PSOKeyHasher> mPSOCache;

		// D3D12 리소스 (소유하지 않음)
		ID3D12Device* mDevice = nullptr;
		DX12ShaderCompiler* mShaderCompiler = nullptr;
	};

} // namespace Graphics