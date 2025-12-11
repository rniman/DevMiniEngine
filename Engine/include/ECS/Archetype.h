#pragma once
#include "Core/Types.h"
#include <tuple>
#include <type_traits>

// Forward declarations
namespace ECS
{
	class Registry;
	struct TransformComponent;
	struct MeshComponent;
	struct MaterialComponent;
	struct CameraComponent;
		struct DirectionalLightComponent;
	struct PointLightComponent;
}

namespace ECS
{
	/**
	 * @brief Component 조합을 타입으로 정의하는 Archetype 베이스
	 *
	 * Archetype은 특정 Component 조합을 나타내는 타입입니다.
	 * 이를 통해 Component 조합을 재사용하고, 타입 안전성을 보장하며,
	 * 메타프로그래밍을 활용할 수 있습니다.
	 *
	 * @tparam Components Component 타입들
	 *
	 * @example
	 * using MyArchetype = Archetype<TransformComponent, MeshComponent>;
	 * auto view = MyArchetype::CreateView(registry);
	 */
	template<typename... Components>
	struct Archetype
	{
		// Component 타입들을 tuple로 저장
		using ComponentTypes = std::tuple<Components...>;

		// Component 개수 (컴파일 타임 상수)
		static constexpr Core::uint32 ComponentCount = sizeof...(Components);

		/**
		 * @brief Registry에서 이 Archetype의 Entity View 생성
		 *
		 * @param registry ECS Registry
		 * @return 이 Archetype의 Component 조합을 가진 Entity View
		 */
		static auto CreateView(Registry& registry);

		/**
		 * @brief 특정 인덱스의 Component 타입 가져오기 (컴파일 타임)
		 *
		 * @tparam Index Component 인덱스 (0부터 시작)
		 * @return Component 타입
		 *
		 * @example
		 * using FirstComponent = MyArchetype::GetComponentType<0>;
		 */
		template<Core::uint32 Index>
		using GetComponentType = std::tuple_element_t<Index, ComponentTypes>;

		/**
		 * @brief 특정 Component 타입이 포함되어 있는지 확인 (컴파일 타임)
		 *
		 * @tparam T 확인할 Component 타입
		 * @return true if T is in Components, false otherwise
		 *
		 * @example
		 * constexpr bool hasTransform = MyArchetype::HasComponent<TransformComponent>();
		 */
		template<typename T>
		static constexpr bool HasComponent()
		{
			return (std::is_same_v<T, Components> || ...);
		}
	};

	// ========== 공통 Archetype 정의 ==========

	/**
	 * @brief 렌더링 가능한 Entity
	 *
	 * Transform, Mesh, Material을 가진 Entity입니다.
	 * 화면에 그려질 수 있는 모든 오브젝트가 이 조합을 가집니다.
	 */
	using RenderableArchetype = Archetype<
		TransformComponent,
		MeshComponent,
		MaterialComponent
	>;

	/**
	 * @brief 카메라 Entity
	 *
	 * Transform과 Camera Component를 가진 Entity입니다.
	 * 씬을 렌더링하는 시점을 제공합니다.
	 */
	using CameraArchetype = Archetype<
		TransformComponent,
		CameraComponent
	>;

	/**
	 * @brief Camera Component만 가진 Entity
	 *
	 * Main Camera 검색 등에서 사용됩니다.
	 */
	using CameraOnlyArchetype = Archetype<CameraComponent>;

	/**
	 * @brief Transform만 있는 Entity
	 *
	 * 위치, 회전, 스케일 정보만 가진 Entity입니다.
	 * 빈 GameObject나 트랜스폼 노드로 사용됩니다.
	 */
	using TransformOnlyArchetype = Archetype<TransformComponent>;

	// ========== Light Archetype 정의 ==========

	/**
	 * @brief Directional Light Entity
	 *
	 * DirectionalLightComponent만 가진 Entity입니다.
	 * 태양광처럼 방향만 있고 위치가 없는 조명입니다.
	 *
	 * @note Transform이 필요 없음 (방향만 사용)
	 */
	using DirectionalLightArchetype = Archetype<DirectionalLightComponent>;

	/**
	 * @brief Point Light Entity
	 *
	 * Transform과 PointLightComponent를 가진 Entity입니다.
	 * 위치에서 모든 방향으로 빛을 발산하는 조명입니다.
	 *
	 * @note Transform의 position을 조명 위치로 사용
	 */
	using PointLightArchetype = Archetype<
		TransformComponent,
		PointLightComponent
	>;

	// ========== Archetype 유틸리티 ==========

	/**
	 * @brief 두 Archetype이 같은지 비교 (컴파일 타임)
	 *
	 * @tparam Archetype1 첫 번째 Archetype
	 * @tparam Archetype2 두 번째 Archetype
	 * @return true if same component types in same order, false otherwise
	 */
	template<typename Archetype1, typename Archetype2>
	struct IsSameArchetype : std::is_same<
		typename Archetype1::ComponentTypes,
		typename Archetype2::ComponentTypes
	>
	{
	};

	template<typename Archetype1, typename Archetype2>
	inline constexpr bool IsSameArchetype_v = IsSameArchetype<Archetype1, Archetype2>::value;

	/**
	 * @brief Archetype의 Component 개수 가져오기 (컴파일 타임)
	 *
	 * @tparam ArchetypeT Archetype 타입
	 * @return Component 개수
	 */
	template<typename ArchetypeT>
	inline constexpr Core::uint32 GetArchetypeSize_v = ArchetypeT::ComponentCount;
	
} // namespace ECS

// 템플릿 구현 include
#include "ECS/Archetype.inl"
