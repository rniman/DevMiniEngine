// Archetype 템플릿 메서드 구현
// 이 파일은 Archetype.h 끝에서 include됩니다.

#pragma once
#include "ECS/Registry.h"

namespace ECS
{
	template<typename... Components>
	auto Archetype<Components...>::CreateView(Registry& registry)
	{
		return registry.CreateView<Components...>();
	}

} // namespace ECS
