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
		Perspective,   // 원근 투영
		Orthographic   // 직교 투영
	};

	/**
	 * @brief 카메라 컴포넌트 (순수 데이터)
	 *
	 * ECS 원칙: Components는 데이터만 포함
	 * 모든 로직은 CameraSystem에서 처리
	 *
	 * @note TransformComponent와 함께 사용되어야 View 행렬 계산 가능
	 */
	struct CameraComponent
	{
		//=====================================================================
		// 상태 플래그 (1 byte씩, 패딩 최적화)
		//=====================================================================

		ProjectionType projectionType = ProjectionType::Perspective;
		bool viewDirty = true;       // View 행렬 재계산 필요 여부
		bool projectionDirty = true; // Projection 행렬 재계산 필요 여부
		bool isMainCamera = false;   // 주 렌더링 카메라 여부

		//=====================================================================
		// Clip Planes
		//=====================================================================

		Core::float32 nearPlane = 0.1f;
		Core::float32 farPlane = 1000.0f;

		//=====================================================================
		// Perspective 파라미터
		//=====================================================================

		Core::float32 fovY = 1.047f;               // 수직 시야각 (라디안, 기본값 60도)
		Core::float32 aspectRatio = 16.0f / 9.0f;  // 화면 종횡비

		//=====================================================================
		// Orthographic 파라미터
		//=====================================================================

		Core::float32 orthoWidth = 10.0f;
		Core::float32 orthoHeight = 10.0f;

		//=====================================================================
		// 카메라 방향 벡터 (로컬 기준)
		//=====================================================================

		Math::Vector3 forward = Math::Vector3::Forward();  // 로컬 Z+
		Math::Vector3 up = Math::Vector3::Up();            // 로컬 Y+

		//=====================================================================
		// 캐시된 행렬 (64 bytes each)
		//=====================================================================

		Math::Matrix4x4 viewMatrix = Math::Matrix4x4::Identity();
		Math::Matrix4x4 projectionMatrix = Math::Matrix4x4::Identity();
	};

} // namespace ECS
