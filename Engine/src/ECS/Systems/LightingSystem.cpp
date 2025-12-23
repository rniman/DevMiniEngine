#include "pch.h"
#include "ECS/Systems/LightingSystem.h"
#include "ECS/Archetype.h"
#include "ECS/Registry.h"
#include "ECS/RegistryView.h"
#include "ECS/Components/LightComponents.h"
#include "ECS/Components/TransformComponent.h"
#include "Core/Logging/LogMacros.h"

namespace ECS
{
	LightingSystem::LightingSystem(Registry& registry)
		: ISystem(registry)
	{
	}

	//=========================================================================
	// ISystem 인터페이스
	//=========================================================================

	void LightingSystem::Initialize()
	{
		LOG_INFO("[LightingSystem] Initialized");
	}

	void LightingSystem::Update(Core::float32 deltaTime)
	{
		// 조명 애니메이션, 그림자 업데이트 등 향후 확장 예정
	}

	void LightingSystem::Shutdown()
	{
		LOG_INFO("[LightingSystem] Shutdown");
	}

	//=========================================================================
	// 고수준 API (Entity 기반)
	//=========================================================================

	bool LightingSystem::SetDirection(Entity entity, const Math::Vector3& direction)
	{
		auto* light = GetRegistry()->GetComponent<DirectionalLightComponent>(entity);
		if (!light) return false;

		light->direction = direction.Normalized();
		light->isDirty = true;
		return true;
	}

	bool LightingSystem::SetColor(Entity entity, const Math::Vector3& color)
	{
		if (auto* dirLight = GetRegistry()->GetComponent<DirectionalLightComponent>(entity))
		{
			dirLight->color = color;
			dirLight->isDirty = true;
			return true;
		}

		if (auto* pointLight = GetRegistry()->GetComponent<PointLightComponent>(entity))
		{
			pointLight->color = color;
			return true;
		}

		return false;
	}

	bool LightingSystem::SetIntensity(Entity entity, Core::float32 intensity)
	{
		if (auto* dirLight = GetRegistry()->GetComponent<DirectionalLightComponent>(entity))
		{
			dirLight->intensity = intensity;
			dirLight->isDirty = true;
			return true;
		}

		if (auto* pointLight = GetRegistry()->GetComponent<PointLightComponent>(entity))
		{
			pointLight->intensity = intensity;
			return true;
		}

		return false;
	}

	bool LightingSystem::SetRange(Entity entity, Core::float32 range)
	{
		auto* light = GetRegistry()->GetComponent<PointLightComponent>(entity);
		if (!light) return false;

		light->range = range;
		return true;
	}

	bool LightingSystem::SetAttenuation(
		Entity entity,
		Core::float32 constant,
		Core::float32 linear,
		Core::float32 quadratic)
	{
		auto* light = GetRegistry()->GetComponent<PointLightComponent>(entity);
		if (!light) return false;

		light->constant = constant;
		light->linear = linear;
		light->quadratic = quadratic;
		return true;
	}

	//=========================================================================
	// GPU 데이터 수집
	//=========================================================================

	void LightingSystem::CollectDirectionalLights(
		Registry& registry,
		std::vector<Graphics::DirectionalLightData>& outLights)
	{
		outLights.clear();

		auto view = DirectionalLightArchetype::CreateView(registry);

		Core::uint32 count = 0;
		for (Entity entity : view)
		{
			if (count >= MAX_DIRECTIONAL_LIGHTS)
			{
				LOG_WARN("[LightingSystem] Max Directional Lights exceeded (%u)", MAX_DIRECTIONAL_LIGHTS);
				break;
			}

			auto* light = registry.GetComponent<DirectionalLightComponent>(entity);
			if (!light) continue;

			Graphics::DirectionalLightData data;
			data.direction = light->direction.ToDirection();
			data.color = light->color;
			data.intensity = light->intensity;

			outLights.push_back(data);
			++count;
		}
	}

	void LightingSystem::CollectPointLights(
		Registry& registry,
		std::vector<Graphics::PointLightData>& outLights)
	{
		outLights.clear();

		auto view = PointLightArchetype::CreateView(registry);

		Core::uint32 count = 0;
		for (Entity entity : view)
		{
			if (count >= MAX_POINT_LIGHTS)
			{
				LOG_WARN("[LightingSystem] Max Point Lights exceeded (%u)", MAX_POINT_LIGHTS);
				break;
			}

			auto* transform = registry.GetComponent<TransformComponent>(entity);
			auto* light = registry.GetComponent<PointLightComponent>(entity);
			if (!transform || !light) continue;

			Graphics::PointLightData data;
			data.position = transform->position.ToPoint();
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
	}

	//=========================================================================
	// Debug Entity 수집
	//=========================================================================

	void LightingSystem::CollectDirectionalLightEntities(
		Registry& registry,
		std::vector<Entity>& outEntities)
	{
		outEntities.clear();

		auto view = DirectionalLightArchetype::CreateView(registry);

		Core::uint32 count = 0;
		for (Entity entity : view)
		{
			if (count >= MAX_DIRECTIONAL_LIGHTS) break;

			auto* light = registry.GetComponent<DirectionalLightComponent>(entity);
			if (!light) continue;

			outEntities.push_back(entity);
			++count;
		}
	}

	void LightingSystem::CollectPointLightEntities(
		Registry& registry,
		std::vector<Entity>& outEntities)
	{
		outEntities.clear();

		auto view = PointLightArchetype::CreateView(registry);

		Core::uint32 count = 0;
		for (Entity entity : view)
		{
			if (count >= MAX_POINT_LIGHTS) break;

			outEntities.push_back(entity);
			++count;
		}
	}

	//=========================================================================
	// 저수준 API
	//=========================================================================

	Core::float32 LightingSystem::CalculateAttenuation(const PointLightComponent& light, Core::float32 distance)
	{
		if (distance > light.range) return 0.0f;

		Core::float32 attenuation = 1.0f / (
			light.constant +
			light.linear * distance +
			light.quadratic * distance * distance
			);

		return std::max(attenuation, 0.0f);
	}

	void LightingSystem::NormalizeDirection(DirectionalLightComponent& light)
	{
		light.direction.Normalize();
		light.isDirty = true;
	}

} // namespace ECS
