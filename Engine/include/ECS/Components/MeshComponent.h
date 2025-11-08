#pragma once
#include "Framework/Resources/ResourceId.h"

namespace ECS
{
	/**
	 * @brief Mesh 컴포넌트 (순수 데이터)
	 *
	 * 실제 Mesh는 ResourceManager가 소유
	 * 이 Component는 ID만 저장
	 */
	struct MeshComponent
	{
		Framework::ResourceId meshId = Framework::ResourceId::Invalid();
	};

} // namespace ECS
