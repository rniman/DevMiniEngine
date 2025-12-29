/**
 * @file MaterialAsset.h
 * @brief 머티리얼 데이터를 담는 Asset 클래스
 *
 * 파일에서 로드된 머티리얼 파라미터를 보유합니다.
 * ResourceManager를 통해 GPU Material로 변환됩니다.
 */
#pragma once
#include "Framework/Assets/IAsset.h"
#include "Core/Types.h"
#include <array>
#include <string>

namespace Framework
{
	/**
	 * @brief 머티리얼 Asset 클래스
	 *
	 * glTF의 PBR 머티리얼 또는 커스텀 머티리얼 정의를 보유합니다.
	 * 베이스 컬러, 메탈릭, 러프니스 등의 파라미터와 텍스처 참조를 포함합니다.
	 *
	 * @note 텍스처 슬롯은 기존 TextureFlags와 일치
	 * @note glTF의 MetallicRoughness 텍스처는 Importer에서 분리 처리
	 * @note Phase 4.1: 구조만 정의, 실제 데이터 로딩은 Phase 4.3
	 */
	class MaterialAsset : public IAsset
	{
	public:
		/**
		 * @brief PBR 텍스처 슬롯 인덱스
		 *
		 * 기존 TextureFlags 열거형과 순서 일치:
		 * - Albedo (1 << 0)
		 * - Normal (1 << 1)
		 * - Metallic (1 << 2)
		 * - Roughness (1 << 3)
		 * - Occlusion (1 << 4)
		 * - Emissive (1 << 5)
		 */
		enum class TextureSlot : Core::uint8
		{
			Albedo = 0,		// 기본 색상 (sRGB)
			Normal,			// 노말 맵 (Linear)
			Metallic,		// 금속성 (단일 채널)
			Roughness,		// 거칠기 (단일 채널)
			Occlusion,		// 앰비언트 오클루전 (단일 채널)
			Emissive,		// 발광 (sRGB)

			Count
		};

		MaterialAsset();
		~MaterialAsset() override = default;

		//=========================================================================
		// IAsset 인터페이스 구현
		//=========================================================================

		AssetType GetType() const override { return AssetType::Material; }
		const std::string& GetPath() const override { return mPath; }
		AssetState GetState() const override { return mState; }
		Core::size_t GetMemoryUsage() const override;

		//=========================================================================
		// MaterialAsset 전용 API (Phase 4.3에서 구현)
		//=========================================================================

		// TODO: Phase 4.3 - PBR 파라미터
		// const Math::Vector4& GetBaseColor() const;
		// Core::float32 GetMetallic() const;
		// Core::float32 GetRoughness() const;
		// const Math::Vector3& GetEmissiveFactor() const;
		// Core::float32 GetAlphaCutoff() const;
		// bool IsDoubleSided() const;

		// TODO: Phase 4.3 - 텍스처 참조 (Asset 경로)
		// const std::string& GetTexturePath(TextureSlot slot) const;
		// bool HasTexture(TextureSlot slot) const;

	private:
		friend class AssetManager;

		std::string mPath;
		AssetState mState = AssetState::Unloaded;

		// TODO: Phase 4.3 - PBR 파라미터
		// Math::Vector4 mBaseColor = Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		// Core::float32 mMetallic = 0.0f;
		// Core::float32 mRoughness = 0.5f;
		// Math::Vector3 mEmissiveFactor = Math::Vector3(0.0f, 0.0f, 0.0f);
		// Core::float32 mAlphaCutoff = 0.5f;
		// bool mDoubleSided = false;

		// TODO: Phase 4.3 - 텍스처 경로 (ModelAsset 내 상대 참조)
		// std::array<std::string, static_cast<size_t>(TextureSlot::Count)> mTexturePaths;
	};

} // namespace Framework
