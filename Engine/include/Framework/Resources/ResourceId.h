#pragma once
#include "Core/Types.h"
#include <functional>

namespace Framework
{
	/**
	 * @brief 64비트 해시 기반 리소스 ID
	 *
	 * 리소스 경로의 해시 값을 저장
	 * 충돌 확률이 사실상 0
	 */
	struct ResourceId
	{
		Core::uint64 id = UINT64_MAX;

		static ResourceId Invalid() { return ResourceId{ UINT64_MAX }; }
		bool IsValid() const { return id != UINT64_MAX; }

		bool operator==(const ResourceId& other) const { return id == other.id; }
		bool operator!=(const ResourceId& other) const { return id != other.id; }
		bool operator<(const ResourceId& other) const { return id < other.id; }
	};

} // namespace Framework

namespace std
{
	template<>
	struct hash<Framework::ResourceId>
	{
		size_t operator()(const Framework::ResourceId& id) const noexcept
		{
			return hash<Core::uint64>()(id.id);
		}
	};
}
