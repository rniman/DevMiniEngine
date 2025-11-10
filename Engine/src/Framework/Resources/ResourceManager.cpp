#include "pch.h"
#include "Framework/Resources/ResourceManager.h"
#include "Core/Logging/LogMacros.h"
#include "Core/Hash.h"
#include "Core/Types.h"
#include "Graphics/DX12/DX12Device.h"
#include "Graphics/DX12/DX12Renderer.h"
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"
#include "Graphics/Texture.h"


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

	//=========================================================================
	// Mesh 관리
	//=========================================================================

	ResourceId ResourceManager::CreateMesh(const std::string& name)
	{
		// 이름을 64비트 해시로 변환
		ResourceId id;
		id.id = Core::Hash64(name);

		// 이미 존재하는지 확인
		auto it = mMeshes.find(id);
		if (it != mMeshes.end())
		{
			LOG_WARN("Mesh '%s' already exists (ID: 0x%llX)", name.c_str(), id.id);
			return id;
		}

		// 새 메시 생성
		auto mesh = std::make_shared<Graphics::Mesh>();
		mMeshes[id] = mesh;
		mMeshNames[id] = name;  // 디버깅용 역참조

		LOG_DEBUG("Created mesh: %s (ID: 0x%llX)", name.c_str(), id.id);
		return id;
	}

	Graphics::Mesh* ResourceManager::GetMesh(ResourceId id)
	{
		auto it = mMeshes.find(id);
		if (it != mMeshes.end())
		{
			return it->second.get();
		}

		LOG_WARN("Mesh not found: ID 0x%llX", id.id);
		return nullptr;
	}

	const Graphics::Mesh* ResourceManager::GetMesh(ResourceId id) const
	{
		auto it = mMeshes.find(id);
		if (it != mMeshes.end())
		{
			return it->second.get();
		}
		return nullptr;
	}

	bool ResourceManager::RemoveMesh(ResourceId id)
	{
		auto it = mMeshes.find(id);
		if (it != mMeshes.end())
		{
			it->second->Shutdown();
			mMeshes.erase(it);
			mMeshNames.erase(id);

			LOG_DEBUG("Removed mesh: ID 0x%llX", id.id);
			return true;
		}
		return false;
	}

	//=========================================================================
	// Material 관리
	//=========================================================================

	ResourceId ResourceManager::CreateMaterial(
		const std::string& name,
		const std::wstring& vertexShader,
		const std::wstring& pixelShader
	)
	{
		// 이름을 64비트 해시로 변환
		ResourceId id;
		id.id = Core::Hash64(name);

		// 이미 존재하는지 확인
		auto it = mMaterials.find(id);
		if (it != mMaterials.end())
		{
			LOG_WARN("Material '%s' already exists (ID: 0x%llX)", name.c_str(), id.id);
			return id;
		}

		// 새 머티리얼 생성
		Graphics::MaterialDesc desc;
		desc.vertexShaderPath = vertexShader.c_str();
		desc.pixelShaderPath = pixelShader.c_str();

		auto material = std::make_shared<Graphics::Material>(desc);
		mMaterials[id] = material;
		mMaterialNames[id] = name;

		LOG_DEBUG("Created material: %s (ID: 0x%llX)", name.c_str(), id.id);
		return id;
	}

	Graphics::Material* ResourceManager::GetMaterial(ResourceId id)
	{
		auto it = mMaterials.find(id);
		if (it != mMaterials.end())
		{
			return it->second.get();
		}

		LOG_WARN("Material not found: ID 0x%llX", id.id);
		return nullptr;
	}

	const Graphics::Material* ResourceManager::GetMaterial(ResourceId id) const
	{
		auto it = mMaterials.find(id);
		if (it != mMaterials.end())
		{
			return it->second.get();
		}
		return nullptr;
	}

	bool ResourceManager::RemoveMaterial(ResourceId id)
	{
		auto it = mMaterials.find(id);
		if (it != mMaterials.end())
		{
			mMaterials.erase(it);
			mMaterialNames.erase(id);

			LOG_DEBUG("Removed material: ID 0x%llX", id.id);
			return true;
		}
		return false;
	}

	//=========================================================================
	// Texture 관리
	//=========================================================================

	ResourceId ResourceManager::LoadTexture(const std::string& path)
	{
		// UTF-8 경로를 해시 (일관성 보장!)
		ResourceId id;
		id.id = Core::Hash64(path);

		// 이미 로드됨?
		auto it = mTextures.find(id);
		if (it != mTextures.end())
		{
			LOG_DEBUG("Texture already loaded: %s (ID: 0x%llX)", path.c_str(), id.id);
			return id;
		}

		// DirectX는 wstring 필요 → 변환
		std::wstring wpath = Core::UTF8ToWString(path);

		auto texture = std::make_shared<Graphics::Texture>();
		Core::uint32 frameIndex = mRenderer->GetCurrentFrameIndex();

		if (!texture->LoadFromFile(
			mDevice->GetDevice(),
			mDevice->GetGraphicsQueue(),
			mDevice->GetCommandContext(frameIndex),
			wpath.c_str()))  // Win32 API에 wstring 전달
		{
			LOG_ERROR("Failed to load texture: %s", path.c_str());
			return ResourceId::Invalid();
		}

		mTextures[id] = texture;
		mTexturePaths[id] = path;  // UTF-8로 저장!

		LOG_DEBUG("Loaded texture: %s (ID: 0x%llX)", path.c_str(), id.id);
		return id;
	}

	ResourceId ResourceManager::LoadTextureW(const std::wstring& path)
	{
		return LoadTexture(Core::WStringToUTF8(path));
	}

	Graphics::Texture* ResourceManager::GetTexture(ResourceId id)
	{
		auto it = mTextures.find(id);
		if (it != mTextures.end())
		{
			return it->second.get();
		}

		LOG_WARN("Texture not found: ID 0x%llX", id.id);
		return nullptr;
	}

	const Graphics::Texture* ResourceManager::GetTexture(ResourceId id) const
	{
		auto it = mTextures.find(id);
		if (it != mTextures.end())
		{
			return it->second.get();
		}
		return nullptr;
	}

	bool ResourceManager::RemoveTexture(ResourceId id)
	{
		auto it = mTextures.find(id);
		if (it != mTextures.end())
		{
			mTextures.erase(it);
			mTexturePaths.erase(id);

			LOG_DEBUG("Removed texture: ID 0x%llX", id.id);
			return true;
		}
		return false;
	}

	//=========================================================================
	// 디버깅 & 편의 함수
	//=========================================================================

	ResourceId ResourceManager::FindMeshByName(const std::string& name) const
	{
		ResourceId id;
		id.id = Core::Hash64(name);

		if (mMeshes.find(id) != mMeshes.end())
		{
			return id;
		}
		return ResourceId::Invalid();
	}

	ResourceId ResourceManager::FindMaterialByName(const std::string& name) const
	{
		ResourceId id;
		id.id = Core::Hash64(name);

		if (mMaterials.find(id) != mMaterials.end())
		{
			return id;
		}
		return ResourceId::Invalid();
	}

	ResourceId ResourceManager::FindTextureByPath(const std::string& path) const
	{
		ResourceId id;
		id.id = Core::Hash64(path);

		if (mTextures.find(id) != mTextures.end())
		{
			return id;
		}
		return ResourceId::Invalid();
	}

	void ResourceManager::Clear()
	{
		LOG_INFO("Clearing all resources...");

		// 메시 정리
		for (auto& [id, mesh] : mMeshes)
		{
			mesh->Shutdown();
		}
		mMeshes.clear();
		mMeshNames.clear();

		// 머티리얼과 텍스처는 자동으로 정리됨 (shared_ptr)
		mMaterials.clear();
		mMaterialNames.clear();

		mTextures.clear();
		mTexturePaths.clear();

		LOG_INFO("All resources cleared");
	}

} // namespace Framework
