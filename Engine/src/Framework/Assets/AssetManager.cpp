/**
 * @file AssetManager.cpp
 * @brief AssetManager 클래스 구현
 *
 * @note Phase 4.3: TextureAsset 메타데이터 관리 추가
 * @note Phase 4.5: Default Asset 시스템 제거 (ResourceManager 폴백 사용)
 */
#include "pch.h"
#include "Framework/Assets/AssetManager.h"
#include "Framework/Assets/MeshAsset.h"
#include "Framework/Assets/TextureAsset.h"
#include "Framework/Assets/MaterialAsset.h"
#include "Framework/Resources/ResourceManager.h"
#include "Core/Hash.h"
#include "Core/Logging/LogMacros.h"

namespace Framework
{
	//=========================================================================
	// 생성자/소멸자
	//=========================================================================

	AssetManager::AssetManager(ResourceManager* resourceManager)
		: mResourceManager(resourceManager)
		, mAssetRoot("Assets/")
		, mInitialized(false)
	{
	}

	AssetManager::~AssetManager()
	{
		if (mInitialized)
		{
			Shutdown();
		}
	}

	//=========================================================================
	// 초기화/종료
	//=========================================================================

	bool AssetManager::Initialize(const std::string& assetRoot)
	{
		if (mInitialized)
		{
			LOG_WARN("[AssetManager] Already initialized");
			return true;
		}

		mAssetRoot = assetRoot;

		// 경로 끝에 슬래시 보장
		if (!mAssetRoot.empty() && mAssetRoot.back() != '/' && mAssetRoot.back() != '\\')
		{
			mAssetRoot += '/';
		}

		mInitialized = true;

		LOG_INFO("[AssetManager] Initialized with root: %s", mAssetRoot.c_str());
		return true;
	}

	void AssetManager::Shutdown()
	{
		if (!mInitialized)
		{
			return;
		}

		LOG_INFO("[AssetManager] Shutting down...");

		// 모든 Asset 해제
		mAssetCache.clear();
		mAssetPaths.clear();

		mInitialized = false;

		LOG_INFO("[AssetManager] Shutdown complete");
	}

	//=========================================================================
	// TextureAsset 등록 (ResourceManager 전용)
	//=========================================================================

	void AssetManager::RegisterTextureAsset(
		ResourceId id,
		const std::string& path,
		Graphics::TextureType textureType,
		Core::uint32 width,
		Core::uint32 height,
		DXGI_FORMAT format,
		bool isSRGB
	)
	{
		// 이미 존재하면 스킵
		auto it = mAssetCache.find(id);
		if (it != mAssetCache.end())
		{
			LOG_DEBUG("[AssetManager] TextureAsset already registered: %s (ID: 0x%llX)", path.c_str(), id.id);
			return;
		}

		// TextureAsset 생성 및 메타데이터 설정
		auto asset = std::make_unique<TextureAsset>();
		asset->mPath = path;
		asset->mState = AssetState::Loaded;
		asset->SetTextureType(textureType);
		asset->SetMetadata(width, height, format, isSRGB);

		// AssetEntry 생성
		AssetEntry entry;
		entry.asset = std::move(asset);
		entry.state = AssetState::Loaded;
		entry.refCount = 0;

		// 캐시에 추가
		mAssetCache[id] = std::move(entry);
		mAssetPaths[id] = path;

		LOG_DEBUG(
			"[AssetManager] Registered TextureAsset: %s (ID: 0x%llX, %ux%u, sRGB: %s)",
			path.c_str(),
			id.id,
			width,
			height,
			isSRGB ? "Yes" : "No"
		);
	}

	void AssetManager::UnregisterTextureAsset(ResourceId id)
	{
		auto it = mAssetCache.find(id);
		if (it == mAssetCache.end())
		{
			return;
		}

		// TextureAsset인지 확인
		if (it->second.asset && it->second.asset->GetType() != AssetType::Texture)
		{
			LOG_WARN("[AssetManager] Asset is not a TextureAsset (ID: 0x%llX)", id.id);
			return;
		}

		mAssetCache.erase(it);
		mAssetPaths.erase(id);

		LOG_DEBUG("[AssetManager] Unregistered TextureAsset (ID: 0x%llX)", id.id);
	}

