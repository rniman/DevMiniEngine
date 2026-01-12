/**
 * @file ResourceManager.cpp
 * @brief ResourceManager 구현
 *
 * @note Phase 4.3: sRGB/Linear 색공간 처리, AssetManager 연동
 * @note Phase 4.4: 서브메시 정보 복사 (CreateMeshFromAsset)
 */
#include "pch.h"
#include "Framework/Resources/ResourceManager.h"
#include "Framework/Assets/AssetManager.h"
#include "Framework/Assets/MeshAsset.h"
#include "Framework/Assets/TextureAsset.h"
#include "Core/Logging/LogMacros.h"
#include "Core/Hash.h"
#include "Core/Types.h"
#include "Graphics/DX12/DX12Device.h"
#include "Graphics/DX12/DX12Renderer.h"
#include "Graphics/MaterialResource.h"
#include "Graphics/MeshResource.h"
#include "Graphics/TextureResource.h"

namespace Framework
{
	ResourceManager::ResourceManager(Graphics::DX12Device* device, Graphics::DX12Renderer* renderer)
		: mDevice(device)
		, mRenderer(renderer)
		, mAssetManager(nullptr)
	{
		LOG_INFO("ResourceManager initialized");

		// 폴백 텍스처 생성 (1x1 Magenta)
		CreateFallbackTexture();
	}

	ResourceManager::~ResourceManager()
	{
		Clear();
		LOG_INFO("ResourceManager destroyed");
	}

	//=========================================================================
	// 폴백 텍스처 생성
	//=========================================================================

	void ResourceManager::CreateFallbackTexture()
	{
		// 1x1 Magenta 픽셀 (RGBA)
		const Core::uint8 magentaPixel[4] = { 255, 0, 255, 255 };

		auto texture = std::make_shared<Graphics::TextureResource>();

		Core::uint32 frameIndex = mRenderer->GetCurrentFrameIndex();

		if (!texture->CreateFromMemory(
			mDevice->GetDevice(),
			mDevice->GetCommandQueue(),
			mDevice->GetCommandContext(frameIndex),
			magentaPixel,
			1, 1,
			DXGI_FORMAT_R8G8B8A8_UNORM
		))
		{
			LOG_ERROR("[ResourceManager] Failed to create fallback texture");
			return;
		}

		// 특별한 경로로 해시 생성 (일반 파일과 충돌 방지)
		mFallbackTextureId.id = Core::Hash64("__fallback_magenta__");
		mTextures[mFallbackTextureId] = texture;
		mTexturePaths[mFallbackTextureId] = "__fallback_magenta__";

		// 폴백 텍스처는 AssetManager에 등록하지 않음 (특수 용도)

		LOG_INFO(
			"[ResourceManager] Fallback texture created (1x1 Magenta, ID: 0x%llX)",
			mFallbackTextureId.id
		);
	}

	//=========================================================================
	// AssetManager 연동 헬퍼
	//=========================================================================

	void ResourceManager::RegisterTextureAssetToManager(
		ResourceId id,
		const std::string& path,
		Graphics::TextureType textureType,
		const Graphics::TextureResource* texture
	)
	{
		if (!mAssetManager)
		{
			return;
		}

		mAssetManager->RegisterTextureAsset(
			id,
			path,
			textureType,
			texture->GetWidth(),
			texture->GetHeight(),
			texture->GetFormat(),
			texture->IsSRGB()
		);
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
		auto mesh = std::make_shared<Graphics::MeshResource>();
		mMeshes[id] = mesh;
		mMeshNames[id] = name;

		LOG_DEBUG("Created mesh: %s (ID: 0x%llX)", name.c_str(), id.id);
		return id;
	}

