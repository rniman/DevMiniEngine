/**
 * @file ResourceManager.h
 * @brief 중앙 집중식 GPU 리소스 관리자
 *
 * 64비트 해시 기반 ResourceId 사용
 * Asset과 GPU Resource를 연결하는 역할
 *
 * @note Phase 4.2: CreateMeshFromAsset 추가
 */
#pragma once
#include "Framework/Resources/ResourceId.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace Graphics
{
	class Mesh;
	class Material;
	class Texture;
	class DX12Device;
	class DX12Renderer;
}

namespace Framework
{
	// 전방 선언
	class MeshAsset;

	/**
	 * @brief 중앙 집중식 리소스 관리자
	 *
	 * 64비트 해시 기반 ResourceId 사용
	 * 모든 리소스의 유일한 소유자
	 */
	class ResourceManager
	{
	public:
		ResourceManager(Graphics::DX12Device* device, Graphics::DX12Renderer* renderer);
		~ResourceManager();

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
		 * @brief MeshAsset에서 GPU Mesh 생성
		 *
		 * @param name 메시 이름 (ResourceId 생성용)
		 * @param meshAsset CPU 측 메시 데이터
		 * @return ResourceId (실패 시 Invalid)
		 *
		 * @note 정점 수에 따라 16비트/32비트 인덱스 자동 선택
		 * @note ReleaseAfterUpload 정책이면 업로드 후 CPU 데이터 해제
		 */
		ResourceId CreateMeshFromAsset(const std::string& name, MeshAsset* meshAsset);

		/**
		 * @brief ResourceId로 Mesh 조회
		 * @param id 리소스 ID
		 * @return Mesh 포인터 (없으면 nullptr)
		 */
		Graphics::Mesh* GetMesh(ResourceId id);
		const Graphics::Mesh* GetMesh(ResourceId id) const;

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

		Graphics::Material* GetMaterial(ResourceId id);
		const Graphics::Material* GetMaterial(ResourceId id) const;
		bool RemoveMaterial(ResourceId id);

		//=====================================================================
		// Texture 관리
		//=====================================================================

		/**
		 * @brief 파일 경로로 Texture 로드 (해시 ID 반환)
		 * @param path 텍스처 파일 UTF-8 경로 (해시되어 ID 생성)
		 * @return 64비트 해시 기반 ResourceId
		 */
		ResourceId LoadTexture(const std::string& path);
		ResourceId LoadTextureW(const std::wstring& path);

		Graphics::Texture* GetTexture(ResourceId id);
		const Graphics::Texture* GetTexture(ResourceId id) const;
		bool RemoveTexture(ResourceId id);

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
		Graphics::DX12Device* mDevice;
		Graphics::DX12Renderer* mRenderer;

		// Mesh
		std::unordered_map<ResourceId, std::shared_ptr<Graphics::Mesh>> mMeshes;
		std::unordered_map<ResourceId, std::string> mMeshNames;

		// Material
		std::unordered_map<ResourceId, std::shared_ptr<Graphics::Material>> mMaterials;
		std::unordered_map<ResourceId, std::string> mMaterialNames;

		// Texture
		std::unordered_map<ResourceId, std::shared_ptr<Graphics::Texture>> mTextures;
		std::unordered_map<ResourceId, std::string> mTexturePaths;
	};

} // namespace Framework
