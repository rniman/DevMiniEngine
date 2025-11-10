#pragma once
#include "Graphics/GraphicsTypes.h"
#include "Graphics/TextureType.h"
#include "Framework/Resources/ResourceId.h" 
#include <string>
#include <array>


namespace Framework
{
	class ResourceManager;
}

namespace Graphics
{
	class DX12DescriptorHeap;
	class Texture;

	/**
	 * @brief 블렌드 모드 프리셋 열거형
	 *
	 * D3D12 블렌드 상태의 일반적인 조합을 제공합니다.
	 */
	enum class BlendMode
	{
		Opaque,      // 불투명 (블렌딩 비활성화)
		AlphaBlend,  // 알파 블렌딩 (SrcAlpha, InvSrcAlpha)
		Additive     // 가산 블렌딩 (SrcAlpha, One)
	};

	/**
	 * @brief Material 생성을 위한 설정 구조체
	 */
	struct MaterialDesc
	{
		const wchar_t* vertexShaderPath = L"BasicShader.hlsl";
		const wchar_t* pixelShaderPath = L"BasicShader.hlsl";
		const char* vsEntryPoint = "VSMain";
		const char* psEntryPoint = "PSMain";

		// 블렌드 모드 (프리셋 제공)
		BlendMode blendMode = BlendMode::Opaque;

		// D3D12 네이티브 타입 직접 사용
		D3D12_CULL_MODE cullMode = D3D12_CULL_MODE_BACK;
		D3D12_FILL_MODE fillMode = D3D12_FILL_MODE_SOLID;

		bool depthTestEnabled = true;
		bool depthWriteEnabled = true;
		D3D12_COMPARISON_FUNC depthComparisonFunc = D3D12_COMPARISON_FUNC_LESS;

		// PSO 생성에 필요한 추가 설정
		D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		uint32 numRenderTargets = 1;
		DXGI_FORMAT rtvFormats[8] = { DXGI_FORMAT_R8G8B8A8_UNORM };  // 최대 8개
		DXGI_FORMAT dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		// MSAA 설정
		uint32 sampleCount = 1;
		uint32 sampleQuality = 0;
		uint32 sampleMask = 0xFFFFFFFF;

		// 텍스쳐 경로
		// const wchar_t* diffuseTexturePath = nullptr;
		// const wchar_t* normalTexturePath = nullptr;
		// const wchar_t* specularTexturePath = nullptr;
	};



	/**
	 * @brief 렌더링에 필요한 셰이더, 상태, 파라미터를 캡슐화하는 Material 클래스
	 *
	 * Material은 PSO 생성에 필요한 모든 렌더링 상태 정보를 관리합니다.
	 * 셰이더 경로, 블렌드 상태, 래스터라이저 상태, 깊이-스텐실 상태를 포함합니다.
	 */
	class Material
	{
	public:
		/**
		 * @brief MaterialDesc를 사용한 생성자
		 *
		 * @param desc Material 설정 정보
		 */
		explicit Material(const MaterialDesc& desc);

		/**
		 * @brief 기본 생성자 (Opaque Material)
		 */
		Material();
		~Material() = default;

		// 복사, 이동 허용 
		Material(const Material&) = default;
		Material& operator=(const Material&) = default;
		Material(Material&&) noexcept = default;
		Material& operator=(Material&&) noexcept = default;

		/**
		 * @brief Material의 모든 텍스처를 위한 Descriptor 블록 할당
		 *
		 * Descriptor Table 사용을 위해 연속된 7개 슬롯을 할당합니다.
		 * 텍스처가 없는 슬롯에는 Dummy SRV를 생성합니다.
		 *
		 * @param device D3D12 Device
		 * @param heap Descriptor Heap (CBV_SRV_UAV 타입)
		 * @param resourceMgr ResourceManager (텍스쳐 조회)
		 * @return 할당 성공 여부
		 */
		bool AllocateDescriptors(
			ID3D12Device* device,
			DX12DescriptorHeap* heap,
			Framework::ResourceManager* resourceMgr
		);

		/**
		 * @brief Descriptor 블록 해제
		 *
		 * @param heap Descriptor Heap
		 */
		void FreeDescriptors(DX12DescriptorHeap* heap);

		// ResourceId 기반 텍스쳐 설정
		void SetTexture(TextureType type, Framework::ResourceId textureId);
		Framework::ResourceId GetTextureId(TextureType type) const;
		bool HasTexture(TextureType type) const;
		uint32 GetTextureCount() const;

		// Getters
		const wchar_t* GetVertexShaderPath() const { return mVertexShaderPath.c_str(); }
		const wchar_t* GetPixelShaderPath() const { return mPixelShaderPath.c_str(); }
		const char* GetVSEntryPoint() const { return mVSEntryPoint.c_str(); }
		const char* GetPSEntryPoint() const { return mPSEntryPoint.c_str(); }

