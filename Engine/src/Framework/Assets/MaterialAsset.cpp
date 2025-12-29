/**
 * @file MaterialAsset.cpp
 * @brief MaterialAsset 클래스 구현
 */
#include "pch.h"
#include "Framework/Assets/MaterialAsset.h"

namespace Framework
{
	MaterialAsset::MaterialAsset()
		: mPath()
		, mState(AssetState::Unloaded)
	{
	}

	Core::size_t MaterialAsset::GetMemoryUsage() const
	{
		Core::size_t usage = sizeof(MaterialAsset);

		// 문자열 메모리
		usage += mPath.capacity();

		// TODO: Phase 4.3 - 텍스처 경로 메모리
		// for (const auto& texPath : mTexturePaths)
		// {
		//     usage += texPath.capacity();
		// }

		return usage;
	}

} // namespace Framework
