#pragma once
#include "ECS/ISystem.h"
#include "ECS/Entity.h"
#include "Core/Types.h"
#include "Math/MathTypes.h"
#include "Graphics/RenderTypes.h"
#include <vector>

namespace ECS
{
	class Registry;
	struct DirectionalLightComponent;
	struct PointLightComponent;

	/**
	 * @brief 조명 데이터 수집 및 처리 System
	 *
	 * 고수준(Entity)과 저수준(Component) API를 모두 제공합니다.
	 */
	class LightingSystem : public ISystem
	{
	public:
		explicit LightingSystem(Registry& registry);
		~LightingSystem() override = default;

		//=====================================================================
		// ISystem 인터페이스
		//=====================================================================

		void Initialize() override;
		void Update(Core::float32 deltaTime) override;
		void Shutdown() override;

		//=====================================================================
		// 고수준 API (Entity 기반)
		//=====================================================================

		bool SetDirection(Entity entity, const Math::Vector3& direction);
		bool SetColor(Entity entity, const Math::Vector3& color);
		bool SetIntensity(Entity entity, Core::float32 intensity);
		bool SetRange(Entity entity, Core::float32 range);
		bool SetAttenuation(Entity entity, Core::float32 constant, Core::float32 linear, Core::float32 quadratic);

		//=====================================================================
		// GPU 데이터 수집 (RenderSystem에서 호출)
		//=====================================================================

		static void CollectDirectionalLights(Registry& registry, std::vector<Graphics::DirectionalLightData>& outLights);
		static void CollectPointLights(Registry& registry, std::vector<Graphics::PointLightData>& outLights);

		//=====================================================================
		// Debug Entity 수집 (DebugRenderer에서 사용)
		//=====================================================================

		static void CollectDirectionalLightEntities(Registry& registry, std::vector<Entity>& outEntities);
		static void CollectPointLightEntities(Registry& registry, std::vector<Entity>& outEntities);

		//=====================================================================
		// 저수준 API (Component 직접)
		//=====================================================================

		static Core::float32 CalculateAttenuation(const PointLightComponent& light, Core::float32 distance);
		static void NormalizeDirection(DirectionalLightComponent& light);
	};

} // namespace ECS
