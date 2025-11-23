#pragma once
#include "Core/Types.h"
#include "Math/MathTypes.h"
#include "Math/MathUtils.h"

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
		// [메모리 레이아웃 최적화] 1바이트 타입들을 묶어서 패딩 제거
		ProjectionType projectionType = ProjectionType::Perspective;
		bool viewDirty = true;       // View 행렬 재계산 필요 여부
		bool projectionDirty = true; // Projection 행렬 재계산 필요 여부
		bool isMainCamera = false;   // 주 렌더링 카메라 여부

		Core::float32 nearPlane = 0.1f;   // 근평면 거리
		Core::float32 farPlane = 1000.0f; // 원평면 거리

		Core::float32 fovY = 1.047f;               // 수직 시야각 (라디안, 기본값 60도)
		Core::float32 aspectRatio = 16.0f / 9.0f;  // 화면 종횡비

		Core::float32 orthoWidth = 10.0f;   // 직교 투영 너비
		Core::float32 orthoHeight = 10.0f;  // 직교 투영 높이

		// 계산된 행렬 (캐시, 64 bytes each)
		// 참고: Component 크기가 권장(64B)을 초과하지만, 카메라는 개체 수가 적으므로 허용
		Math::Matrix4x4 viewMatrix = Math::MatrixIdentity();
		Math::Matrix4x4 projectionMatrix = Math::MatrixIdentity();
	};

} // namespace ECS
