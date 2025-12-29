/**
 * @file AnimationAsset.h
 * @brief 애니메이션 데이터를 담는 Asset 클래스
 *
 * glTF 애니메이션 데이터를 보유합니다.
 *
 * @note Phase 6+에서 구현 예정, 현재는 구조만 정의
 */
#pragma once
#include "Framework/Assets/IAsset.h"
#include "Core/Types.h"
#include <string>

namespace Framework
{
	/**
	 * @brief 애니메이션 Asset 클래스
	 *
	 * glTF 애니메이션의 키프레임 데이터를 보유합니다.
	 *
	 * @note Phase 6+에서 구현 예정
	 *       - Keyframe 데이터
	 *       - Channel 정보 (어떤 노드의 어떤 속성)
	 *       - Sampling 메서드
	 */
	class AnimationAsset : public IAsset
	{
	public:
		AnimationAsset();
		~AnimationAsset() override = default;

		//=========================================================================
		// IAsset 인터페이스 구현
		//=========================================================================

		AssetType GetType() const override { return AssetType::Animation; }
		const std::string& GetPath() const override { return mPath; }
		AssetState GetState() const override { return mState; }
		Core::size_t GetMemoryUsage() const override;

		//=========================================================================
		// AnimationAsset 전용 API (Phase 6+에서 구현)
		//=========================================================================

		// TODO: Phase 6+ - 애니메이션 정보
		// const std::string& GetName() const;
		// Core::float32 GetDuration() const;
		// Core::uint32 GetChannelCount() const;

		// TODO: Phase 6+ - 샘플링
		// void Sample(Core::float32 time, AnimationPose& outPose) const;

	private:
		friend class AssetManager;

		std::string mPath;
		AssetState mState = AssetState::Unloaded;

		// TODO: Phase 6+ - 애니메이션 데이터
		// std::string mName;
		// Core::float32 mDuration = 0.0f;
		// std::vector<AnimationChannel> mChannels;
		// std::vector<AnimationSampler> mSamplers;
	};

} // namespace Framework
