#include "pch.h"
#include "ECS/Registry.h"

namespace ECS
{
	Registry::Registry()
	{
	}

	Registry::~Registry()
	{
		// Component Storage는 unique_ptr로 자동 정리됨
	}

	Entity Registry::CreateEntity()
	{
		Entity entity;

		// 재활용 가능한 ID가 있으면 사용
		if (!mFreeIds.empty())
		{
			entity.id = mFreeIds.front();
			mFreeIds.pop();

			// 버전 증가 (Entity 재활용 감지용)
			entity.version = ++mVersions[entity.id];
		}
		else
		{
			// 새로운 ID 생성
			entity.id = mNextEntityId++;
			entity.version = 0;

			// 버전 테이블 확장
			mVersions.push_back(0);
		}

		mEntities.push_back(entity);
		return entity;
	}

	void Registry::DestroyEntity(Entity entity)
	{
		CORE_ASSERT(IsEntityValid(entity), "Invalid entity");

		// 모든 컴포넌트 제거
		for (auto& [typeIndex, storage] : mComponentStorages)
		{
			if (storage->HasComponent(entity.id))
			{
				storage->RemoveComponent(entity.id);
			}
		}

		// Entity 목록에서 제거
		auto it = std::find(mEntities.begin(), mEntities.end(), entity);
		if (it != mEntities.end())
		{
			mEntities.erase(it);
		}

		// ID를 재활용 풀에 추가
		mFreeIds.push(entity.id);
	}

	bool Registry::IsEntityValid(Entity entity) const
	{
		if (!entity.IsValid())
		{
			return false;
		}

		if (entity.id >= mVersions.size())
		{
			return false;
		}

		// 버전이 일치하는지 확인 (이미 삭제된 Entity인지 검증)
		return mVersions[entity.id] == entity.version;
	}

} // namespace ECS
