#pragma once
#include "Core/Types.h"

namespace ECS
{
	class Registry;

	/**
	 * @brief ECS System의 기본 인터페이스
	 *
	 * 모든 System은 이 인터페이스를 구현해야 합니다.
	 * System은 Component 데이터를 읽고 처리하는 로직을 담당합니다.
	 */
	class ISystem
	{
	public:
		virtual ~ISystem() = default;

		/**
		 * @brief System 초기화
		 *
		 * SystemManager에 등록된 직후 한 번 호출됩니다.
		 * 필요한 리소스 할당이나 초기 설정을 수행합니다.
		 *
		 * @param registry ECS Registry 참조
		 */
		virtual void OnInitialize(Registry& registry) {}

		/**
		 * @brief 매 프레임 업데이트
		 *
		 * SystemManager::Update()에서 호출됩니다.
		 * Component 데이터를 읽고 처리하는 메인 로직입니다.
		 *
		 * @param registry ECS Registry 참조
		 * @param deltaTime 이전 프레임으로부터 경과 시간 (초)
		 */
		virtual void OnUpdate(Registry& registry, Core::float32 deltaTime) = 0;

		/**
		 * @brief System 종료
		 *
		 * SystemManager가 파괴되기 전에 호출됩니다.
		 * 할당된 리소스를 정리합니다.
		 *
		 * @param registry ECS Registry 참조
		 */
		virtual void OnShutdown(Registry& registry) {}

		/**
		 * @brief System 활성화 상태 설정
		 *
		 * 비활성화된 System은 OnUpdate가 호출되지 않습니다.
		 *
		 * @param active 활성화 여부
		 */
		void SetActive(bool active) { mIsActive = active; }

		/**
		 * @brief System 활성화 상태 확인
		 *
		 * @return true면 활성화, false면 비활성화
		 */
		bool IsActive() const { return mIsActive; }

	protected:
		bool mIsActive = true;
	};

} // namespace ECS
