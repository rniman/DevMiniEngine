/**
 * @file ISystem.h
 * @brief ECS System의 기본 인터페이스
 *
 * 모든 System은 이 인터페이스를 상속해야 합니다.
 * 생성자 주입 방식으로 Registry를 받아 안전성을 보장합니다.
 */
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
	 *
	 * 생성자에서 Registry를 주입받아 멤버 변수로 저장합니다.
	 * - nullptr 상태 불가능 (생성 시점에 유효성 보장)
	 * - 불변성 (런타임에 Registry 변경 불가)
	 * - 다중 Registry 지원 (World마다 독립된 System 인스턴스)
	 */
	class ISystem
	{
	public:
		/**
		 * @brief 생성자 (Registry 주입)
		 * @param registry 이 System이 속한 Registry
		 */
		explicit ISystem(Registry& registry)
			: mRegistry(&registry)
		{
		}

		virtual ~ISystem() = default;

		// 복사 금지
		ISystem(const ISystem&) = delete;
		ISystem& operator=(const ISystem&) = delete;

		// 이동 허용
		ISystem(ISystem&&) = default;
		ISystem& operator=(ISystem&&) = default;

		/**
		 * @brief System 초기화
		 *
		 * SystemManager에 등록된 직후 한 번 호출됩니다.
		 * 필요한 리소스 할당이나 초기 설정을 수행합니다.
		 */
		virtual void Initialize() {}

		/**
		 * @brief 매 프레임 업데이트
		 *
		 * SystemManager::UpdateSystems()에서 호출됩니다.
		 * Component 데이터를 읽고 처리하는 메인 로직입니다.
		 *
		 * @param deltaTime 이전 프레임으로부터 경과 시간 (초)
		 */
		virtual void Update(Core::float32 deltaTime) = 0;

		/**
		 * @brief System 종료
		 *
		 * SystemManager가 파괴되기 전에 호출됩니다.
		 * 할당된 리소스를 정리합니다.
		 */
		virtual void Shutdown() {}

		/**
		 * @brief System 활성화 상태 설정
		 *
		 * 비활성화된 System은 Update가 호출되지 않습니다.
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
		/**
		 * @brief Registry 접근자
		 * @return 이 System이 속한 Registry
		 */
		Registry* GetRegistry() { return mRegistry; }
		const Registry* GetRegistry() const { return mRegistry; }

	private:
		Registry* mRegistry;    // 항상 유효 (생성자에서 보장)
		bool mIsActive = true;
	};

} // namespace ECS
