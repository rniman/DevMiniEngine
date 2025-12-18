/**
 * @file TransformComponent.h
 * @brief Transform 컴포넌트 (순수 데이터)
 *
 * ECS 원칙: Components는 데이터만 포함
 * 모든 로직은 TransformSystem에서 처리
 *
 * @warning position/rotation/scale 직접 수정 금지!
 *          반드시 TransformSystem API 사용
 *          - TransformSystem::SetPosition()
 *          - TransformSystem::SetRotation()
 *          - TransformSystem::SetScale()
 *          직접 수정 시 dirty 플래그가 설정되지 않아 행렬이 갱신되지 않음
 */
#pragma once
#include "ECS/Entity.h"
#include "Math/MathTypes.h"

namespace ECS
{
	/**
	 * @brief Transform 컴포넌트 (순수 데이터)
	 *
	 * 위치, 회전, 스케일 정보와 캐시된 행렬을 저장합니다.
	 * Dirty Flag 패턴을 사용하여 불필요한 행렬 재계산을 방지합니다.
	 *
	 * @note 계층 구조가 필요한 경우 HierarchyComponent와 함께 사용
	 */
	struct TransformComponent
	{
		//=====================================================================
		// 입력 데이터 (Transform 정보)
		//=====================================================================

		Math::Vector3 position = Math::Vector3::Zero();
		Math::Quaternion rotation = Math::Quaternion::Identity();
		Math::Vector3 scale = Math::Vector3::One();

		//=====================================================================
		// 캐시된 행렬
		//=====================================================================

		/// Local 변환 행렬 (Scale * Rotation * Translation)
		Math::Matrix4x4 localMatrix = Math::Matrix4x4::Identity();

		/// World 변환 행렬 (Parent.worldMatrix * localMatrix)
		Math::Matrix4x4 worldMatrix = Math::Matrix4x4::Identity();

		//=====================================================================
		// Dirty Flags
		//=====================================================================

		/// Local 행렬 재계산 필요 여부 (position/rotation/scale 변경 시 true)
		bool localDirty = true;

		/// World 행렬 재계산 필요 여부 (local 변경 또는 parent 변경 시 true)
		bool worldDirty = true;

		//=====================================================================
		// TODO: [OPTIMIZATION] Phase 4+
		// - 현재: localMatrix + worldMatrix 캐시
		// - 최적화: worldInvTranspose 추가 캐시 (조명 병목 시)
		// - 최적화: 단일 dirty + 비트 플래그 (메모리 최적화 필요 시)
		//=====================================================================
	};

} // namespace ECS