	ResourceId ResourceManager::CreateMeshFromAsset(
		const std::string& name,
		std::unique_ptr<MeshAsset> meshAsset)
	{
		if (!meshAsset)
		{
			LOG_ERROR("[ResourceManager] CreateMeshFromAsset: meshAsset is null");
			return ResourceId::Invalid();
		}
		if (!meshAsset->HasSourceData())
		{
			LOG_ERROR("[ResourceManager] CreateMeshFromAsset: meshAsset has no source data");
			return ResourceId::Invalid();
		}
		if (meshAsset->GetVertexCount() == 0)
		{
			LOG_ERROR("[ResourceManager] CreateMeshFromAsset: meshAsset has no vertices");
			return ResourceId::Invalid();
		}

		// 이름을 64비트 해시로 변환
		ResourceId id;
		id.id = Core::Hash64(name);

		// 이미 존재하는지 확인
		auto it = mMeshes.find(id);
		if (it != mMeshes.end())
		{
			LOG_WARN("[ResourceManager] Mesh '%s' already exists (ID: 0x%llX)", name.c_str(), id.id);
			return id;
		}

		// 새 메시 생성
		auto mesh = std::make_shared<Graphics::MeshResource>();

		// 정점 데이터
		const auto& vertices = meshAsset->GetVertices();
		const auto& indices = meshAsset->GetIndices();

		// 인덱스 포맷 선택 (16비트 vs 32비트)
		bool use16BitIndices = meshAsset->CanUse16BitIndices();

		Core::uint32 frameIndex = mRenderer->GetCurrentFrameIndex();
		bool success = false;

		if (use16BitIndices)
		{
			// 16비트 인덱스 사용
			std::vector<Core::uint16> indices16 = meshAsset->GetIndices16();

			success = mesh->InitializeStandard(
				mDevice->GetDevice(),
				mDevice->GetCommandQueue(),
				mDevice->GetCommandContext(frameIndex),
				vertices.data(),
				static_cast<Core::uint32>(vertices.size()),
				indices16.data(),
				static_cast<Core::uint32>(indices16.size())
			);
		}
		else
		{
			// 32비트 인덱스 사용
			success = mesh->InitializeStandard32(
				mDevice->GetDevice(),
				mDevice->GetCommandQueue(),
				mDevice->GetCommandContext(frameIndex),
				vertices.data(),
				static_cast<Core::uint32>(vertices.size()),
				indices.data(),
				static_cast<Core::uint32>(indices.size())
			);
		}

		if (!success)
		{
			LOG_ERROR("[ResourceManager] Failed to initialize mesh from asset: %s", name.c_str());
			return ResourceId::Invalid();
		}

		// Phase 4.4: 서브메시 정보 복사
		const auto& submeshes = meshAsset->GetSubmeshes();
		if (submeshes.size() > 1)
		{
			mesh->SetSubmeshes(submeshes);
			LOG_DEBUG("[ResourceManager] Multi-submesh copied: %u submesh(es)", mesh->GetSubmeshCount());
		}

		LOG_DEBUG(
			"[ResourceManager] Submeshes copied: %u submesh(es)",
			mesh->GetSubmeshCount()
		);

		// MeshDataPolicy에 따라 CPU 데이터 해제
		if (meshAsset->GetDataPolicy() == MeshDataPolicy::ReleaseAfterUpload)
		{
			meshAsset->ReleaseSourceData();
			LOG_DEBUG("[ResourceManager] Released CPU mesh data after GPU upload");
		}

		// GPU Resource 등록
		mMeshes[id] = mesh;
		mMeshNames[id] = name;

		// AssetManager에 MeshAsset 등록 (소유권 이전)
		if (mAssetManager)
		{
			mAssetManager->RegisterMeshAsset(id, name, std::move(meshAsset));
		}

		LOG_INFO(
			"[ResourceManager] Created mesh from asset: %s (V:%u, I:%u, %s, ID: 0x%llX)",
			name.c_str(),
			static_cast<Core::uint32>(vertices.size()),
			static_cast<Core::uint32>(indices.size()),
			use16BitIndices ? "16-bit" : "32-bit",
			id.id
		);

		return id;
	}

	Graphics::MeshResource* ResourceManager::GetMesh(ResourceId id)
	{
		auto it = mMeshes.find(id);
		if (it != mMeshes.end())
		{
			return it->second.get();
		}

		LOG_WARN("Mesh not found: ID 0x%llX", id.id);
		return nullptr;
	}

	const Graphics::MeshResource* ResourceManager::GetMesh(ResourceId id) const
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
		ResourceId id;
		id.id = Core::Hash64(name);

		auto it = mMaterials.find(id);
		if (it != mMaterials.end())
		{
			LOG_WARN("Material '%s' already exists (ID: 0x%llX)", name.c_str(), id.id);
			return id;
		}

		Graphics::MaterialDesc desc;
		desc.vertexShaderPath = vertexShader.c_str();
		desc.pixelShaderPath = pixelShader.c_str();

