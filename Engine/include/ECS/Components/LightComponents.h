#pragma once
#include "Core/Types.h"
#include "Math/MathTypes.h"

/**
 * @file LightComponents.h
 * @brief ECS 조명 Component 정의 (Phase 3.3: Phong Shading)
 */
namespace ECS
{
	// Shader Constant Buffer 크기와 일치 (변경 시 Shader도 수정 필요)
	constexpr Core::uint32 MAX_DIRECTIONAL_LIGHTS = 4;
	constexpr Core::uint32 MAX_POINT_LIGHTS = 8;

	/**
	 * @brief Directional Light (태양광, 달빛)
	 *
	 * 무한히 먼 거리에서 모든 픽셀에 동일한 방향으로 빛이 도달합니다.
	 * Transform Component 불필요 (direction 필드로 방향 관리)
	 */
	struct DirectionalLightComponent
	{
		Math::Vector3 direction = Math::Vector3::Down();   // 정규화된 방향 벡터
		Math::Vector3 color = Math::Vector3(1.0f, 1.0f, 1.0f);        // RGB (0~1)
		Core::float32 intensity = 1.0f;                               // 조명 강도
		bool castsShadow = false;                                     // Phase 6: Shadow Map
		bool isDirty = true;                                          // Phase 3.4: ImGui 통합 대비
	};

	/**
	 * @brief Point Light (전구, 횃불, 촛불)
	 *
	 * 특정 위치에서 모든 방향으로 빛을 발산, 거리에 따라 감쇠됩니다.
	 * TransformComponent의 position 사용 (반드시 함께 사용)
	 *
	 * Attenuation = 1.0 / (constant + linear * d + quadratic * d^2)
	 */
	struct PointLightComponent
	{
		Math::Vector3 color = Math::Vector3(1.0f, 1.0f, 1.0f);	 // RGB (0~1)
		Core::float32 intensity = 1.0f;                          // 조명 강도
		Core::float32 range = 10.0f;                             // 최대 영향 범위

		// Attenuation 파라미터
		Core::float32 constant = 1.0f;                           // Kc
		Core::float32 linear = 0.09f;                            // Kl (10 단위 거리 기준)
		Core::float32 quadratic = 0.032f;                        // Kq

		bool isDirty = true;                                     // Phase 3.4: ImGui 통합 대비
	};

	/**
	 * @brief Spot Light (Phase 4+ 예정)
	 *
	 * 원뿔 형태로 빛 발산 (손전등, 무대 조명)
	 * TransformComponent의 position과 rotation 사용
	 */
	struct SpotLightComponent
	{
		// TODO: Phase 4
		// - color, intensity, range
		// - innerConeAngle, outerConeAngle
		// - constant, linear, quadratic
		// - isDirty
	};

} // namespace ECS
