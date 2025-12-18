/**
 * @file HierarchyComponent.h
 * @brief Entity 계층 구조를 저장하는 Component
 *
 * ECS 원칙: Component는 순수 데이터만 포함
 * 계층 관련 로직은 TransformSystem에서 처리
 *
 * @note Phase 3.5에서 추가
 */
#pragma once
#include "ECS/Entity.h"
#include <vector>

namespace ECS
{
	/**
	 * @brief 계층 구조 컴포넌트 (순수 데이터)
	 *
	 * Entity 간의 부모-자식 관계를 저장합니다.
	 * Transform 계층 구조를 위해 TransformComponent와 함께 사용됩니다.
	 *
	 * @warning position/rotation/scale 변경은 반드시 TransformSystem API 사용
	 *          직접 수정 시 dirty 플래그가 설정되지 않음
	 *
	 * @example
	 * // 계층 구조 설정
	 * registry.AddComponent<HierarchyComponent>(childEntity);
	 * registry.AddComponent<HierarchyComponent>(parentEntity);
	 * transformSystem.SetParent(childEntity, parentEntity);
	 */
	struct HierarchyComponent
	{
		/// 부모 Entity (Invalid면 Root)
		Entity parent = Entity::Invalid();

		/// 자식 Entity 목록 (순회 최적화를 위해 vector 사용)
		std::vector<Entity> children;

		//=====================================================================
		// TODO: [OPTIMIZATION] Phase 4+
		// - 현재: std::vector<Entity> children
		// - 최적화: firstChild + nextSibling 연결 리스트 (메모리 제약 시)
		//=====================================================================
	};

} // namespace ECS
