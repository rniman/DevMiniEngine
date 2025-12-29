/**
 * @file TextureAsset.h
 * @brief 텍스처 데이터를 담는 Asset 클래스
 *
 * 파일에서 로드된 픽셀 데이터를 보유합니다.
 * ResourceManager를 통해 GPU Texture로 변환됩니다.
 */
#pragma once
#include "Framework/Assets/IAsset.h"
#include "Core/Types.h"
#include <dxgiformat.h>
#include <string>
#include <vector>

namespace Framework
{
	/**
	 * @brief 텍스처 Asset 클래스
	 *
	 * DDS, PNG 등에서 로드된 텍스처의 CPU 측 데이터를 보유합니다.
	 * 픽셀 데이터, 크기, 포맷, 밉맵 정보를 포함합니다.
	 *
	 * @note DXGI_FORMAT을 직접 사용하여 모든 DirectX 포맷 지원
	 * @note Phase 4.1: 구조만 정의, 실제 데이터 로딩은 Phase 4.3
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
		// TextureAsset 전용 API (Phase 4.3에서 구현)
		//=========================================================================

		// TODO: Phase 4.3 - 텍스처 정보
		// Core::uint32 GetWidth() const;
		// Core::uint32 GetHeight() const;
		// Core::uint32 GetMipLevels() const;
		// DXGI_FORMAT GetFormat() const;
		// bool IsSRGB() const;
		// bool IsCubeMap() const;

		// TODO: Phase 4.3 - 픽셀 데이터 접근
		// const Core::uint8* GetPixelData() const;
		// Core::size_t GetPixelDataSize() const;
		// const Core::uint8* GetMipData(Core::uint32 mipLevel) const;

	private:
		friend class AssetManager;

		std::string mPath;
		AssetState mState = AssetState::Unloaded;

		// TODO: Phase 4.3 - 텍스처 메타데이터
		// Core::uint32 mWidth = 0;
		// Core::uint32 mHeight = 0;
		// Core::uint32 mDepth = 1;           // 3D 텍스처용
		// Core::uint32 mMipLevels = 1;
		// Core::uint32 mArraySize = 1;       // 텍스처 배열용
		// DXGI_FORMAT mFormat = DXGI_FORMAT_UNKNOWN;
		// bool mIsCubeMap = false;

		// TODO: Phase 4.3 - 픽셀 데이터
		// std::vector<Core::uint8> mPixelData;
	};

} // namespace Framework
