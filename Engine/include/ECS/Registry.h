// Engine/include/ECS/Registry.h
#pragma once
#include "Core/Types.h"
#include "ECS/Entity.h"
#include "ECS/Component.h"
#include "Core/Assert.h"
#include <vector>
#include <unordered_map>
#include <queue>
#include <memory>
#include <typeindex>

namespace ECS
{
	// 컴포넌트 저장소 인터페이스 (타입 소거)
	class IComponentStorage
	{
	public:
		virtual ~IComponentStorage() = default;
		virtual void RemoveComponent(Core::uint32 entityId) = 0;
		virtual bool HasComponent(Core::uint32 entityId) const = 0;
	};

	// 특정 타입의 컴포넌트 저장소
	template<typename T>
	class ComponentStorage : public IComponentStorage
	{
	public:
		ComponentStorage() = default;
		~ComponentStorage() override = default;

		// 컴포넌트 추가
		T* AddComponent(Core::uint32 entityId, const T& component)
		{
			mComponents[entityId] = component;
			return &mComponents[entityId];
		}

		// 컴포넌트 추가 (이동 의미론)
		T* AddComponent(Core::uint32 entityId, T&& component)
		{
			mComponents[entityId] = std::move(component);
			return &mComponents[entityId];
		}

		// 컴포넌트 제거
		void RemoveComponent(Core::uint32 entityId) override
		{
			mComponents.erase(entityId);
		}

		// 컴포넌트 조회
		T* GetComponent(Core::uint32 entityId)
		{
			auto it = mComponents.find(entityId);
			return (it != mComponents.end()) ? &it->second : nullptr;
		}

		const T* GetComponent(Core::uint32 entityId) const
		{
			auto it = mComponents.find(entityId);
			return (it != mComponents.end()) ? &it->second : nullptr;
		}

		// 컴포넌트 존재 여부
		bool HasComponent(Core::uint32 entityId) const override
		{
			return mComponents.find(entityId) != mComponents.end();
		}

		// 모든 컴포넌트 순회
		std::unordered_map<Core::uint32, T>& GetAllComponents()
		{
			return mComponents;
		}

		const std::unordered_map<Core::uint32, T>& GetAllComponents() const
		{
			return mComponents;
		}

	private:
		std::unordered_map<Core::uint32, T> mComponents;
	};

	// Registry: ECS의 중앙 관리자
	class Registry
	{
	public:
		Registry();
		~Registry();

		// Entity 생성 및 삭제
		Entity CreateEntity();
		void DestroyEntity(Entity entity);
		bool IsEntityValid(Entity entity) const;

		// Component 추가
		template<typename T>
		T* AddComponent(Entity entity, const T& component);

		template<typename T>
		T* AddComponent(Entity entity, T&& component);

		// Component 제거
		template<typename T>
		void RemoveComponent(Entity entity);

		// Component 조회
		template<typename T>
		T* GetComponent(Entity entity);

		template<typename T>
		const T* GetComponent(Entity entity) const;

		// Component 존재 확인
		template<typename T>
		bool HasComponent(Entity entity) const;

		// 모든 Entity 조회 (디버깅용)
		const std::vector<Entity>& GetAllEntities() const { return mEntities; }

		// 통계
		Core::uint32 GetEntityCount() const { return static_cast<Core::uint32>(mEntities.size()); }
		Core::uint32 GetRecycledCount() const { return static_cast<Core::uint32>(mFreeIds.size()); }

	private:
		// Entity 관리
		std::vector<Entity> mEntities;                     // 활성 Entity 목록
		std::queue<Core::uint32> mFreeIds;                 // 재활용 가능한 ID 풀
		std::vector<Core::uint32> mVersions;               // 각 ID의 현재 버전

		Core::uint32 mNextEntityId = 0;                    // 다음 할당할 ID

		// Component Storage 관리
		std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> mComponentStorages;

		// Component Storage 가져오기 (없으면 생성)
		template<typename T>
		ComponentStorage<T>* GetOrCreateStorage();

		template<typename T>
		ComponentStorage<T>* GetStorage();

		template<typename T>
		const ComponentStorage<T>* GetStorage() const;
	};

	// ============================================================================
	// Template 구현부
	// ============================================================================

	template<typename T>
	ComponentStorage<T>* Registry::GetOrCreateStorage()
	{
		std::type_index typeIndex(typeid(T));

		auto it = mComponentStorages.find(typeIndex);
		if (it == mComponentStorages.end())
		{
			auto storage = std::make_unique<ComponentStorage<T>>();
			auto* ptr = storage.get();
			mComponentStorages[typeIndex] = std::move(storage);
			return ptr;
		}

		return static_cast<ComponentStorage<T>*>(it->second.get());
	}

	template<typename T>
	ComponentStorage<T>* Registry::GetStorage()
	{
		std::type_index typeIndex(typeid(T));
		auto it = mComponentStorages.find(typeIndex);

		CORE_ASSERT(it != mComponentStorages.end(), "Component storage does not exist");
		return static_cast<ComponentStorage<T>*>(it->second.get());
	}

	template<typename T>
	const ComponentStorage<T>* Registry::GetStorage() const
	{
		std::type_index typeIndex(typeid(T));
		auto it = mComponentStorages.find(typeIndex);

		CORE_ASSERT(it != mComponentStorages.end(), "Component storage does not exist");
		return static_cast<const ComponentStorage<T>*>(it->second.get());
	}

	template<typename T>
	T* Registry::AddComponent(Entity entity, const T& component)
	{
		CORE_ASSERT(IsEntityValid(entity), "Invalid entity");

		auto* storage = GetOrCreateStorage<T>();
		return storage->AddComponent(entity.id, component);
	}

	template<typename T>
	T* Registry::AddComponent(Entity entity, T&& component)
	{
		CORE_ASSERT(IsEntityValid(entity), "Invalid entity");

		auto* storage = GetOrCreateStorage<T>();
		return storage->AddComponent(entity.id, std::move(component));
	}

	template<typename T>
	void Registry::RemoveComponent(Entity entity)
	{
		CORE_ASSERT(IsEntityValid(entity), "Invalid entity");

		auto* storage = GetStorage<T>();
		storage->RemoveComponent(entity.id);
	}

	template<typename T>
	T* Registry::GetComponent(Entity entity)
	{
		if (!IsEntityValid(entity))
		{
			return nullptr;
		}

		auto* storage = GetStorage<T>();
		return storage->GetComponent(entity.id);
	}

	template<typename T>
	const T* Registry::GetComponent(Entity entity) const
	{
		if (!IsEntityValid(entity))
		{
			return nullptr;
		}

		auto* storage = GetStorage<T>();
		return storage->GetComponent(entity.id);
	}

	template<typename T>
	bool Registry::HasComponent(Entity entity) const
	{
		if (!IsEntityValid(entity))
		{
			return false;
		}

		std::type_index typeIndex(typeid(T));
		auto it = mComponentStorages.find(typeIndex);

		if (it == mComponentStorages.end())
		{
			return false;
		}

		return it->second->HasComponent(entity.id);
	}

} // namespace ECS