		D3D12_BLEND_DESC GetBlendState() const { return mBlendDesc; }
		D3D12_RASTERIZER_DESC GetRasterizerState() const { return mRasterizerDesc; }
		D3D12_DEPTH_STENCIL_DESC GetDepthStencilState() const { return mDepthStencilDesc; }

		// PSO 생성 시 필요한 추가 설정
		D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveTopology() const { return mPrimitiveTopology; }
		uint32 GetNumRenderTargets() const { return mNumRenderTargets; }
		const DXGI_FORMAT* GetRTVFormats() const { return mRTVFormats; }
		DXGI_FORMAT GetDSVFormat() const { return mDSVFormat; }
		uint32 GetSampleCount() const { return mSampleCount; }
		uint32 GetSampleQuality() const { return mSampleQuality; }
		uint32 GetSampleMask() const { return mSampleMask; }

		//// Texture 관련
		//std::shared_ptr<Texture> GetTexture(TextureType type) const;
		//bool HasTexture(TextureType type) const;
		//uint32 GetTextureCount() const;

		/**
		 * @brief Descriptor Table의 GPU Handle 획득
		 *
		 * 렌더링 시 SetGraphicsRootDescriptorTable에 사용합니다.
		 *
		 * @param heap Descriptor Heap
		 * @return Descriptor Table 시작 GPU Handle
		 */
		D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorTableHandle(const DX12DescriptorHeap* heap) const;
		/**
		 * @brief Descriptor가 할당되었는지 확인
		 */
		bool HasAllocatedDescriptors() const;


		/**
		 * @brief ResourceId 기반 텍스처 순회
		 *
		 * 렌더링 시 바인딩할 텍스처를 순회합니다.
		 * 
		 * @param func 각 텍스처에 대해 호출할 함수
		 *
		 */
		template<typename Func>
		void ForEachTextureId(Func&& func) const
		{
			for (size_t i = 0; i < static_cast<size_t>(TextureType::Count); ++i)
			{
				if (mTextureIds[i].IsValid())
				{
					func(static_cast<TextureType>(i), mTextureIds[i]);
				}
			}
		}

		/**
		 * @brief Material 설정 기반 해시 값 계산
		 *
		 * PSO 캐싱에 사용됩니다.
		 *
		 * @return Material 고유 해시 값
		 */
		size_t GetHash() const;

	private:
		/**
		 * @brief BlendMode를 D3D12_BLEND_DESC로 변환
		 */
		static D3D12_BLEND_DESC CreateBlendDesc(BlendMode mode);

		/**
		 * @brief 깊이 테스트 설정을 D3D12_DEPTH_STENCIL_DESC로 변환
		 */
		static D3D12_DEPTH_STENCIL_DESC CreateDepthStencilDesc(
			bool depthTest,
			bool depthWrite,
			D3D12_COMPARISON_FUNC depthComparisonFunc
		);

		/**
		 * @brief 빈 텍스처 슬롯에 Dummy SRV 생성
		 *
		 * @param device D3D12 Device
		 * @param heap Descriptor Heap
		 * @param index Descriptor 인덱스
		 */
		void CreateDummySRV(
			ID3D12Device* device,
			DX12DescriptorHeap* heap,
			uint32 index
		);

		/**
		 * @brief 문자열 해시 계산 헬퍼 함수
		 */
		static size_t HashString(const std::wstring& str);
		static size_t HashString(const std::string& str);

		// 셰이더 정보
		std::wstring mVertexShaderPath;
		std::wstring mPixelShaderPath;
		std::string mVSEntryPoint;
		std::string mPSEntryPoint;

		// 렌더 상태
		D3D12_BLEND_DESC mBlendDesc;
		D3D12_RASTERIZER_DESC mRasterizerDesc;
		D3D12_DEPTH_STENCIL_DESC mDepthStencilDesc;

		// PSO 생성 설정
		D3D12_PRIMITIVE_TOPOLOGY_TYPE mPrimitiveTopology;
		uint32 mNumRenderTargets;
		DXGI_FORMAT mRTVFormats[8];  // D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT
		DXGI_FORMAT mDSVFormat;
		uint32 mSampleCount;
		uint32 mSampleQuality;
		uint32 mSampleMask;

		uint32 mDescriptorStartIndex;

		// Resource Id를 통해 관리
		std::array<Framework::ResourceId, static_cast<size_t>(TextureType::Count)> mTextureIds;

		// 캐싱을 위한 해시 (지연 계산)
		mutable size_t mCachedHash = 0;
	};

} // namespace Graphics
