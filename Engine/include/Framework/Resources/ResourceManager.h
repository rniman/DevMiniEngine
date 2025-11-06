// Engine/include/Framework/Resources/ResourceManager.h
#pragma once

#include "Core/Types.h"
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
	 * @brief 리소스 관리자 클래스
	 *
	 * 메시, 머티리얼, 텍스처 등의 리소스를 중앙에서 관리합니다.
	 */
	class ResourceManager
	{
	public:
		ResourceManager(Graphics::DX12Device* device, Graphics::DX12Renderer* renderer);
		~ResourceManager();

		// 메시 관리
		std::shared_ptr<Graphics::Mesh> CreateMesh(const std::string& name);
		std::shared_ptr<Graphics::Mesh> GetMesh(const std::string& name) const;
		bool RemoveMesh(const std::string& name);

		// 머티리얼 관리
		std::shared_ptr<Graphics::Material> CreateMaterial(
			const std::string& name,
			const std::wstring& vertexShader,
			const std::wstring& pixelShader
		);
		std::shared_ptr<Graphics::Material> GetMaterial(const std::string& name) const;
		bool RemoveMaterial(const std::string& name);

		// 텍스처 관리
		std::shared_ptr<Graphics::Texture> LoadTexture(const std::wstring& path);
		std::shared_ptr<Graphics::Texture> GetTexture(const std::wstring& path) const;
		bool RemoveTexture(const std::wstring& path);

		// 모든 리소스 제거
		void Clear();

	private:
		Graphics::DX12Device* mDevice;
		Graphics::DX12Renderer* mRenderer;

		std::unordered_map<std::string, std::shared_ptr<Graphics::Mesh>> mMeshes;
		std::unordered_map<std::string, std::shared_ptr<Graphics::Material>> mMaterials;
		std::unordered_map<std::wstring, std::shared_ptr<Graphics::Texture>> mTextures;
	};

} // namespace Framework
