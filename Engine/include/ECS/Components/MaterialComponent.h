/**
 * @file MaterialComponent.h
 * @brief Material 컴포넌트 (순수 데이터)
 *
 * @note Phase 4.4: 다중 Material 지원 (서브메시별 Material)
 */
#pragma once
#include "Framework/Resources/ResourceId.h"
#include "Core/Types.h"
#include <array>

namespace ECS
{
	/**
	 * @brief Material 컴포넌트 (순수 데이터)
	 *
	 * 실제 Material은 ResourceManager가 소유합니다.
	 * 서브메시별로 다른 Material을 적용할 수 있습니다.
	 *
	 * @note Phase 4.4: 다중 Material 지원 (최대 8개)
	 * @note 서브메시 인덱스와 materialIds 인덱스가 1:1 매핑됩니다.
	 */
	struct MaterialComponent
	{
		static constexpr Core::uint32 MAX_MATERIALS = 8;

		std::array<Framework::ResourceId, MAX_MATERIALS> materialIds = {};
		Core::uint32 count = 1;
	};

	//=========================================================================
	// MaterialComponent 헬퍼 함수
	//=========================================================================

	namespace MaterialHelpers
	{
		/**
		 * @brief 서브메시 인덱스에 해당하는 Material 반환
		 *
		 * @param material MaterialComponent
		 * @param submeshIndex 서브메시 인덱스
		 * @return Material ID (부족하면 마지막 Material 반복)
		 */
		inline Framework::ResourceId GetMaterial(
			const MaterialComponent& material,
			Core::uint32 submeshIndex
		)
		{
			if (submeshIndex < material.count)
			{
				return material.materialIds[submeshIndex];
			}
			// 부족하면 마지막 Material 반복
			if (material.count > 0)
			{
				return material.materialIds[material.count - 1];
			}
			return Framework::ResourceId::Invalid();
		}

		/**
		 * @brief Material 설정
		 *
		 * @param material MaterialComponent
		 * @param index 슬롯 인덱스
		 * @param id Material ID
		 */
		inline void SetMaterial(
			MaterialComponent& material,
			Core::uint32 index,
			Framework::ResourceId id
		)
		{
			if (index < MaterialComponent::MAX_MATERIALS)
			{
				material.materialIds[index] = id;
				if (index >= material.count)
				{
					material.count = index + 1;
				}
			}
		}

		/**
		 * @brief 단일 Material 설정 (하위 호환)
		 *
		 * @param material MaterialComponent
		 * @param id Material ID
		 */
		inline void SetSingleMaterial(MaterialComponent& material, Framework::ResourceId id)
		{
			material.materialIds[0] = id;
			material.count = 1;
		}

	} // namespace MaterialHelpers

} // namespace ECS
