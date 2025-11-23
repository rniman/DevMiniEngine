#pragma once
#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Entity.h"
#include "Core/Types.h"
#include "Math/MathTypes.h"

namespace ECS
{
	// 전방 선언
	class Registry;

	/**
	 * @brief 카메라 시스템 (정적 함수 집합)
	 *
	 * ECS 원칙: Systems는 순수 로직만 포함
	 * Component 데이터를 받아서 처리하는 정적 함수들
	 */
	class CameraSystem
	{
	public:
		//=============================================================================
		// 행렬 업데이트
		//=============================================================================

		/**
		 * @brief View 행렬 업데이트 (TransformComponent 기반)
		 * @param transform Entity의 Transform Component
		 * @param camera 업데이트할 Camera Component
		 * @note Dirty Flag가 true일 때만 재계산
		 */
		static void UpdateViewMatrix(
			const TransformComponent& transform,
			CameraComponent& camera
		);

		/**
		 * @brief Projection 행렬 업데이트
		 * @param camera 업데이트할 Camera Component
		 * @note Dirty Flag가 true일 때만 재계산
		 */
		static void UpdateProjectionMatrix(CameraComponent& camera);

		/**
		 * @brief Registry의 모든 카메라 업데이트
		 * @param registry ECS Registry
		 * @note Transform + Camera를 가진 모든 Entity 순회
		 */
		static void UpdateAllCameras(Registry& registry);

		//=============================================================================
		// 카메라 조회
		//=============================================================================

		/**
		 * @brief Main Camera Entity 찾기
		 * @param registry ECS Registry
		 * @return Main Camera Entity (없으면 Invalid Entity)
		 */
		static Entity FindMainCamera(Registry& registry);

		//=============================================================================
		// Helper 함수
		//=============================================================================

		/**
		 * @brief FOV 설정 (도 단위)
		 * @param camera 설정할 Camera Component
		 * @param degrees 시야각 (도 단위, 0~180)
		 * @note projectionDirty 플래그를 true로 설정
		 */
		static void SetFovYDegrees(CameraComponent& camera, Core::float32 degrees);

		/**
		 * @brief FOV 설정 (라디안 단위)
		 * @param camera 설정할 Camera Component
		 * @param radians 시야각 (라디안, 0~π)
		 * @note projectionDirty 플래그를 true로 설정
		 */
		static void SetFovYRadians(CameraComponent& camera, Core::float32 radians);

		/**
		 * @brief Aspect Ratio 설정
		 * @param camera 설정할 Camera Component
		 * @param aspectRatio 종횡비 (width / height)
		 * @note projectionDirty 플래그를 true로 설정
		 */
		static void SetAspectRatio(CameraComponent& camera, Core::float32 aspectRatio);

		/**
		 * @brief Aspect Ratio 설정 (너비, 높이)
		 * @param camera 설정할 Camera Component
		 * @param width 화면 너비
		 * @param height 화면 높이
		 * @note projectionDirty 플래그를 true로 설정
		 */
		static void SetAspectRatio(
			CameraComponent& camera,
			Core::float32 width,
			Core::float32 height
		);

		/**
		 * @brief Clip Planes 설정
		 * @param camera 설정할 Camera Component
		 * @param nearPlane 근평면 거리
		 * @param farPlane 원평면 거리
		 * @note projectionDirty 플래그를 true로 설정
		 */
		static void SetClipPlanes(
			CameraComponent& camera,
			Core::float32 nearPlane,
			Core::float32 farPlane
		);

		/**
		 * @brief Look At 설정 (Transform에 적용)
		 * @param transform 설정할 Transform Component
		 * @param position 카메라 위치
		 * @param target 바라볼 타겟 위치
		 * @param up Up 벡터 (기본값: Y-up)
		 * @note Transform의 position과 rotation을 설정하여 viewDirty 플래그가 true로 설정됨
		 */
		static void SetLookAt(
			TransformComponent& transform,
			const Math::Vector3& position,
			const Math::Vector3& target,
			const Math::Vector3& up = Math::Vector3(0.0f, 1.0f, 0.0f)
		);

		/**
		 * @brief Main Camera 플래그 설정
		 * @param registry ECS Registry
		 * @param entity Main Camera로 설정할 Entity
		 * @note 기존 Main Camera는 자동으로 해제됨
		 */
		static void SetMainCamera(Registry& registry, Entity entity);

	private:
		// 정적 클래스이므로 생성자 삭제
		CameraSystem() = delete;
		~CameraSystem() = delete;
	};

} // namespace ECS
