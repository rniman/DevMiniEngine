#pragma once

#include "Core/Types.h"
#include "Framework/Resources/ResourceId.h"
#include <memory>
#include <unordered_map>
#include <string>

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
		 * @brief 이름으로 Mesh 생성 (해시 ID 반환)
		 * @param name 메시 이름 (내부적으로 해시됨)
		 * @return 64비트 해시 기반 ResourceId
		 */
		ResourceId CreateMesh(const std::string& name);

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
		// 디버깅 & 편의 함수
		//=====================================================================

		// 이름으로 ID 조회 (디버깅용)
		ResourceId FindMeshByName(const std::string& name) const;
		ResourceId FindMaterialByName(const std::string& name) const;
		ResourceId FindTextureByPath(const std::string& path) const;

		// 모든 리소스 제거
		void Clear();

	private:
		Graphics::DX12Device* mDevice;
		Graphics::DX12Renderer* mRenderer;

		// ResourceId - 리소스 맵 (유일한 소유자)
		std::unordered_map<ResourceId, std::shared_ptr<Graphics::Mesh>> mMeshes;
		std::unordered_map<ResourceId, std::shared_ptr<Graphics::Material>> mMaterials;
		std::unordered_map<ResourceId, std::shared_ptr<Graphics::Texture>> mTextures;

		// 역참조 맵 (디버깅용)
		std::unordered_map<ResourceId, std::string> mMeshNames;
		std::unordered_map<ResourceId, std::string> mMaterialNames;
		std::unordered_map<ResourceId, std::string> mTexturePaths;
	};

} // namespace Framework
