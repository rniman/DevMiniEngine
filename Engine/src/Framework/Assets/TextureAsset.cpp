/**
 * @file TextureAsset.cpp
 * @brief TextureAsset 클래스 구현
 */
#include "pch.h"
#include "Framework/Assets/TextureAsset.h"

namespace Framework
{
	TextureAsset::TextureAsset()
		: mPath()
		, mState(AssetState::Unloaded)
	{
	}

	Core::size_t TextureAsset::GetMemoryUsage() const
	{
		Core::size_t usage = sizeof(TextureAsset);

		// 문자열 메모리
		usage += mPath.capacity();

		// TODO: Phase 4.3 - 픽셀 데이터 메모리
		// usage += mPixelData.capacity();

		return usage;
	}

} // namespace Framework
