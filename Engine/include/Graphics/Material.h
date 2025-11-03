#pragma once

#include "Graphics/GraphicsTypes.h"
#include <string>

namespace Graphics
{

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
		Core::uint32 numRenderTargets = 1;
		DXGI_FORMAT rtvFormats[8] = { DXGI_FORMAT_R8G8B8A8_UNORM };  // 최대 8개
		DXGI_FORMAT dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		// MSAA 설정
		Core::uint32 sampleCount = 1;
		Core::uint32 sampleQuality = 0;
		Core::uint32 sampleMask = 0xFFFFFFFF;
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

		// 복사 허용 (설정 정보만 복사)
		Material(const Material&) = default;
		Material& operator=(const Material&) = default;

		// 이동 허용
		Material(Material&&) noexcept = default;
		Material& operator=(Material&&) noexcept = default;

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
		Core::uint32 GetNumRenderTargets() const { return mNumRenderTargets; }
		const DXGI_FORMAT* GetRTVFormats() const { return mRTVFormats; }
		DXGI_FORMAT GetDSVFormat() const { return mDSVFormat; }
		Core::uint32 GetSampleCount() const { return mSampleCount; }
		Core::uint32 GetSampleQuality() const { return mSampleQuality; }
		Core::uint32 GetSampleMask() const { return mSampleMask; }

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
		static D3D12_DEPTH_STENCIL_DESC CreateDepthStencilDesc(bool depthTest, bool depthWrite, D3D12_COMPARISON_FUNC depthComparisonFunc);

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
		Core::uint32 mNumRenderTargets;
		DXGI_FORMAT mRTVFormats[8];  // D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT
		DXGI_FORMAT mDSVFormat;
		Core::uint32 mSampleCount;
		Core::uint32 mSampleQuality;
		Core::uint32 mSampleMask;

		// 캐싱을 위한 해시 (지연 계산)
		mutable size_t mCachedHash = 0;
	};

} // namespace Graphics