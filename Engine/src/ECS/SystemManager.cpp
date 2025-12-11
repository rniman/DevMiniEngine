#include "pch.h"
#include "ECS/SystemManager.h"
#include "ECS/Registry.h"

namespace ECS
{
	SystemManager::SystemManager(Registry& registry)
		: mRegistry(&registry)
	{
		LOG_INFO("[SystemManager] Created");
	}

	SystemManager::~SystemManager()
	{
		ShutdownAllSystems();
		LOG_INFO("[SystemManager] Destroyed");
	}

	void SystemManager::UpdateSystems(Core::float32 deltaTime)
	{
		for (auto& system : mSystems)
		{
			if (system->IsActive())
			{
				system->Update(deltaTime);
			}
		}
	}

	void SystemManager::ShutdownAllSystems()
	{
		// 역순으로 종료 (LIFO)
		for (auto it = mSystems.rbegin(); it != mSystems.rend(); ++it)
		{
			(*it)->Shutdown();
		}

		mSystems.clear();
		mSystemMap.clear();

		LOG_INFO("[SystemManager] All systems shutdown");
	}

} // namespace ECS
