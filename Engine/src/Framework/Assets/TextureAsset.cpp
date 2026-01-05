/**
 * @file TextureAsset.cpp
 * @brief TextureAsset 클래스 구현
 *
 * @note Phase 4.3: 메타데이터 구조 구현
 */
#include "pch.h"
#include "Framework/Assets/TextureAsset.h"

namespace Framework
{
	TextureAsset::TextureAsset()
		: mPath()
		, mState(AssetState::Unloaded)
		, mTextureType(Graphics::TextureType::Albedo)
		, mWidth(0)
		, mHeight(0)
		, mFormat(DXGI_FORMAT_UNKNOWN)
		, mIsSRGB(false)
	{
	}

	Core::size_t TextureAsset::GetMemoryUsage() const
	{
		Core::size_t usage = sizeof(TextureAsset);

		// 문자열 메모리
		usage += mPath.capacity();

		// TODO: Phase 4.3+ (DirectXTex 도입 시)
		// usage += mPixelData.capacity();

		return usage;
	}

	void TextureAsset::SetMetadata(
		Core::uint32 width,
		Core::uint32 height,
		DXGI_FORMAT format,
		bool isSRGB
	)
	{
		mWidth = width;
		mHeight = height;
		mFormat = format;
		mIsSRGB = isSRGB;
	}

} // namespace Framework