	TextureAsset* AssetManager::GetTextureAsset(ResourceId id)
	{
		auto it = mAssetCache.find(id);
		if (it != mAssetCache.end() && it->second.asset)
		{
			if (it->second.asset->GetType() == AssetType::Texture)
			{
				return static_cast<TextureAsset*>(it->second.asset.get());
			}
		}
		return nullptr;
	}

	const TextureAsset* AssetManager::GetTextureAsset(ResourceId id) const
	{
		auto it = mAssetCache.find(id);
		if (it != mAssetCache.end() && it->second.asset)
		{
			if (it->second.asset->GetType() == AssetType::Texture)
			{
				return static_cast<const TextureAsset*>(it->second.asset.get());
			}
		}
		return nullptr;
	}

	//=========================================================================
	// MeshAsset 등록 (ResourceManager 전용)
	//=========================================================================

	void AssetManager::RegisterMeshAsset(
		ResourceId id,
		const std::string& name,
		std::unique_ptr<MeshAsset> asset
	)
	{
		// 이미 존재하면 스킵
		auto it = mAssetCache.find(id);
		if (it != mAssetCache.end())
		{
			LOG_DEBUG("[AssetManager] MeshAsset already registered: %s (ID: 0x%llX)", name.c_str(), id.id);
			return;
		}

		if (!asset)
		{
			LOG_ERROR("[AssetManager] Cannot register null MeshAsset: %s", name.c_str());
			return;
		}

		// AssetEntry 생성
		AssetEntry entry;
		entry.asset = std::move(asset);
		entry.state = AssetState::Loaded;
		entry.refCount = 0;

		// 캐시에 추가
		mAssetCache[id] = std::move(entry);
		mAssetPaths[id] = name;

		LOG_DEBUG(
			"[AssetManager] Registered MeshAsset: %s (ID: 0x%llX)",
			name.c_str(),
			id.id
		);
	}

	void AssetManager::UnregisterMeshAsset(ResourceId id)
	{
		auto it = mAssetCache.find(id);
		if (it == mAssetCache.end())
		{
			return;
		}

		// MeshAsset인지 확인
		if (it->second.asset && it->second.asset->GetType() != AssetType::Mesh)
		{
			LOG_WARN("[AssetManager] Asset is not a MeshAsset (ID: 0x%llX)", id.id);
			return;
		}

		mAssetCache.erase(it);
		mAssetPaths.erase(id);

		LOG_DEBUG("[AssetManager] Unregistered MeshAsset (ID: 0x%llX)", id.id);
	}

	MeshAsset* AssetManager::GetMeshAsset(ResourceId id)
	{
		auto it = mAssetCache.find(id);
		if (it != mAssetCache.end() && it->second.asset)
		{
			if (it->second.asset->GetType() == AssetType::Mesh)
			{
				return static_cast<MeshAsset*>(it->second.asset.get());
			}
		}
		return nullptr;
	}

	const MeshAsset* AssetManager::GetMeshAsset(ResourceId id) const
	{
		auto it = mAssetCache.find(id);
		if (it != mAssetCache.end() && it->second.asset)
		{
			if (it->second.asset->GetType() == AssetType::Mesh)
			{
				return static_cast<const MeshAsset*>(it->second.asset.get());
			}
		}
		return nullptr;
	}

	//=========================================================================
	// Asset 조회
	//=========================================================================

	bool AssetManager::HasAsset(ResourceId id) const
	{
		return mAssetCache.find(id) != mAssetCache.end();
	}

	AssetState AssetManager::GetState(ResourceId id) const
	{
		auto it = mAssetCache.find(id);
		if (it != mAssetCache.end())
		{
			return it->second.state;
		}
		return AssetState::Unloaded;
	}

	bool AssetManager::IsLoaded(ResourceId id) const
	{
		return GetState(id) == AssetState::Loaded;
	}

	//=========================================================================
	// Asset 해제
	//=========================================================================

	bool AssetManager::Unload(ResourceId id)
	{
		auto it = mAssetCache.find(id);
		if (it == mAssetCache.end())
		{
			return false;
		}

		// 경로 조회 (로깅용)
		std::string path;
		auto pathIt = mAssetPaths.find(id);
		if (pathIt != mAssetPaths.end())
		{
			path = pathIt->second;
		}

		// 캐시에서 제거
		mAssetCache.erase(it);
		mAssetPaths.erase(id);

		LOG_DEBUG("[AssetManager] Unloaded: %s (ID: 0x%llX)", path.c_str(), id.id);
		return true;
	}

