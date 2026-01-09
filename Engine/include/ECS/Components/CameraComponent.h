/**
 * @file CameraComponent.h
 * @brief 카메라 컴포넌트 (순수 데이터)
 *
 * ECS 원칙: Components는 데이터만 포함
 * 모든 로직은 CameraSystem에서 처리
 *
 * @note TransformComponent와 함께 사용되어야 View 행렬 계산 가능
 * @note 카메라는 씬에 1-2개이므로 매 프레임 행렬을 재계산합니다.
 */
#pragma once
#include "Core/Types.h"
#include "Math/MathTypes.h"

namespace ECS
{
	/**
	 * @brief 카메라 투영 타입
	 */
	enum class ProjectionType : Core::uint8
	{
		Perspective,   ///< 원근 투영
		Orthographic   ///< 직교 투영
	};

	/**
	 * @brief 카메라 Up 벡터 모드
	 *
	 * View 행렬 계산 시 up 벡터를 어떻게 결정할지 지정합니다.
	 */
	enum class CameraUpMode : Core::uint8
	{
		/// 월드 up 고정 (FPS, 3인칭 액션, RTS 등)
		/// worldUpReference 벡터를 그대로 사용
		WorldUp,

		/// 로컬 up 사용 (비행 시뮬, 우주 게임 등)
		/// localUp 벡터를 TransformComponent의 rotation으로 회전하여 사용
		LocalUp
	};

	/**
	 * @brief 카메라 컴포넌트 (순수 데이터)
	 *
	 * 카메라의 투영 방식, 시야각, 클리핑 평면 등의 파라미터와
	 * 캐시된 View/Projection 행렬을 저장합니다.
	 *
	 * 카메라는 씬에 1-2개이므로 매 프레임 행렬을 재계산합니다.
	 * (Dirty Flag 불필요)
	 */
	struct CameraComponent
	{
		//=====================================================================
		// 상태 플래그
		//=====================================================================

		ProjectionType projectionType = ProjectionType::Perspective;
		CameraUpMode upMode = CameraUpMode::WorldUp;
		bool isMainCamera = false;   ///< 주 렌더링 카메라 여부

		//=====================================================================
		// Clip Planes
		//=====================================================================

		Core::float32 nearPlane = 0.1f;
		Core::float32 farPlane = 1000.0f;

		//=====================================================================
		// Perspective 파라미터
		//=====================================================================

		Core::float32 fovY = 1.047f;               ///< 수직 시야각 (라디안, 기본값 60도)
		Core::float32 aspectRatio = 16.0f / 9.0f;  ///< 화면 종횡비

		//=====================================================================
		// Orthographic 파라미터
		//=====================================================================

		Core::float32 orthoWidth = 10.0f;
		Core::float32 orthoHeight = 10.0f;

		//=====================================================================
		// 카메라 방향 벡터
		//=====================================================================

		/// 로컬 전방 벡터 (TransformComponent rotation으로 회전됨)
		Math::Vector3 forward = Math::Vector3::Forward();

		/// WorldUp 모드에서 사용할 월드 기준 up 벡터
		/// 일반적으로 (0, 1, 0)이지만, 경사면이나 특수 상황에서 변경 가능
		Math::Vector3 worldUpReference = Math::Vector3::Up();

		/// LocalUp 모드에서 사용할 로컬 up 벡터
		/// TransformComponent rotation으로 회전되어 실제 up 방향 결정
		Math::Vector3 localUp = Math::Vector3::Up();

		//=====================================================================
		// 캐시된 행렬
		//=====================================================================

		Math::Matrix4x4 viewMatrix = Math::Matrix4x4::Identity();
		Math::Matrix4x4 projectionMatrix = Math::Matrix4x4::Identity();
	};

} // namespace ECS
