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
	 */
	class SystemManager
	{
	public:
		SystemManager(Registry* registry);
		~SystemManager();

		/**
		 * @brief System을 등록하고 초기화
		 *
		 * @tparam T System 타입 (ISystem을 상속해야 함)
		 * @tparam Args 생성자 인자 타입들
		 * @param args System 생성자에 전달할 인자들
		 * @return 등록된 System 포인터 (nullptr이면 이미 등록됨)
		 */
		template<typename T, typename... Args>
		T* RegisterSystem(Args&&... args);

		/**
		 * @brief 모든 활성 System 업데이트
		 *
		 * 등록 순서대로 각 System의 OnUpdate를 호출합니다.
		 *
		 * @param deltaTime 이전 프레임으로부터 경과 시간 (초)
		 */
		void UpdateSystems(Core::float32 deltaTime);

		/**
		 * @brief 모든 System 종료 및 제거
		 *
		 * 각 System의 OnShutdown을 호출하고 메모리를 해제합니다.
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
		// is_base_of_v<ISystem, T> == is_base_of<ISystem, T>::value
		static_assert(std::is_base_of_v<ISystem, T>, "T must inherit from ISystem");

		std::type_index typeIndex(typeid(T));

		// 이미 등록된 System인지 확인
		if (mSystemMap.find(typeIndex) != mSystemMap.end())
		{
			LOG_WARN("[SystemManager] System already registered: %s", typeid(T).name());
			return nullptr;
		}

		// System 생성
		auto system = std::make_unique<T>(std::forward<Args>(args)...);
		T* systemPtr = system.get();

		// 초기화
		systemPtr->Initialize(*mRegistry);

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
