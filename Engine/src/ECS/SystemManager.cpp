#include "pch.h"
#include "ECS/SystemManager.h"
#include "ECS/Registry.h"

namespace ECS
{
	SystemManager::SystemManager(Registry* registry)
		: mRegistry(registry)
	{
		CORE_ASSERT(registry != nullptr, "Registry cannot be null");
		LOG_INFO("[SystemManager] SystemManager created");
	}

	SystemManager::~SystemManager()
	{
		ShutdownAllSystems();
		LOG_INFO("[SystemManager] SystemManager destroyed");
	}

	void SystemManager::UpdateSystems(Core::float32 deltaTime)
	{
		for (auto& system : mSystems)
		{
			if (system->IsActive())
			{
				system->Update(*mRegistry, deltaTime);
			}
		}
	}

	void SystemManager::ShutdownAllSystems()
	{
		// 역순으로 종료 (LIFO)
		for (auto it = mSystems.rbegin(); it != mSystems.rend(); ++it)
		{
			(*it)->Shutdown(*mRegistry);
		}

		mSystems.clear();
		mSystemMap.clear();

		LOG_INFO("[SystemManager] All systems shutdown");
	}

} // namespace ECS
