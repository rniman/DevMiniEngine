/**
 * @file AssetHandle.h
 * @brief Asset에 대한 참조 카운팅 기반 스마트 핸들
 *
 * AssetManager가 소유한 Asset에 대한 안전한 참조를 제공합니다.
 * shared_ptr과 유사하지만 AssetManager의 캐시 시스템과 통합됩니다.
 */
#pragma once
#include "Framework/Assets/AssetTypes.h"
#include "Core/Types.h"

namespace Framework
{
	// 전방 선언
	class AssetManager;

	/**
	 * @brief Asset 참조 카운팅 스마트 핸들
	 *
	 * AssetManager가 관리하는 Asset에 대한 참조를 보유합니다.
	 * 복사 시 참조 카운트가 증가하고, 소멸 시 감소합니다.
	 *
	 * @tparam T Asset 타입 (MeshAsset, TextureAsset 등)
	 *
	 * @note Asset의 실제 소유권은 AssetManager에 있습니다.
	 *       AssetHandle은 참조만 보유합니다.
	 *
	 * @example
	 * AssetHandle<MeshAsset> mesh = assetManager.Load<MeshAsset>("Mesh.gltf");
	 * if (mesh.IsLoaded())
	 * {
	 *     mesh->GetVertexCount();
	 * }
	 */
	template<typename T>
	class AssetHandle
	{
	public:
		//=========================================================================
		// 생성자/소멸자
		//=========================================================================

		/** @brief 기본 생성자 (빈 핸들) */
		AssetHandle()
			: mAsset(nullptr)
			, mRefCount(nullptr)
		{
		}

		/** @brief nullptr 생성자 */
		AssetHandle(std::nullptr_t)
			: mAsset(nullptr)
			, mRefCount(nullptr)
		{
		}

		/** @brief 소멸자 - 참조 카운트 감소 */
		~AssetHandle()
		{
			Release();
		}

		//=========================================================================
		// 복사 (참조 카운트 증가)
		//=========================================================================

		/** @brief 복사 생성자 */
		AssetHandle(const AssetHandle& other)
			: mAsset(other.mAsset)
			, mRefCount(other.mRefCount)
		{
			AddRef();
		}

		/** @brief 복사 대입 연산자 */
		AssetHandle& operator=(const AssetHandle& other)
		{
			if (this != &other)
			{
				Release();
				mAsset = other.mAsset;
				mRefCount = other.mRefCount;
				AddRef();
			}
			return *this;
		}

		//=========================================================================
		// 이동 (참조 카운트 변경 없음)
		//=========================================================================

		/** @brief 이동 생성자 */
		AssetHandle(AssetHandle&& other) noexcept
			: mAsset(other.mAsset)
			, mRefCount(other.mRefCount)
		{
			other.mAsset = nullptr;
			other.mRefCount = nullptr;
		}

		/** @brief 이동 대입 연산자 */
		AssetHandle& operator=(AssetHandle&& other) noexcept
		{
			if (this != &other)
			{
				Release();
				mAsset = other.mAsset;
				mRefCount = other.mRefCount;
				other.mAsset = nullptr;
				other.mRefCount = nullptr;
			}
			return *this;
		}

		//=========================================================================
		// nullptr 대입
		//=========================================================================

		/** @brief nullptr 대입 연산자 */
		AssetHandle& operator=(std::nullptr_t)
		{
			Release();
			return *this;
		}

		//=========================================================================
		// 접근자
		//=========================================================================

		/** @brief Asset 포인터 반환 */
		T* Get() const { return mAsset; }

		/** @brief 역참조 연산자 */
		T& operator*() const { return *mAsset; }

		/** @brief 멤버 접근 연산자 */
		T* operator->() const { return mAsset; }

		//=========================================================================
		// 상태 확인
		//=========================================================================

		/** @brief 유효한 Asset을 가리키는지 확인 */
		bool IsValid() const { return mAsset != nullptr; }

		/** @brief bool 변환 연산자 */
		explicit operator bool() const { return IsValid(); }

		/**
		 * @brief Asset 로딩 상태 반환
		 * @return Asset 상태 (무효하면 Unloaded)
		 */
		AssetState GetState() const
		{
			if (mAsset)
			{
				return mAsset->GetState();
			}
			return AssetState::Unloaded;
		}

		/** @brief Asset이 로드 완료되었는지 확인 */
		bool IsLoaded() const { return GetState() == AssetState::Loaded; }

		/** @brief Asset이 로딩 중인지 확인 */
		bool IsLoading() const
		{
			AssetState state = GetState();
			return state == AssetState::Queued || state == AssetState::Loading;
		}

		/** @brief Asset 로드가 실패했는지 확인 */
		bool IsFailed() const { return GetState() == AssetState::Failed; }

		/**
		 * @brief 현재 참조 카운트 반환 (디버그용)
		 * @return 참조 카운트 (무효하면 0)
		 */
		Core::uint32 GetRefCount() const
		{
			if (mRefCount)
			{
				return *mRefCount;
			}
			return 0;
		}

		//=========================================================================
		// 비교 연산자
		//=========================================================================

		bool operator==(const AssetHandle& other) const { return mAsset == other.mAsset; }
		bool operator!=(const AssetHandle& other) const { return mAsset != other.mAsset; }
		bool operator==(std::nullptr_t) const { return mAsset == nullptr; }
		bool operator!=(std::nullptr_t) const { return mAsset != nullptr; }

		//=========================================================================
		// 해시 지원 (unordered_map 키로 사용 가능)
		//=========================================================================

		/** @brief 해시값 반환 */
		Core::size_t Hash() const
		{
			return std::hash<T*>()(mAsset);
		}

	private:
		friend class AssetManager;

		/**
		 * @brief AssetManager 전용 생성자
		 * @param asset Asset 포인터
		 * @param refCount AssetEntry의 참조 카운트 포인터
		 */
		AssetHandle(T* asset, Core::uint32* refCount)
			: mAsset(asset)
			, mRefCount(refCount)
		{
			AddRef();
		}

		/** @brief 참조 카운트 증가 */
		void AddRef()
		{
			if (mRefCount)
			{
				++(*mRefCount);
			}
		}

		/** @brief 참조 해제 (카운트 감소) */
		void Release()
		{
			if (mRefCount)
			{
				--(*mRefCount);
			}
			mAsset = nullptr;
			mRefCount = nullptr;
		}

		T* mAsset;					// Asset 포인터 (AssetManager 소유)
		Core::uint32* mRefCount;	// AssetEntry의 refCount 포인터
	};

} // namespace Framework

//=============================================================================
// std::hash 특수화 (unordered_map/set 지원)
//=============================================================================
namespace std
{
	template<typename T>
	struct hash<Framework::AssetHandle<T>>
	{
		size_t operator()(const Framework::AssetHandle<T>& handle) const
		{
			return handle.Hash();
		}
	};
}
