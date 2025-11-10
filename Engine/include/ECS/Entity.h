#pragma once
#include "Core/Types.h"
#include <functional>

namespace ECS
{
	struct Entity
	{
		Core::uint32 id = UINT32_MAX;
		Core::uint32 version = UINT32_MAX;

		bool IsValid() const { return id != UINT32_MAX; }

		bool operator==(const Entity& other) const
		{
			return (id == other.id) && (version == other.version);
		}

		bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}

		bool operator<(const Entity& other) const
		{
			if (id != other.id)
			{
				return id < other.id;
			}

			return version < other.version;
		}
	};

}; // namespace ECS

namespace std
{
	template<>
	struct hash<ECS::Entity>
	{
		size_t operator()(const ECS::Entity& entity) const noexcept
		{
			// id와 version을 조합한 hash 값 생성
			return hash<Core::uint32>()(entity.id) ^ (hash<Core::uint32>()(entity.version) << 1);
		}
	};
}
