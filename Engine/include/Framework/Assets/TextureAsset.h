/**
 * @file TextureAsset.h
 * @brief 텍스처 데이터를 담는 Asset 클래스
 *
 * 파일에서 로드된 텍스처의 메타데이터를 보유합니다.
 * ResourceManager를 통해 GPU TextureResource로 변환됩니다.
 *
 * @note Phase 4.3: 메타데이터 구조 구현
 */
#pragma once
#include "Framework/Assets/IAsset.h"
#include "Graphics/TextureType.h"
#include "Core/Types.h"
#include <dxgiformat.h>
#include <string>

namespace Framework
{
	/**
	 * @brief 텍스처 Asset 클래스
	 *
	 * DDS, PNG 등에서 로드된 텍스처의 메타데이터를 보유합니다.
	 * 현재는 메타데이터만 저장하며, 실제 픽셀 데이터는 TextureResource가 관리합니다.
	 *
	 * @note DXGI_FORMAT을 직접 사용하여 모든 DirectX 포맷 지원
	 * @note Phase 4.3: 메타데이터 구조 (DirectXTex 도입 시 픽셀 데이터 추가 예정)
	 */
	class TextureAsset : public IAsset
	{
	public:
		TextureAsset();
		~TextureAsset() override = default;

		//=========================================================================
		// IAsset 인터페이스 구현
		//=========================================================================

		AssetType GetType() const override { return AssetType::Texture; }
		const std::string& GetPath() const override { return mPath; }
		AssetState GetState() const override { return mState; }
		Core::size_t GetMemoryUsage() const override;

		//=========================================================================
		// 메타데이터 Getters
		//=========================================================================

		/** @brief 텍스처 너비 (픽셀) */
		Core::uint32 GetWidth() const { return mWidth; }

		/** @brief 텍스처 높이 (픽셀) */
		Core::uint32 GetHeight() const { return mHeight; }

		/** @brief DXGI 포맷 */
		DXGI_FORMAT GetFormat() const { return mFormat; }

		/** @brief 텍스처 용도 (Albedo, Normal 등) */
		Graphics::TextureType GetTextureType() const { return mTextureType; }

		/** @brief sRGB 색공간 여부 */
		bool IsSRGB() const { return mIsSRGB; }

		//=========================================================================
		// 메타데이터 설정 (ResourceManager 전용)
		//=========================================================================

		/**
		 * @brief GPU 로드 후 메타데이터 설정
		 *
		 * ResourceManager가 TextureResource 생성 후 호출합니다.
		 *
		 * @param width 텍스처 너비
		 * @param height 텍스처 높이
		 * @param format DXGI 포맷
		 * @param isSRGB sRGB 색공간 여부
		 */
		void SetMetadata(
			Core::uint32 width,
			Core::uint32 height,
			DXGI_FORMAT format,
			bool isSRGB
		);

		/**
		 * @brief 텍스처 타입 설정
		 *
		 * AssetManager 또는 ResourceManager가 로딩 요청 시 호출합니다.
		 *
		 * @param type 텍스처 용도 (Albedo, Normal 등)
		 */
		void SetTextureType(Graphics::TextureType type) { mTextureType = type; }

	private:
		friend class AssetManager;
		friend class ResourceManager;

		//=========================================================================
		// 경로 및 상태
		//=========================================================================

		std::string mPath;
		AssetState mState = AssetState::Unloaded;

		//=========================================================================
		// 텍스처 메타데이터
		//=========================================================================

		Graphics::TextureType mTextureType = Graphics::TextureType::Albedo;
		Core::uint32 mWidth = 0;       
		Core::uint32 mHeight = 0;               
		DXGI_FORMAT mFormat = DXGI_FORMAT_UNKNOWN;
		bool mIsSRGB = false;                                                

		// TODO: Phase 4.3+ (DirectXTex 도입 시)
		// Core::uint32 mMipLevels = 1;
		// std::vector<Core::uint8> mPixelData;
	};

} // namespace Framework