	Core::uint32 AssetManager::UnloadUnusedAssets()
	{
		Core::uint32 unloadedCount = 0;

		for (auto it = mAssetCache.begin(); it != mAssetCache.end();)
		{
			if (it->second.refCount == 0)
			{
				ResourceId id = it->first;

				// 경로 조회 (로깅용)
				std::string path;
				auto pathIt = mAssetPaths.find(id);
				if (pathIt != mAssetPaths.end())
				{
					path = pathIt->second;
					mAssetPaths.erase(pathIt);
				}

				LOG_DEBUG("[AssetManager] Unloading unused: %s (ID: 0x%llX)", path.c_str(), id.id);

				it = mAssetCache.erase(it);
				++unloadedCount;
			}
			else
			{
				++it;
			}
		}

		if (unloadedCount > 0)
		{
			LOG_INFO("[AssetManager] Unloaded %u unused assets", unloadedCount);
		}

		return unloadedCount;
	}

	void AssetManager::Clear()
	{
		Core::uint32 count = static_cast<Core::uint32>(mAssetCache.size());

		mAssetCache.clear();
		mAssetPaths.clear();

		LOG_INFO("[AssetManager] Cleared %u assets", count);
	}

	//=========================================================================
	// 참조 카운팅
	//=========================================================================

	void AssetManager::AddRef(ResourceId id)
	{
		auto it = mAssetCache.find(id);
		if (it != mAssetCache.end())
		{
			++it->second.refCount;
		}
	}

	void AssetManager::Release(ResourceId id)
	{
		auto it = mAssetCache.find(id);
		if (it != mAssetCache.end() && it->second.refCount > 0)
		{
			--it->second.refCount;
		}
	}

	Core::uint32 AssetManager::GetRefCount(ResourceId id) const
	{
		auto it = mAssetCache.find(id);
		if (it != mAssetCache.end())
		{
			return it->second.refCount;
		}
		return 0;
	}

	//=========================================================================
	// 유틸리티
	//=========================================================================

	void AssetManager::SetAssetRoot(const std::string& assetRoot)
	{
		mAssetRoot = assetRoot;

		// 경로 끝에 슬래시 보장
		if (!mAssetRoot.empty() && mAssetRoot.back() != '/' && mAssetRoot.back() != '\\')
		{
			mAssetRoot += '/';
		}

		LOG_INFO("[AssetManager] Asset root changed to: %s", mAssetRoot.c_str());
	}

	Core::uint32 AssetManager::GetLoadedAssetCount() const
	{
		Core::uint32 count = 0;
		for (const auto& [id, entry] : mAssetCache)
		{
			if (entry.state == AssetState::Loaded)
			{
				++count;
			}
		}
		return count;
	}

	std::vector<AssetInfo> AssetManager::GetLoadedAssetInfos() const
	{
		std::vector<AssetInfo> infos;
		infos.reserve(mAssetCache.size());

		for (const auto& [id, entry] : mAssetCache)
		{
			AssetInfo info;

			// 경로 조회
			auto pathIt = mAssetPaths.find(id);
			if (pathIt != mAssetPaths.end())
			{
				info.path = pathIt->second;
			}

			// Asset 정보
			if (entry.asset)
			{
				info.type = entry.asset->GetType();
				info.memoryUsage = entry.asset->GetMemoryUsage();
			}
			else
			{
				info.type = AssetType::Unknown;
				info.memoryUsage = 0;
			}

			info.state = entry.state;
			info.refCount = entry.refCount;

			infos.push_back(info);
		}

		return infos;
	}

	ResourceId AssetManager::FindByPath(const std::string& path) const
	{
		ResourceId id = PathToId(path);

		if (mAssetCache.find(id) != mAssetCache.end())
		{
			return id;
		}

		return ResourceId::Invalid();
	}

	std::string AssetManager::ResolvePath(const std::string& relativePath) const
	{
		return mAssetRoot + relativePath;
	}

	ResourceId AssetManager::PathToId(const std::string& path) const
	{
		ResourceId id;
		id.id = Core::Hash64(path);
		return id;
	}

} // namespace Framework
