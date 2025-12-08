#include "pch.h"
#include "ECS/Systems/LightingSystem.h"
#include "ECS/Registry.h"
#include "ECS/Components/LightComponents.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/RegistryView.h"
#include "Core/Logging/LogMacros.h"
#include "Math/MathUtils.h"


namespace ECS
{
	void LightingSystem::CollectDirectionalLights(
		Registry& registry,
		std::vector<Graphics::DirectionalLightData>& outLights
	)
	{
		outLights.clear();

		// DirectionalLightComponent를 가진 Entity 찾기
		auto view = registry.CreateView<DirectionalLightComponent>();

		Core::uint32 count = 0;
		for (Entity entity : view)
		{
			// 최대 개수 제한
			if (count >= MAX_DIRECTIONAL_LIGHTS)
			{
				LOG_WARN(
					"[LightingSystem] Exceeded maximum Directional Light count (%u). Excess lights will be ignored.",
					MAX_DIRECTIONAL_LIGHTS
				);
				break;
			}

			auto* light = registry.GetComponent<DirectionalLightComponent>(entity);
			if (!light)
			{
				continue;
			}

			// Component(Vector3) - GPU Data(Vector4) 변환
			Graphics::DirectionalLightData data;

			// Vector3 direction - Vector4 (w=0.0, homogeneous vector)
			data.direction = Math::Vector4(
				light->direction.x,
				light->direction.y,
				light->direction.z,
				0.0f  // 방향 벡터는 w=0.0 (평행이동 영향 없음)
			);

			data.color = light->color;
			data.intensity = light->intensity;

			outLights.push_back(data);
			++count;
		}

#ifdef _DEBUG
		if (!outLights.empty())
		{
			LOG_DEBUG(
				"[LightingSystem] Collected %zu Directional Lights (Max: %u)",
				outLights.size(),
				MAX_DIRECTIONAL_LIGHTS
			);
		}
#endif
	}

	void LightingSystem::CollectPointLights(
		Registry& registry,
		std::vector<Graphics::PointLightData>& outLights
	)
	{
		outLights.clear();

		// TransformComponent + PointLightComponent를 가진 Entity 찾기
		auto view = registry.CreateView<TransformComponent, PointLightComponent>();

		Core::uint32 count = 0;
		for (Entity entity : view)
		{
			// 최대 개수 제한
			if (count >= MAX_POINT_LIGHTS)
			{
				LOG_WARN(
					"[LightingSystem] Exceeded maximum Point Light count (%u). Excess lights will be ignored.",
					MAX_POINT_LIGHTS
				);
				break;
			}

			auto* transform = registry.GetComponent<TransformComponent>(entity);
			auto* light = registry.GetComponent<PointLightComponent>(entity);

			if (!transform || !light)
			{
				continue;
			}

			// Component(Vector3) - GPU Data(Vector4) 변환
			Graphics::PointLightData data;

			// Vector3 position - Vector4 (w=1.0, homogeneous point)
			data.position = Math::Vector4(
				transform->position.x,
				transform->position.y,
				transform->position.z,
				1.0f  // 위치는 w=1.0 (평행이동 영향 받음)
			);

			data.rangeAndColor.x = light->range;
			data.rangeAndColor.y = light->color.x;
			data.rangeAndColor.z = light->color.y;
			data.rangeAndColor.w = light->color.z;
			data.intensityAndAttenuation.x = light->intensity;
			data.intensityAndAttenuation.y = light->constant;
			data.intensityAndAttenuation.z = light->linear;
			data.intensityAndAttenuation.w = light->quadratic;

			outLights.push_back(data);
			++count;
		}

#ifdef _DEBUG
		if (!outLights.empty())
		{
			LOG_DEBUG(
				"[LightingSystem] Collected %zu Point Lights (Max: %u)",
				outLights.size(),
				MAX_POINT_LIGHTS
			);
		}
#endif
	}

	Core::float32 LightingSystem::CalculateAttenuation(
		const PointLightComponent& light,
		Core::float32 distance
	)
	{
		// 범위를 벗어나면 즉시 0.0 반환
		if (distance > light.range)
		{
			return 0.0f;
		}

		// Attenuation = 1.0 / (Kc + Kl * d + Kq * d^2)
		Core::float32 attenuation = 1.0f / (
			light.constant +
			light.linear * distance +
			light.quadratic * distance * distance
			);

		// 최소값 제한 (0.0 이하로 내려가지 않도록)
		return std::max(attenuation, 0.0f);
	}

	void LightingSystem::NormalizeDirection(DirectionalLightComponent& light)
	{
		// 방향 벡터 정규화
		light.direction = Math::Normalize(light.direction);

		// Dirty Flag 설정 (Phase 3.4에서 활용)
		light.isDirty = true;
	}

} // namespace ECS
