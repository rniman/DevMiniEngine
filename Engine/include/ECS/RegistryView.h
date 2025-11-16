#pragma once
#include "ECS/Registry.h"
#include <vector>

namespace ECS
{
	/**
	 * @brief Component 조합 쿼리를 위한 View 클래스
	 *
	 * 여러 Component를 동시에 가진 Entity만 순회합니다.
	 *
	 * @example
	 * auto view = registry.RegistryView<TransformComponent, MeshComponent>();
	 * for (Entity entity : view)
	 * {
	 *     auto* transform = registry.GetComponent<TransformComponent>(entity);
	 *     auto* mesh = registry.GetComponent<MeshComponent>(entity);
	 *     // ...
	 * }
	 */
	template<typename... Components>
	class RegistryView
	{
	public:
		RegistryView(Registry* registry)
			: mRegistry(registry)
		{
			CollectEntities();
		}

		// Iterator 인터페이스
		std::vector<Entity>::iterator begin() { return mEntities.begin(); }
		std::vector<Entity>::iterator end() { return mEntities.end(); }
		std::vector<Entity>::const_iterator begin() const { return mEntities.begin(); }
		std::vector<Entity>::const_iterator end() const { return mEntities.end(); }

		// 결과 개수
		size_t size() const { return mEntities.size(); }
		bool empty() const { return mEntities.empty(); }

	private:
		void CollectEntities()
		{
			// 모든 활성 Entity를 순회
			for (const Entity& entity : mRegistry->GetAllEntities())
			{
				// 모든 Component를 가지고 있는지 확인
				if (HasAllComponents(entity))
				{
					mEntities.push_back(entity);
				}
			}
		}

		// 가변 템플릿으로 모든 Component 존재 확인
		bool HasAllComponents(Entity entity) const
		{
			return (mRegistry->HasComponent<Components>(entity) && ...);
		}

		Registry* mRegistry;
		std::vector<Entity> mEntities;
	};

	// Registry에 RegistryView 생성 함수 추가
	template<typename... Components>
	RegistryView<Components...> Registry::CreateView()
	{
		return RegistryView<Components...>(this);
	}

} // namespace ECS
