/**
 * @file AnimationAsset.cpp
 * @brief AnimationAsset 클래스 구현
 *
 * @note Phase 6+에서 구현 예정
 */
#include "pch.h"
#include "Framework/Assets/AnimationAsset.h"

namespace Framework
{
	AnimationAsset::AnimationAsset()
		: mPath()
		, mState(AssetState::Unloaded)
	{
	}

	Core::size_t AnimationAsset::GetMemoryUsage() const
	{
		Core::size_t usage = sizeof(AnimationAsset);

		// 문자열 메모리
		usage += mPath.capacity();

		// TODO: Phase 6+ - 애니메이션 데이터 메모리
		// usage += mName.capacity();
		// usage += mChannels.capacity() * sizeof(AnimationChannel);
		// usage += mSamplers.capacity() * sizeof(AnimationSampler);

		return usage;
	}

} // namespace Framework
