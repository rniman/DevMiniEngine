#pragma once
#include "Core/Types.h"
#include <functional>

namespace ECS
{
	namespace Internal
	{
		// 컴포넌트 타입마다 고유한 ID를 생성
		inline Core::uint32 GenerateComponentId()
		{
			static Core::uint32 nextId = 0;
			return nextId++;
		}

		// 각 컴포넌트 타입의 고유 ID를 반환
		template<typename T>
		inline Core::uint32 GetComponentId()
		{
			static const Core::uint32 id = GenerateComponentId();
			return id;
		}

#ifdef _DEBUG
		template<typename T>
		inline const char* GetComponentTypeName()
		{
			return typeid(T).name();
		}
#endif

	} // namespace Internal

} // namespace ECS
