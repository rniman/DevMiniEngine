/**
 * @file TransformSystem.h
 * @brief Transform 관련 로직을 처리하는 System
 *
 * 생성자 주입 방식으로 Registry를 받습니다.
 * 고수준(Entity 기반)과 저수준(Component 직접) API를 모두 제공합니다.
 *
 * Phase 3.5: 계층 구조 및 Dirty Flag 최적화 추가
 */
#pragma once
#include "ECS/ISystem.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/HierarchyComponent.h"
#include "ECS/Entity.h"
#include "Core/Types.h"
#include <vector>

namespace ECS
{
	class Registry;

	/**
	 * @brief Transform System
	 *
	 * Transform 계층 구조 업데이트를 처리합니다.
	 * 고수준(Entity)과 저수준(Component) API를 모두 제공합니다.
	 *
	 * @note Update()에서 계층 구조 순회 및 World Matrix 계산 수행
	 */
	class TransformSystem : public ISystem
	{
	public:
		/**
		 * @brief 생성자
		 * @param registry 이 System이 속한 Registry
		 */
		explicit TransformSystem(Registry& registry);
		~TransformSystem() override = default;

		//=========================================================================
		// ISystem 인터페이스 구현
		//=========================================================================

		void Initialize() override;

		/**
		 * @brief 매 프레임 Transform 계층 구조 업데이트
		 *
		 * 1. Root Entity부터 DFS 순회
		 * 2. localDirty면 localMatrix 재계산
		 * 3. worldDirty면 worldMatrix 재계산 + 자식 worldDirty 마킹
		 * 4. Hierarchy 없는 Entity는 단독 처리
		 */
		void Update(Core::float32 deltaTime) override;

		void Shutdown() override;

		//=========================================================================
		// 계층 구조 API (Phase 3.5)
		//=========================================================================

		/**
		 * @brief 부모-자식 관계 설정
		 *
		 * @param child 자식 Entity
		 * @param parent 부모 Entity (Invalid면 Root로 설정)
		 * @return 성공 여부
		 *
		 * @note 두 Entity 모두 HierarchyComponent 필요
		 *       child의 worldDirty가 true로 설정됨
		 */
		bool SetParent(Entity child, Entity parent);

		/**
		 * @brief 부모 Entity 조회
		 * @return 부모 Entity (없으면 Invalid)
		 */
		Entity GetParent(Entity entity) const;

		/**
		 * @brief 자식 Entity 목록 조회
		 * @return 자식 Entity 벡터 (없으면 빈 벡터)
		 */
		const std::vector<Entity>& GetChildren(Entity entity) const;

		/**
		 * @brief Root Entity 여부 확인
		 * @return HierarchyComponent가 있고 parent가 Invalid면 true
		 */
		bool IsRoot(Entity entity) const;

		/**
		 * @brief 모든 Root Entity 목록 조회
		 */
		const std::vector<Entity>& GetRootEntities() const { return mRootEntities; }

		//=========================================================================
		// 고수준 API (Entity 기반) - Dirty Flag 자동 마킹
		//=========================================================================

		bool SetPosition(Entity entity, const Math::Vector3& position);
		bool GetPosition(Entity entity, Math::Vector3& outPosition) const;

		bool SetRotationEuler(Entity entity, const Math::Vector3& eulerAngles);
		bool SetRotation(Entity entity, const Math::Quaternion& rotation);
		bool GetRotationEuler(Entity entity, Math::Vector3& outEuler) const;

		bool SetScale(Entity entity, const Math::Vector3& scale);
		bool SetScale(Entity entity, Core::float32 uniformScale);

		bool Rotate(Entity entity, const Math::Vector3& eulerDelta);
		bool RotateAround(Entity entity, const Math::Vector3& axis, Core::float32 angle);
		bool Translate(Entity entity, const Math::Vector3& delta);

