#pragma once
#include "ECS/Entity.h"
#include "Math/MathTypes.h"

namespace ECS
{
	/**
	 * @brief Transform 컴포넌트 (순수 데이터)
	 *
	 * ECS 원칙: Components는 데이터만 포함
	 * 모든 로직은 TransformSystem에서 처리
	 */
	struct TransformComponent
	{
		Math::Vector3 position = Math::Vector3::Zero();
		Math::Quaternion rotation = Math::Quaternion::Identity();
		Math::Vector3 scale = Math::Vector3::One();

		// TODO: Phase 3+에서 추가 예정
		// Entity parent = Entity::Invalid();
		// bool worldMatrixDirty = true;
		// Math::Matrix4x4 cachedWorldMatrix;
	};

} // namespace ECS
