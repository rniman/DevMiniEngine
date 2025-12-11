/**
 * @file SystemManager.h
 * @brief ECS System들의 생명주기를 관리하는 매니저
 *
 * System 등록 시 Registry를 자동으로 주입합니다.
 */
#pragma once
#include "ECS/ISystem.h"

#include "Core/Assert.h"
#include "Core/Logging/LogMacros.h"
#include "Core/Types.h"

#include <memory>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace ECS
{
	class Registry;

	/**
	 * @brief ECS System들의 생명주기를 관리하는 매니저
	 *
	 * System을 등록하고, 실행 순서대로 Update를 호출합니다.
	 * 각 System은 한 번만 등록 가능합니다.
	 *
	 * 생성자 주입 방식:
	 * - RegisterSystem<T>()가 내부적으로 Registry를 첫 번째 인자로 전달
	 * - System은 생성 시점에 유효한 Registry를 보장받음
	 */
	class SystemManager
	{
	public:
		/**
		 * @brief 생성자
		 * @param registry 이 SystemManager가 관리할 Registry
		 */
		explicit SystemManager(Registry& registry);
		~SystemManager();

		// 복사 금지
		SystemManager(const SystemManager&) = delete;
		SystemManager& operator=(const SystemManager&) = delete;

		/**
		 * @brief System을 등록하고 초기화
		 *
		 * Registry는 자동으로 첫 번째 인자로 전달됩니다.
		 *
		 * @tparam T System 타입 (ISystem을 상속해야 함)
		 * @tparam Args 추가 생성자 인자 타입들
		 * @param args System 생성자에 전달할 추가 인자들
		 * @return 등록된 System 포인터 (nullptr이면 이미 등록됨)
		 *
		 * @example
		 * // Registry만 필요한 System
		 * manager.RegisterSystem<TransformSystem>();
		 *
		 * // 추가 인자가 필요한 System
		 * manager.RegisterSystem<RenderSystem>(resourceManager);
		 */
		template<typename T, typename... Args>
		T* RegisterSystem(Args&&... args);

		/**
		 * @brief 모든 활성 System 업데이트
		 *
		 * 등록 순서대로 각 System의 Update를 호출합니다.
		 *
		 * @param deltaTime 이전 프레임으로부터 경과 시간 (초)
		 */
		void UpdateSystems(Core::float32 deltaTime);

		/**
		 * @brief 모든 System 종료 및 제거
		 *
		 * 각 System의 Shutdown을 호출하고 메모리를 해제합니다.
		 * 역순(LIFO)으로 종료합니다.
		 */
		void ShutdownAllSystems();

		/**
		 * @brief 등록된 System 가져오기
		 *
		 * @tparam T System 타입
		 * @return System 포인터 (등록되지 않았으면 nullptr)
		 */
		template<typename T>
		T* GetSystem();

		/**
		 * @brief System이 등록되어 있는지 확인
		 *
		 * @tparam T System 타입
		 * @return true면 등록됨, false면 미등록
		 */
		template<typename T>
		bool HasSystem() const;

		/**
		 * @brief 등록된 System 개수
		 */
		size_t GetSystemCount() const { return mSystems.size(); }

	private:
		Registry* mRegistry;

		// System 저장 (등록 순서 유지)
		std::vector<std::unique_ptr<ISystem>> mSystems;

		// 타입별 빠른 조회용 맵
		std::unordered_map<std::type_index, ISystem*> mSystemMap;
	};

	// ============================================================================
	// Template 구현부
	// ============================================================================

	template<typename T, typename... Args>
	T* SystemManager::RegisterSystem(Args&&... args)
	{
		static_assert(std::is_base_of_v<ISystem, T>, "T must inherit from ISystem");

		std::type_index typeIndex(typeid(T));

		// 이미 등록된 System인지 확인
		if (mSystemMap.find(typeIndex) != mSystemMap.end())
		{
			LOG_WARN("[SystemManager] System already registered: %s", typeid(T).name());
			return nullptr;
		}

		// System 생성 (Registry를 첫 번째 인자로 자동 전달)
		auto system = std::make_unique<T>(*mRegistry, std::forward<Args>(args)...);
		T* systemPtr = system.get();

		// 초기화 (파라미터 없음)
		systemPtr->Initialize();

		// 저장
		mSystems.push_back(std::move(system));
		mSystemMap[typeIndex] = systemPtr;

		LOG_INFO("[SystemManager] System registered: %s", typeid(T).name());

		return systemPtr;
	}

	template<typename T>
	T* SystemManager::GetSystem()
	{
		static_assert(std::is_base_of_v<ISystem, T>, "T must inherit from ISystem");

		std::type_index typeIndex(typeid(T));
		auto it = mSystemMap.find(typeIndex);

		if (it == mSystemMap.end())
		{
			return nullptr;
		}

		return static_cast<T*>(it->second);
	}

	template<typename T>
	bool SystemManager::HasSystem() const
	{
		static_assert(std::is_base_of_v<ISystem, T>, "T must inherit from ISystem");

		std::type_index typeIndex(typeid(T));
		return mSystemMap.find(typeIndex) != mSystemMap.end();
	}

} // namespace ECS
