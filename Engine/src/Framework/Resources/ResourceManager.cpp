#include "pch.h"
#include "Framework/Resources/ResourceManager.h"
#include "Graphics/Mesh.h"
#include "Graphics/Material.h"
#include "Graphics/Texture.h"
#include "Graphics/DX12/DX12Device.h"
#include "Graphics/DX12/DX12Renderer.h"

namespace Framework
{
	ResourceManager::ResourceManager(Graphics::DX12Device* device, Graphics::DX12Renderer* renderer)
		: mDevice(device)
		, mRenderer(renderer)
	{
		LOG_INFO("ResourceManager initialized");
	}

	ResourceManager::~ResourceManager()
	{
		Clear();
		LOG_INFO("ResourceManager destroyed");
	}

	std::shared_ptr<Graphics::Mesh> ResourceManager::CreateMesh(const std::string& name)
	{
		// 이미 존재하는지 확인
		auto it = mMeshes.find(name);
		if (it != mMeshes.end())
		{
			LOG_WARN("Mesh '%s' already exists", name.c_str());
			return it->second;
		}

		// 새 메시 생성
		auto mesh = std::make_shared<Graphics::Mesh>();
		mMeshes[name] = mesh;

		LOG_DEBUG("Created mesh: %s", name.c_str());
		return mesh;
	}

	std::shared_ptr<Graphics::Mesh> ResourceManager::GetMesh(const std::string& name) const
	{
		auto it = mMeshes.find(name);
		if (it != mMeshes.end())
		{
			return it->second;
		}
		return nullptr;
	}

	bool ResourceManager::RemoveMesh(const std::string& name)
	{
		auto it = mMeshes.find(name);
		if (it != mMeshes.end())
		{
			it->second->Shutdown();
			mMeshes.erase(it);
			LOG_DEBUG("Removed mesh: %s", name.c_str());
			return true;
		}
		return false;
	}

	std::shared_ptr<Graphics::Material> ResourceManager::CreateMaterial(
		const std::string& name,
		const std::wstring& vertexShader,
		const std::wstring& pixelShader)
	{
		// 이미 존재하는지 확인
		auto it = mMaterials.find(name);
		if (it != mMaterials.end())
		{
			LOG_WARN("Material '%s' already exists", name.c_str());
			return it->second;
		}

		// 새 머티리얼 생성
		Graphics::MaterialDesc desc;
		desc.vertexShaderPath = vertexShader.c_str();
		desc.pixelShaderPath = pixelShader.c_str();

		auto material = std::make_shared<Graphics::Material>(desc);
		mMaterials[name] = material;

		LOG_DEBUG("Created material: %s", name.c_str());
		return material;
	}

	std::shared_ptr<Graphics::Material> ResourceManager::GetMaterial(const std::string& name) const
	{
		auto it = mMaterials.find(name);
		if (it != mMaterials.end())
		{
			return it->second;
		}
		return nullptr;
	}

	bool ResourceManager::RemoveMaterial(const std::string& name)
	{
		auto it = mMaterials.find(name);
		if (it != mMaterials.end())
		{
			mMaterials.erase(it);
			LOG_DEBUG("Removed material: %s", name.c_str());
			return true;
		}
		return false;
	}

	std::shared_ptr<Graphics::Texture> ResourceManager::LoadTexture(const std::wstring& path)
	{
		// 이미 로드된 텍스처인지 확인
		auto it = mTextures.find(path);
		if (it != mTextures.end())
		{
			return it->second;
		}

		// 새 텍스처 로드
		auto texture = std::make_shared<Graphics::Texture>();

		// 현재 프레임 인덱스 가져오기
		Core::uint32 frameIndex = mRenderer->GetCurrentFrameIndex();

		if (!texture->LoadFromFile(
			mDevice->GetDevice(),
			mDevice->GetGraphicsQueue(),
			mDevice->GetCommandContext(frameIndex),
			path.c_str()))
		{
			LOG_ERROR("Failed to load texture: %ls", path.c_str());
			return nullptr;
		}

		mTextures[path] = texture;
		LOG_DEBUG("Loaded texture: %ls", path.c_str());
		return texture;
	}

	std::shared_ptr<Graphics::Texture> ResourceManager::GetTexture(const std::wstring& path) const
	{
		auto it = mTextures.find(path);
		if (it != mTextures.end())
		{
			return it->second;
		}
		return nullptr;
	}

	bool ResourceManager::RemoveTexture(const std::wstring& path)
	{
		auto it = mTextures.find(path);
		if (it != mTextures.end())
		{
			mTextures.erase(it);
			LOG_DEBUG("Removed texture: %ls", path.c_str());
			return true;
		}
		return false;
	}

	void ResourceManager::Clear()
	{
		LOG_INFO("Clearing all resources...");

		// 메시 정리
		for (auto& [name, mesh] : mMeshes)
		{
			mesh->Shutdown();
		}
		mMeshes.clear();

		// 머티리얼과 텍스처는 자동으로 정리됨 (shared_ptr)
		mMaterials.clear();
		mTextures.clear();

		LOG_INFO("All resources cleared");
	}

} // namespace Framework
