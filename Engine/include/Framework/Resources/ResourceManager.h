/**
 * @file ResourceManager.h
 * @brief 중앙 집중식 GPU 리소스 관리자
 *
 * 64비트 해시 기반 ResourceId 사용
 * Asset과 GPU Resource를 연결하는 역할
 *
 * @note Phase 4.2: CreateMeshFromAsset, 폴백 텍스처 추가
 * @note Phase 4.2+: 임베디드 텍스처 로드 지원
 * @note Phase 4.3: sRGB/Linear 색공간 처리, AssetManager 연동
 */
#pragma once
#include "Framework/Resources/ResourceId.h"
#include "Graphics/TextureType.h"
#include "Core/Types.h"
#include <dxgiformat.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace Graphics
{
	class MeshResource;
	class MaterialResource;
	class TextureResource;
	class DX12Device;
	class DX12Renderer;
}

namespace Framework
{
	// 전방 선언
	class MeshAsset;
	class TextureAsset;
	class AssetManager;

	/**
	 * @brief 중앙 집중식 리소스 관리자
	 *
	 * 64비트 해시 기반 ResourceId 사용
	 * 모든 GPU 리소스의 유일한 소유자
	 *
	 * @note TextureAsset 메타데이터는 AssetManager가 소유
	 */
	class ResourceManager
	{
	public:
		ResourceManager(Graphics::DX12Device* device, Graphics::DX12Renderer* renderer);
		~ResourceManager();

		//=====================================================================
		// AssetManager 연결
		//=====================================================================

		/**
		 * @brief AssetManager 설정
		 *
		 * ResourceManager 생성 후, AssetManager 생성 후 호출합니다.
		 * TextureAsset 메타데이터 등록에 사용됩니다.
		 *
		 * @param assetManager AssetManager 포인터
		 */
		void SetAssetManager(AssetManager* assetManager) { mAssetManager = assetManager; }

		/**
		 * @brief AssetManager 접근자
		 */
		AssetManager* GetAssetManager() const { return mAssetManager; }

		//=====================================================================
		// Mesh 관리
		//=====================================================================

		/**
		 * @brief 이름으로 빈 Mesh 생성 (해시 ID 반환)
		 * @param name 메시 이름 (내부적으로 해시됨)
		 * @return 64비트 해시 기반 ResourceId
		 */
		ResourceId CreateMesh(const std::string& name);

		/**
		 * @brief MeshAsset에서 GPU MeshResource 생성
		 *
		 * MeshAsset의 소유권이 AssetManager로 이전됩니다.
		 *
		 * @param name 리소스 이름
		 * @param asset MeshAsset (소유권 이전)
		 * @return 생성된 MeshResource의 ID
		 */
		ResourceId CreateMeshFromAsset(
			const std::string& name,
			std::unique_ptr<MeshAsset> asset
		);

		/**
		 * @brief ResourceId로 Mesh 조회
		 * @param id 리소스 ID
		 * @return Mesh 포인터 (없으면 nullptr)
		 */
		Graphics::MeshResource* GetMesh(ResourceId id);
		const Graphics::MeshResource* GetMesh(ResourceId id) const;

		/**
		 * @brief ResourceId로 Mesh 제거
		 */
		bool RemoveMesh(ResourceId id);

		//=====================================================================
		// Material 관리
		//=====================================================================

		ResourceId CreateMaterial(
			const std::string& name,
			const std::wstring& vertexShader,
			const std::wstring& pixelShader
		);

		Graphics::MaterialResource* GetMaterial(ResourceId id);
		const Graphics::MaterialResource* GetMaterial(ResourceId id) const;
		bool RemoveMaterial(ResourceId id);

		//=====================================================================
		// Texture 관리
		//=====================================================================

		/**
		 * @brief 파일 경로로 Texture 로드 (색공간 자동 적용)
		 *
		 * TextureType에 따라 sRGB/Linear 색공간을 자동으로 선택합니다.
		 * - Albedo, Emissive: sRGB
		 * - Normal, Roughness 등: Linear
		 *
		 * @param path 텍스처 파일 UTF-8 경로
		 * @param textureType 텍스처 용도 (색공간 결정)
		 * @return 64비트 해시 기반 ResourceId
		 *
		 * @note TextureAsset 메타데이터가 AssetManager에 자동 등록됨
		 */
		ResourceId LoadTexture(const std::string& path, Graphics::TextureType textureType);

		/**
		 * @brief 와이드 문자열 경로로 Texture 로드 (색공간 자동 적용)
		 */
		ResourceId LoadTextureW(const std::wstring& path, Graphics::TextureType textureType);

		/**
		 * @brief 메모리에서 Texture 로드 (색공간 자동 적용)
		 *
		 * glb 파일의 임베디드 텍스처 로딩에 사용됩니다.
		 *
		 * @param name 텍스처 이름 (ResourceId 생성용, 예: "Model_*0")
		 * @param data 압축된 이미지 데이터 (PNG, JPG 바이너리)
		 * @param dataSize 데이터 크기 (바이트)
		 * @param textureType 텍스처 용도 (색공간 결정)
		 * @return 64비트 해시 기반 ResourceId (실패 시 Invalid)
		 *
		 * @note TextureAsset 메타데이터가 AssetManager에 자동 등록됨
		 */
		ResourceId LoadTextureFromMemory(
			const std::string& name,
			const void* data,
			Core::uint32 dataSize,
			Graphics::TextureType textureType
		);