		auto material = std::make_shared<Graphics::MaterialResource>(desc);
		mMaterials[id] = material;
		mMaterialNames[id] = name;

		LOG_DEBUG("Created material: %s (ID: 0x%llX)", name.c_str(), id.id);
		return id;
	}

	Graphics::MaterialResource* ResourceManager::GetMaterial(ResourceId id)
	{
		auto it = mMaterials.find(id);
		if (it != mMaterials.end())
		{
			return it->second.get();
		}

		LOG_WARN("Material not found: ID 0x%llX", id.id);
		return nullptr;
	}

	const Graphics::MaterialResource* ResourceManager::GetMaterial(ResourceId id) const
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

	ResourceId ResourceManager::LoadTexture(const std::string& path, Graphics::TextureType textureType)
	{
		ResourceId id;
		id.id = Core::Hash64(path);

		auto it = mTextures.find(id);
		if (it != mTextures.end())
		{
			LOG_DEBUG("[ResourceManager] Texture already loaded: %s (ID: 0x%llX)", path.c_str(), id.id);
			return id;
		}

		std::wstring wpath = Core::UTF8ToWString(path);

		auto texture = std::make_shared<Graphics::TextureResource>();
		Core::uint32 frameIndex = mRenderer->GetCurrentFrameIndex();

		// TextureType 전달하여 색공간 자동 적용
		if (!texture->LoadFromFile(
			mDevice->GetDevice(),
			mDevice->GetCommandQueue(),
			mDevice->GetCommandContext(frameIndex),
			wpath.c_str(),
			textureType
		))
		{
			LOG_ERROR("[ResourceManager] Failed to load texture: %s", path.c_str());
			return ResourceId::Invalid();
		}

		mTextures[id] = texture;
		mTexturePaths[id] = path;

		// AssetManager에 TextureAsset 등록
		RegisterTextureAssetToManager(id, path, textureType, texture.get());

		LOG_DEBUG(
			"[ResourceManager] Loaded texture: %s (ID: 0x%llX, %ux%u, Type: %s, sRGB: %s)",
			path.c_str(),
			id.id,
			texture->GetWidth(),
			texture->GetHeight(),
			Graphics::TextureTypeToString(textureType),
			texture->IsSRGB() ? "Yes" : "No"
		);
		return id;
	}

	ResourceId ResourceManager::LoadTextureW(const std::wstring& path, Graphics::TextureType textureType)
	{
		return LoadTexture(Core::WStringToUTF8(path), textureType);
	}

	ResourceId ResourceManager::LoadTextureFromMemory(
		const std::string& name,
		const void* data,
		Core::uint32 dataSize,
		Graphics::TextureType textureType
	)
	{
		if (!data || dataSize == 0)
		{
			LOG_ERROR("[ResourceManager] LoadTextureFromMemory: Invalid data");
			return ResourceId::Invalid();
		}

		ResourceId id;
		id.id = Core::Hash64(name);

		// 이미 존재하는지 확인
		auto it = mTextures.find(id);
		if (it != mTextures.end())
		{
			LOG_DEBUG("[ResourceManager] Texture already loaded: %s (ID: 0x%llX)", name.c_str(), id.id);
			return id;
		}

		auto texture = std::make_shared<Graphics::TextureResource>();
		Core::uint32 frameIndex = mRenderer->GetCurrentFrameIndex();

		// TextureType 전달하여 색공간 자동 적용
		if (!texture->LoadFromMemory(
			mDevice->GetDevice(),
			mDevice->GetCommandQueue(),
			mDevice->GetCommandContext(frameIndex),
			data,
			dataSize,
			textureType
		))
		{
			LOG_ERROR("[ResourceManager] Failed to load texture from memory: %s", name.c_str());
			return ResourceId::Invalid();
		}

		mTextures[id] = texture;
		mTexturePaths[id] = name;

		// AssetManager에 TextureAsset 등록
		RegisterTextureAssetToManager(id, name, textureType, texture.get());

		LOG_DEBUG(
			"[ResourceManager] Loaded texture from memory: %s (ID: 0x%llX, %ux%u, %u bytes, Type: %s, sRGB: %s)",
			name.c_str(),
			id.id,
			texture->GetWidth(),
			texture->GetHeight(),
			dataSize,
			Graphics::TextureTypeToString(textureType),
			texture->IsSRGB() ? "Yes" : "No"
		);
		return id;
	}

	ResourceId ResourceManager::CreateTextureFromMemory(
		const std::string& name,
		const void* data,
		Core::uint32 width,
		Core::uint32 height,
		Graphics::TextureType textureType
	)
	{
		if (!data || width == 0 || height == 0)
		{
			LOG_ERROR("[ResourceManager] CreateTextureFromMemory: Invalid parameters");
			return ResourceId::Invalid();
		}

		ResourceId id;
		id.id = Core::Hash64(name);

		// 이미 존재하는지 확인
		auto it = mTextures.find(id);
		if (it != mTextures.end())
		{
			LOG_DEBUG("[ResourceManager] Texture already exists: %s (ID: 0x%llX)", name.c_str(), id.id);
			return id;
		}

		// TextureType에 따라 포맷 결정
		DXGI_FORMAT format = Graphics::IsSRGBTexture(textureType)
			? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
			: DXGI_FORMAT_R8G8B8A8_UNORM;

		auto texture = std::make_shared<Graphics::TextureResource>();
		Core::uint32 frameIndex = mRenderer->GetCurrentFrameIndex();

		if (!texture->CreateFromMemory(
			mDevice->GetDevice(),
			mDevice->GetCommandQueue(),
			mDevice->GetCommandContext(frameIndex),
			data,
			width,
			height,
			format
		))
		{
			LOG_ERROR("[ResourceManager] Failed to create texture from memory: %s", name.c_str());
			return ResourceId::Invalid();
		}

		mTextures[id] = texture;
		mTexturePaths[id] = name;

		// AssetManager에 TextureAsset 등록
		RegisterTextureAssetToManager(id, name, textureType, texture.get());

		LOG_DEBUG(
			"[ResourceManager] Created texture from memory: %s (ID: 0x%llX, %ux%u, Type: %s, sRGB: %s)",
			name.c_str(),
			id.id,
			width,
			height,
			Graphics::TextureTypeToString(textureType),
			texture->IsSRGB() ? "Yes" : "No"
		);
		return id;
	}

	Graphics::TextureResource* ResourceManager::GetTexture(ResourceId id)
	{
		auto it = mTextures.find(id);
		if (it != mTextures.end())
		{
			return it->second.get();
		}

		LOG_WARN("Texture not found: ID 0x%llX", id.id);
		return nullptr;
	}

	const Graphics::TextureResource* ResourceManager::GetTexture(ResourceId id) const
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
		// 폴백 텍스처는 제거 불가
		if (id.id == mFallbackTextureId.id)
		{
			LOG_WARN("[ResourceManager] Cannot remove fallback texture");
			return false;
		}

		auto it = mTextures.find(id);
		if (it != mTextures.end())
		{
			mTextures.erase(it);
			mTexturePaths.erase(id);

			// AssetManager에서도 제거
			if (mAssetManager)
			{
				mAssetManager->UnregisterTextureAsset(id);
			}

			LOG_DEBUG("Removed texture: ID 0x%llX", id.id);
			return true;
		}
		return false;
	}

	//=========================================================================
	// TextureAsset 접근 (AssetManager 위임)
	//=========================================================================

	TextureAsset* ResourceManager::GetTextureAsset(ResourceId id)
	{
		if (mAssetManager)
		{
			return mAssetManager->GetTextureAsset(id);
		}
		return nullptr;
	}

	const TextureAsset* ResourceManager::GetTextureAsset(ResourceId id) const
	{
		if (mAssetManager)
		{
			return mAssetManager->GetTextureAsset(id);
		}
		return nullptr;
	}

	//=========================================================================
	// 검색 & 유틸리티
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

		for (auto& [id, mesh] : mMeshes)
		{
			if (mesh)
			{
				mesh->Shutdown();
			}
		}
		mMeshes.clear();
		mMeshNames.clear();

		mMaterials.clear();
		mMaterialNames.clear();

		for (auto& [id, texture] : mTextures)
		{
			if (texture)
			{
				texture->Shutdown();
			}
		}
		mTextures.clear();
		mTexturePaths.clear();

		// 폴백 텍스처 ID도 초기화
		mFallbackTextureId = ResourceId::Invalid();

		LOG_INFO("All resources cleared");
	}

} // namespace Framework
