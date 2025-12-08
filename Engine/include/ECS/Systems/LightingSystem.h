#pragma once
#include "Core/Types.h"
#include "Math/MathTypes.h"
#include "ECS/Entity.h"
#include "Graphics/RenderTypes.h"  // DirectionalLightData, PointLightData 사용
#include <vector>

/**
 * @file LightingSystem.h
 * @brief 조명 데이터 수집 및 처리 System (Phase 3.3)
 */

namespace ECS
{
	// Forward declarations
	class Registry;
	struct DirectionalLightComponent;
	struct PointLightComponent;
	struct TransformComponent;

	/**
	 * @brief 조명 데이터 수집 및 처리 System
	 *
	 * ECS 원칙: 상태 없는 정적 함수로 구성
	 * Component는 데이터만, 모든 로직은 System에서 처리합니다.
	 *
	 * Phase 3.3: Phong Shading 지원
	 * Component(Vector3) → GPU Data(Vector4) 변환 담당
	 */
	class LightingSystem
	{
	public:
		/**
		 * @brief Directional Light 데이터 수집
		 *
		 * Component의 direction(Vector3)를 Vector4(xyz, 0.0)로 변환
		 * Registry에서 DirectionalLightComponent를 가진 Entity를 찾아
		 * Graphics::DirectionalLightData로 변환합니다.
		 *
		 * @param registry ECS Registry
		 * @param outLights 수집된 조명 데이터 (출력)
		 *
		 * @note MAX_DIRECTIONAL_LIGHTS 개수만큼만 수집
		 */
		static void CollectDirectionalLights(
			Registry& registry,
			std::vector<Graphics::DirectionalLightData>& outLights
		);

		/**
		 * @brief Point Light 데이터 수집
		 *
		 * Transform의 position(Vector3)를 Vector4(xyz, 1.0)로 변환
		 * Registry에서 TransformComponent + PointLightComponent를 가진
		 * Entity를 찾아 Graphics::PointLightData로 변환합니다.
		 *
		 * @param registry ECS Registry
		 * @param outLights 수집된 조명 데이터 (출력)
		 *
		 * @note MAX_POINT_LIGHTS 개수만큼만 수집
		 * @note Transform의 position을 조명 위치로 사용
		 */
		static void CollectPointLights(
			Registry& registry,
			std::vector<Graphics::PointLightData>& outLights
		);

		/**
		 * @brief Point Light Attenuation 계산
		 *
		 * 거리에 따른 조명 감쇠 값을 계산합니다.
		 * Attenuation = 1.0 / (Kc + Kl * d + Kq * d^2)
		 *
		 * @param light Point Light Component
		 * @param distance 조명으로부터의 거리
		 * @return 감쇠 계수 (0.0 ~ 1.0)
		 *
		 * @note range를 벗어나면 0.0 반환
		 */
		static Core::float32 CalculateAttenuation(
			const PointLightComponent& light,
			Core::float32 distance
		);

		/**
		 * @brief Directional Light 방향 정규화
		 *
		 * Component의 direction 벡터를 정규화합니다.
		 *
		 * @param light 정규화할 Directional Light Component
		 *
		 * @note 매 프레임 호출할 필요 없음 (데이터 변경 시에만)
		 */
		static void NormalizeDirection(DirectionalLightComponent& light);
	};

} // namespace ECS
