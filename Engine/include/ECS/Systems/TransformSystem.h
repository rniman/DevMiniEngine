#pragma once
#include "ECS/Components/TransformComponent.h"
#include "Core/Types.h"

namespace ECS
{
	/**
	 * @brief Transform 관련 모든 로직을 처리하는 시스템
	 *
	 * Phase 3 초기: 정적 유틸리티 함수만 제공
	 * Phase 3+: ISystem 상속하여 계층 구조 업데이트 처리
	 */
	class TransformSystem
	{
	public:
		//=====================================================================
		// Euler Angle - Quaternion 변환 (사용자 편의 함수)
		//=====================================================================

		/**
		 * @brief Euler Angle로 Rotation 설정
		 * @param transform Transform 컴포넌트
		 * @param eulerAngles Euler Angle (Radian)
		 */
		static void SetRotationEuler(TransformComponent& transform, const Math::Vector3& eulerAngles);

		/**
		 * @brief Euler Angle로 Rotation 설정
		 * @param transform Transform 컴포넌트
		 * @param pitch X축 회전 (Radian)
		 * @param yaw Y축 회전 (Radian)
		 * @param roll Z축 회전 (Radian)
		 */
		static void SetRotationEuler(
			TransformComponent& transform,
			Core::float32 pitch,
			Core::float32 yaw,
			Core::float32 roll
		);

		/**
		 * @brief Euler Angle로 Rotation 조회
		 * @param transform Transform 컴포넌트
		 * @return Euler Angle (Radian)
		 */
		static Math::Vector3 GetRotationEuler(const TransformComponent& transform);

		//=====================================================================
		// Transform 조작
		//=====================================================================

		/**
		 * @brief Euler Angle만큼 회전 추가
		 * @param transform Transform 컴포넌트
		 * @param eulerDelta 추가 회전량 (Radian)
		 */
		static void Rotate(TransformComponent& transform, const Math::Vector3& eulerDelta);

		/**
		 * @brief 특정 축 기준 회전
		 * @param transform Transform 컴포넌트
		 * @param axis 회전축 (정규화 필요 없음)
		 * @param angle 회전 각도 (Radian)
		 */
		static void RotateAround(
			TransformComponent& transform,
			const Math::Vector3& axis,
			Core::float32 angle
		);

		//=====================================================================
		// Matrix 계산
		//=====================================================================

		/**
		 * @brief Local Transform을 World Matrix로 변환
		 * @param transform Transform 컴포넌트
		 * @return World Matrix (4x4)
		 *
		 * @note S * R * T 순서로 변환 (Scale -> Rotation -> Translation)
		 */
		static Math::Matrix4x4 GetLocalMatrix(const TransformComponent& transform);

		// Phase 3+에서 추가 예정
		// static Math::Matrix4x4 GetWorldMatrix(Registry& registry, Entity entity);
		// static void MarkDirty(Registry& registry, Entity entity);
		// static void MarkDirtyRecursive(Registry& registry, Entity entity);
		// static void UpdateWorldMatrices(Registry& registry);
	};

} // namespace ECS
