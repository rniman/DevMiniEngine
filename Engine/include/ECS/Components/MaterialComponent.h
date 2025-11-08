#pragma once
#include "Framework/Resources/ResourceId.h"

namespace ECS
{
	/**
	 * @brief Material 컴포넌트 (순수 데이터)
	 *
	 * 실제 Material은 ResourceManager가 소유
	 * 이 Component는 ID만 저장
	 */
	struct MaterialComponent
	{
		Framework::ResourceId materialId = Framework::ResourceId::Invalid();
	};

} // namespace ECS
