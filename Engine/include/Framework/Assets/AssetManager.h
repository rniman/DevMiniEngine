/**
 * @file AssetManager.h
 * @brief 파일 기반 Asset의 중앙 집중식 관리자
 *
 * Asset의 로딩, 캐싱, 생명주기를 관리합니다.
 * ResourceManager와 협력하여 Asset을 GPU Resource로 변환합니다.
 *
 * @note Phase 4.3: TextureAsset 메타데이터 관리 추가
 * @note Phase 4.5: Default Asset 시스템 제거 (ResourceManager 폴백 사용)
 */
#pragma once
#include "Framework/Assets/AssetTypes.h"
#include "Framework/Assets/IAsset.h"
#include "Framework/Resources/ResourceId.h"
#include "Graphics/TextureType.h"
#include "Core/Types.h"
#include <dxgiformat.h>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace Framework
{
	// 전방 선언
	class ResourceManager;
	class MeshAsset;
	class TextureAsset;
	class MaterialAsset;

	//=========================================================================
	// 타입 → AssetType 매핑 (컴파일 타임)
	//=========================================================================

	/**
	 * @brief Asset 클래스 타입에서 AssetType 열거형을 반환
	 * @tparam T Asset 클래스 타입
	 * @return 해당 AssetType
	 */
	template<typename T>
	constexpr AssetType GetAssetTypeFor()
	{
		static_assert(std::is_base_of_v<IAsset, T>, "T must derive from IAsset");
		return AssetType::Unknown;
	}

	template<> constexpr AssetType GetAssetTypeFor<MeshAsset>() { return AssetType::Mesh; }
	template<> constexpr AssetType GetAssetTypeFor<TextureAsset>() { return AssetType::Texture; }
	template<> constexpr AssetType GetAssetTypeFor<MaterialAsset>() { return AssetType::Material; }

	/**
	 * @brief Asset 엔트리 (내부 관리용)
	 */
	struct AssetEntry
	{
		std::unique_ptr<IAsset> asset;
		AssetState state = AssetState::Unloaded;
		Core::uint32 refCount = 0;
		// TODO: Hot Reload용
		// std::filesystem::file_time_type lastModifiedTime;
	};

	/**
	 * @brief 파일 기반 Asset의 중앙 집중식 관리자
	 *
	 * ResourceManager와 동일한 패턴으로 ResourceId 기반 관리.
	 * 파일 경로의 해시를 ID로 사용하여 중복 로딩 방지.
	 *
	 * @note Phase 4.1: 동기 로딩만 구현, 비동기는 API만 정의
	 * @note Phase 4.3: TextureAsset 메타데이터 관리
	 * @note Asset은 CPU 데이터, GPU Resource 변환은 ResourceManager 담당
	 * @note 폴백 텍스처는 ResourceManager::GetFallbackTexture() 사용
	 */
	class AssetManager
	{
	public:
		/**
		 * @brief 생성자
		 * @param resourceManager GPU 리소스 생성용 (의존성 주입)
		 */
		explicit AssetManager(ResourceManager* resourceManager);
		~AssetManager();

		// 복사 금지
		AssetManager(const AssetManager&) = delete;
		AssetManager& operator=(const AssetManager&) = delete;

		//=========================================================================
		// 초기화/종료
		//=========================================================================

		/**
		 * @brief AssetManager 초기화
		 * @param assetRoot Asset 루트 디렉토리 (예: "Assets/")
		 * @return 성공 여부
		 */
		bool Initialize(const std::string& assetRoot = "Assets/");

		/**
		 * @brief AssetManager 종료 및 모든 Asset 해제
		 */
		void Shutdown();

		//=========================================================================
		// Asset 로딩
		//=========================================================================

		/**
		 * @brief Asset 동기 로드
		 *
		 * @tparam T Asset 타입 (MeshAsset, TextureAsset 등)
		 * @param path Asset 상대 경로 (예: "Models/Helmet.gltf")
		 * @return ResourceId (실패 시 Invalid)
		 *
		 * @note 이미 로드된 Asset은 캐시에서 반환
		 * @note Phase 4.1: 구조만 생성, 실제 파일 로딩은 Phase 4.2+
		 */
		template<typename T>
		ResourceId Load(const std::string& path);

		/**
		 * @brief Asset 비동기 로드 요청
		 *
		 * @tparam T Asset 타입
		 * @param path Asset 상대 경로
		 * @return ResourceId (즉시 반환, 로딩은 백그라운드)
		 *
		 * @note Phase 4.1: 내부적으로 동기 로딩, 향후 실제 비동기 구현
		 */
		template<typename T>
		ResourceId LoadAsync(const std::string& path);

		//=========================================================================
		// Asset 조회
		//=========================================================================

		/**
		 * @brief ResourceId로 Asset 조회
		 *
		 * @tparam T Asset 타입
		 * @param id ResourceId
		 * @return Asset 포인터 (없거나 타입 불일치 시 nullptr)
		 *
		 * @note DEBUG 빌드에서 타입 불일치 시 에러 로그 출력
		 */
		template<typename T>
		T* GetAsset(ResourceId id);

		template<typename T>
		const T* GetAsset(ResourceId id) const;

		/**
		 * @brief Asset 존재 여부 확인
		 * @param id ResourceId
		 * @return 캐시에 존재하면 true
		 */
		bool HasAsset(ResourceId id) const;

		/**
		 * @brief Asset 상태 조회
		 * @param id ResourceId
		 * @return AssetState (없으면 Unloaded)
		 */
		AssetState GetState(ResourceId id) const;

		/**
		 * @brief Asset이 로드 완료되었는지 확인
		 * @param id ResourceId
		 * @return Loaded 상태이면 true
		 */
		bool IsLoaded(ResourceId id) const;

		//=========================================================================
		// TextureAsset 등록 (ResourceManager 전용)
		//=========================================================================

		/**
		 * @brief TextureAsset 등록 (ResourceManager가 호출)
		 *
		 * ResourceManager가 GPU 텍스처 로드 후 메타데이터를 등록합니다.
		 *
		 * @param id ResourceId (ResourceManager와 동일한 ID 사용)
		 * @param path 파일 경로 또는 이름
		 * @param textureType 텍스처 용도
		 * @param width 텍스처 너비
		 * @param height 텍스처 높이
		 * @param format DXGI 포맷
		 * @param isSRGB sRGB 색공간 여부
		 */
		void RegisterTextureAsset(
			ResourceId id,
			const std::string& path,
			Graphics::TextureType textureType,
			Core::uint32 width,
			Core::uint32 height,
			DXGI_FORMAT format,
			bool isSRGB
		);

		/**
		 * @brief TextureAsset 등록 해제 (ResourceManager가 호출)
		 * @param id ResourceId
		 */
		void UnregisterTextureAsset(ResourceId id);

		/**
		 * @brief TextureAsset 조회
		 * @param id ResourceId
		 * @return TextureAsset 포인터 (없으면 nullptr)
		 */
		TextureAsset* GetTextureAsset(ResourceId id);
		const TextureAsset* GetTextureAsset(ResourceId id) const;

		//=========================================================================
		// MeshAsset 등록 (ResourceManager 전용)
		//=========================================================================

		/**
		 * @brief MeshAsset 등록 (ResourceManager에서 호출)
		 *
		 * GPU MeshResource 생성 시 대응되는 MeshAsset을 등록합니다.
		 * 소유권이 AssetManager로 이전됩니다.
		 *
		 * @param id ResourceManager에서 생성한 ResourceId
		 * @param name 메시 이름
		 * @param asset MeshAsset (소유권 이전)
		 */
		void RegisterMeshAsset(
			ResourceId id,
			const std::string& name,
			std::unique_ptr<MeshAsset> asset
		);

		/**
		 * @brief MeshAsset 등록 해제
		 *
		 * GPU MeshResource 해제 시 호출합니다.
		 *
		 * @param id 리소스 ID
		 */
		void UnregisterMeshAsset(ResourceId id);

		/**
		 * @brief MeshAsset 조회
		 *
		 * @param id 리소스 ID
		 * @return MeshAsset 포인터 (없으면 nullptr)
		 */
		MeshAsset* GetMeshAsset(ResourceId id);
		const MeshAsset* GetMeshAsset(ResourceId id) const;

		//=========================================================================
		// Asset 해제
		//=========================================================================

		/**
		 * @brief 특정 Asset 강제 해제
		 * @param id ResourceId
		 * @return 해제 성공 여부
		 */
		bool Unload(ResourceId id);

		/**
		 * @brief 참조되지 않는 Asset 일괄 해제
		 *
		 * refCount가 0인 Asset만 해제합니다.
		 * 씬 전환 시 호출하면 효과적입니다.
		 *
		 * @return 해제된 Asset 개수
		 */
		Core::uint32 UnloadUnusedAssets();

		/**
		 * @brief 모든 Asset 해제
		 */
		void Clear();

		//=========================================================================
		// 참조 카운팅 (수동)
		//=========================================================================

		/**
		 * @brief 참조 카운트 증가
		 * @param id ResourceId
		 */
		void AddRef(ResourceId id);

		/**
		 * @brief 참조 카운트 감소
		 * @param id ResourceId
		 */
		void Release(ResourceId id);

		/**
		 * @brief 현재 참조 카운트 조회
		 * @param id ResourceId
		 * @return 참조 카운트 (없으면 0)
		 */
		Core::uint32 GetRefCount(ResourceId id) const;

		//=========================================================================
		// 유틸리티
		//=========================================================================

		/**
		 * @brief Asset 루트 경로 설정
		 * @param assetRoot 루트 디렉토리 경로
		 */
		void SetAssetRoot(const std::string& assetRoot);

		/**
		 * @brief Asset 루트 경로 반환
		 */
		const std::string& GetAssetRoot() const { return mAssetRoot; }

		/**
		 * @brief 로드된 Asset 개수 반환
		 */
		Core::uint32 GetLoadedAssetCount() const;

		/**
		 * @brief 모든 로드된 Asset 정보 반환 (디버그/UI용)
		 */
		std::vector<AssetInfo> GetLoadedAssetInfos() const;

		/**
		 * @brief 경로로 ResourceId 조회
		 * @param path Asset 상대 경로
		 * @return ResourceId (없으면 Invalid)
		 */
		ResourceId FindByPath(const std::string& path) const;

		/**
		 * @brief ResourceManager 접근자
		 */
		ResourceManager* GetResourceManager() { return mResourceManager; }

	private:
		/**
		 * @brief 상대 경로를 전체 경로로 변환
		 */
		std::string ResolvePath(const std::string& relativePath) const;

		/**
		 * @brief 경로에서 ResourceId 생성
		 */
		ResourceId PathToId(const std::string& path) const;

		ResourceManager* mResourceManager;
		std::string mAssetRoot;

		// Asset 캐시 (ResourceId → AssetEntry)
		std::unordered_map<ResourceId, AssetEntry> mAssetCache;

		// 역참조 맵 (ResourceId → 경로, 디버깅용)
		std::unordered_map<ResourceId, std::string> mAssetPaths;

		bool mInitialized = false;
	};

	//=========================================================================
	// 템플릿 구현
	//=========================================================================

	template<typename T>
	ResourceId AssetManager::Load(const std::string& path)
	{
		// 컴파일 타임 타입 검증
		static_assert(std::is_base_of_v<IAsset, T>, "T must derive from IAsset");

		// 경로를 ID로 변환
		ResourceId id = PathToId(path);

		// 이미 캐시에 있으면 반환
		auto it = mAssetCache.find(id);
		if (it != mAssetCache.end())
		{
			return id;
		}

		// 새 Asset 생성
		auto asset = std::make_unique<T>();

		// AssetEntry 생성
		AssetEntry entry;
		entry.asset = std::move(asset);
		entry.state = AssetState::Loaded;  // Phase 4.1: 즉시 Loaded 상태
		entry.refCount = 0;

		// 캐시에 추가
		mAssetCache[id] = std::move(entry);
		mAssetPaths[id] = path;

		// TODO: 실제 파일 로딩
		// entry.state = AssetState::Loading;
		// bool success = LoadFromFile<T>(ResolvePath(path), entry.asset.get());
		// entry.state = success ? AssetState::Loaded : AssetState::Failed;

		return id;
	}

	template<typename T>
	ResourceId AssetManager::LoadAsync(const std::string& path)
	{
		// Phase 4.1: 내부적으로 동기 로딩
		// 향후: 실제 비동기 구현
		return Load<T>(path);
	}

	template<typename T>
	T* AssetManager::GetAsset(ResourceId id)
	{
		static_assert(std::is_base_of_v<IAsset, T>, "T must derive from IAsset");

		auto it = mAssetCache.find(id);
		if (it != mAssetCache.end() && it->second.asset)
		{
#ifdef _DEBUG
			// DEBUG 빌드에서 타입 검증
			constexpr AssetType expectedType = GetAssetTypeFor<T>();
			AssetType actualType = it->second.asset->GetType();

			if (expectedType != AssetType::Unknown && actualType != expectedType)
			{
				// 타입 불일치 - nullptr 반환
				return nullptr;
			}
#endif
			return static_cast<T*>(it->second.asset.get());
		}
		return nullptr;
	}

	template<typename T>
	const T* AssetManager::GetAsset(ResourceId id) const
	{
		static_assert(std::is_base_of_v<IAsset, T>, "T must derive from IAsset");

		auto it = mAssetCache.find(id);
		if (it != mAssetCache.end() && it->second.asset)
		{
#ifdef _DEBUG
			// DEBUG 빌드에서 타입 검증
			constexpr AssetType expectedType = GetAssetTypeFor<T>();
			AssetType actualType = it->second.asset->GetType();

			if (expectedType != AssetType::Unknown && actualType != expectedType)
			{
				return nullptr;
			}
#endif
			return static_cast<const T*>(it->second.asset.get());
		}
		return nullptr;
	}

} // namespace Framework