		/**
		 * @brief 원시 픽셀 데이터에서 Texture 생성 (색공간 자동 적용)
		 *
		 * TextureType에 따라 sRGB/Linear 포맷을 자동으로 선택합니다.
		 * - Albedo, Emissive: DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
		 * - Normal, Roughness 등: DXGI_FORMAT_R8G8B8A8_UNORM
		 *
		 * @param name 텍스처 이름 (ResourceId 생성용)
		 * @param data RGBA 픽셀 데이터
		 * @param width 텍스처 너비
		 * @param height 텍스처 높이
		 * @param textureType 텍스처 용도 (색공간 결정)
		 * @return 64비트 해시 기반 ResourceId (실패 시 Invalid)
		 *
		 * @note TextureAsset 메타데이터가 AssetManager에 자동 등록됨
		 */
		ResourceId CreateTextureFromMemory(
			const std::string& name,
			const void* data,
			Core::uint32 width,
			Core::uint32 height,
			Graphics::TextureType textureType
		);

		Graphics::TextureResource* GetTexture(ResourceId id);
		const Graphics::TextureResource* GetTexture(ResourceId id) const;
		bool RemoveTexture(ResourceId id);

		/**
		 * @brief 폴백 텍스처 ID 반환
		 *
		 * 텍스처 로드 실패 시 사용할 1x1 Magenta 텍스처입니다.
		 * 누락된 텍스처를 시각적으로 식별하는 데 유용합니다.
		 *
		 * @return 폴백 텍스처의 ResourceId
		 * @note 생성자에서 자동으로 생성됩니다
		 */
		ResourceId GetFallbackTexture() const { return mFallbackTextureId; }

		/**
		 * @brief 폴백 텍스처 존재 여부 확인
		 */
		bool HasFallbackTexture() const { return mFallbackTextureId.IsValid(); }

		//=====================================================================
		// TextureAsset 접근 (AssetManager 위임)
		//=====================================================================

		/**
		 * @brief ResourceId로 TextureAsset 조회
		 *
		 * 텍스처의 메타데이터(크기, 포맷, 색공간 등)에 접근할 때 사용합니다.
		 * 내부적으로 AssetManager에 위임합니다.
		 *
		 * @param id 리소스 ID
		 * @return TextureAsset 포인터 (없으면 nullptr)
		 */
		TextureAsset* GetTextureAsset(ResourceId id);
		const TextureAsset* GetTextureAsset(ResourceId id) const;

		//=====================================================================
		// 검색 & 유틸리티
		//=====================================================================

		ResourceId FindMeshByName(const std::string& name) const;
		ResourceId FindMaterialByName(const std::string& name) const;
		ResourceId FindTextureByPath(const std::string& path) const;

		void Clear();

		//=====================================================================
		// 디바이스 접근 (내부용)
		//=====================================================================

		Graphics::DX12Device* GetDevice() const { return mDevice; }
		Graphics::DX12Renderer* GetRenderer() const { return mRenderer; }

	private:
		/**
		 * @brief 폴백 텍스처 생성 (1x1 Magenta)
		 *
		 * 생성자에서 호출되어 텍스처 로드 실패 시 사용할
		 * 기본 텍스처를 미리 생성합니다.
		 */
		void CreateFallbackTexture();

		/**
		 * @brief AssetManager에 TextureAsset 등록 (내부 헬퍼)
		 *
		 * @param id 리소스 ID
		 * @param path 파일 경로 또는 이름
		 * @param textureType 텍스처 용도
		 * @param texture GPU 텍스처 (메타데이터 소스)
		 */
		void RegisterTextureAssetToManager(
			ResourceId id,
			const std::string& path,
			Graphics::TextureType textureType,
			const Graphics::TextureResource* texture
		);

		Graphics::DX12Device* mDevice;
		Graphics::DX12Renderer* mRenderer;
		AssetManager* mAssetManager = nullptr;

		// Mesh
		std::unordered_map<ResourceId, std::shared_ptr<Graphics::MeshResource>> mMeshes;
		std::unordered_map<ResourceId, std::string> mMeshNames;

		// Material
		std::unordered_map<ResourceId, std::shared_ptr<Graphics::MaterialResource>> mMaterials;
		std::unordered_map<ResourceId, std::string> mMaterialNames;

		// Texture (GPU Resource만, Asset은 AssetManager 소유)
		std::unordered_map<ResourceId, std::shared_ptr<Graphics::TextureResource>> mTextures;
		std::unordered_map<ResourceId, std::string> mTexturePaths;

		// 폴백 텍스처 (1x1 Magenta)
		ResourceId mFallbackTextureId;
	};

} // namespace Framework
