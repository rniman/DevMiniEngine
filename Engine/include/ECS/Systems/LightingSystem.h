/**
 * @file LightingSystem.h
 * @brief 조명 데이터 수집 및 처리 System
 *
 * 생성자 주입 방식으로 Registry를 받습니다.
 * 고수준(Entity 기반)과 저수준(Component 직접) API를 모두 제공합니다.
 */
#pragma once
#include "ECS/ISystem.h"
#include "Core/Types.h"
#include "Math/MathTypes.h"
#include "ECS/Entity.h"
#include "Graphics/RenderTypes.h"  // DirectionalLightData, PointLightData 사용
#include <vector>

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
	 * 매 프레임 조명 관련 처리를 수행합니다.
	 * 고수준(Entity)과 저수준(Component) API를 모두 제공합니다.
	 */
	class LightingSystem : public ISystem
	{
	public:
		/**
		 * @brief 생성자
		 * @param registry 이 System이 속한 Registry
		 */
		explicit LightingSystem(Registry& registry);
		~LightingSystem() override = default;

		//=========================================================================
		// ISystem 인터페이스 구현
		//=========================================================================

		void Initialize() override;
		void Update(Core::float32 deltaTime) override;
		void Shutdown() override;

		//=========================================================================
		// 고수준 API (Entity 기반)
		//=========================================================================

		bool SetDirection(Entity entity, const Math::Vector3& direction);
		bool SetColor(Entity entity, const Math::Vector3& color);
		bool SetIntensity(Entity entity, Core::float32 intensity);
		bool SetRange(Entity entity, Core::float32 range);
		bool SetAttenuation(Entity entity, Core::float32 constant, Core::float32 linear, Core::float32 quadratic);

		//=========================================================================
		// 조명 데이터 수집 (RenderSystem에서 호출)
		//=========================================================================

		static void CollectDirectionalLights(Registry& registry, std::vector<Graphics::DirectionalLightData>& outLights);
		static void CollectPointLights(Registry& registry, std::vector<Graphics::PointLightData>& outLights);

		//=========================================================================
		// 저수준 API (Component 직접) - 정적
		//=========================================================================

		static Core::float32 CalculateAttenuation(const PointLightComponent& light, Core::float32 distance);
		static void NormalizeDirection(DirectionalLightComponent& light);
	};

} // namespace ECS