		/**
		 * @brief 캐시된 World Matrix 반환
		 *
		 * @note Update() 호출 전에는 이전 프레임 값일 수 있음
		 *       즉시 정확한 값이 필요하면 ForceUpdateWorldMatrix() 사용
		 */
		bool GetWorldMatrix(Entity entity, Math::Matrix4x4& outMatrix) const;

		/**
		 * @brief 캐시된 Local Matrix 반환
		 */
		bool GetLocalMatrix(Entity entity, Math::Matrix4x4& outMatrix) const;

		bool GetWorldInvTranspose(Entity entity, Math::Matrix4x4& outMatrix) const;

		bool LookAt(
			Entity entity,
			const Math::Vector3& target,
			const Math::Vector3& up = Math::Vector3::Up()
		);

		/**
		 * @brief World Matrix 즉시 계산 (특수 용도)
		 *
		 * @note 런타임 스폰 직후 정확한 위치가 필요한 경우 사용
		 *       일반적으로는 Update()에서 일괄 계산되므로 불필요
		 */
		void ForceUpdateWorldMatrix(Entity entity);

		//=========================================================================
		// 저수준 API (Component 직접) - System 내부, 다른 System에서 호출
		//=========================================================================

		static void SetRotationEuler(TransformComponent& transform, const Math::Vector3& eulerAngles);
		static void SetRotationEuler(
			TransformComponent& transform,
			Core::float32 pitch,
			Core::float32 yaw,
			Core::float32 roll
		);
		static Math::Vector3 GetRotationEuler(const TransformComponent& transform);

		static void Rotate(TransformComponent& transform, const Math::Vector3& eulerDelta);
		static void RotateAround(TransformComponent& transform, const Math::Vector3& axis, Core::float32 angle);

		/// Local Matrix 계산 (캐시 사용하지 않음, 항상 재계산)
		static Math::Matrix4x4 CalculateLocalMatrix(const TransformComponent& transform);

		/// 캐시된 Local Matrix 반환
		static const Math::Matrix4x4& GetLocalMatrix(const TransformComponent& transform);

		/// 캐시된 World Matrix 반환
		static const Math::Matrix4x4& GetWorldMatrix(const TransformComponent& transform);

		static Math::Matrix4x4 GetWorldInvTranspose(const TransformComponent& transform);

		static Math::Vector3 GetForward(const TransformComponent& transform);
		static Math::Vector3 GetRight(const TransformComponent& transform);
		static Math::Vector3 GetUp(const TransformComponent& transform);

	private:
		/// Root Entity 목록에 추가
		void AddRootEntity(Entity entity);

		/// Root Entity 목록에서 제거
		void RemoveRootEntity(Entity entity);

		/// 단일 Entity의 Local Matrix 업데이트 (dirty면 재계산)
		void UpdateLocalMatrix(TransformComponent& transform);

		/// 계층 구조 DFS 순회 및 World Matrix 업데이트
		void UpdateHierarchy(Entity entity, const Math::Matrix4x4& parentWorldMatrix);

		/// Hierarchy 없는 Entity들 업데이트
		void UpdateStandaloneEntities();

		/// Transform 변경 시 dirty 플래그 설정
		void MarkLocalDirty(TransformComponent& transform);

	private:
		/// Root Entity 목록 (parent가 Invalid인 HierarchyComponent를 가진 Entity)
		std::vector<Entity> mRootEntities;

		/// 빈 children 벡터 (GetChildren 반환용)
		static const std::vector<Entity> sEmptyChildren;

		//=========================================================================
		// TODO: [OPTIMIZATION] Phase 4+
		// - 현재: DFS 재귀 순회
		// - 최적화: 명시적 스택 (깊이 100+ 시)
		// - 최적화: Topological Sort 캐싱 (Entity 10,000+ 시)
		// - 최적화: 독립 서브트리 병렬 처리 (Job System 도입 후)
		//=========================================================================
	};

} // namespace ECS
