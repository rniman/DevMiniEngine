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
		 * 2. localDirty/worldDirty/subtreeDirty 모두 false면 서브트리 스킵
		 * 3. localDirty면 localMatrix 재계산
		 * 4. worldDirty면 worldMatrix 재계산 + 자식 worldDirty 마킹
		 * 5. Hierarchy 없는 Entity는 단독 처리
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

		//=========================================================================
		// 고수준 API 확장 - Position
		//=========================================================================

		/**
		 * @brief 목표 지점을 향해 최대 maxDistance만큼 이동
		 * @param entity 대상 Entity
		 * @param target 목표 위치
		 * @param maxDistance 최대 이동 거리
		 * @return 성공 여부
		 */
		bool MoveTowards(Entity entity, const Math::Vector3& target, Core::float32 maxDistance);

		/**
		 * @brief 위치 선형 보간
		 * @param entity 대상 Entity
		 * @param target 목표 위치
		 * @param t 보간 계수 (0~1, 자동 클램프)
		 * @return 성공 여부
		 */
		bool LerpPosition(Entity entity, const Math::Vector3& target, Core::float32 t);

		//=========================================================================
		// 고수준 API 확장 - Rotation
		//=========================================================================

		/**
		 * @brief 목표 회전으로 최대 maxRadians만큼 회전
		 * @param entity 대상 Entity
		 * @param target 목표 회전 (Quaternion)
		 * @param maxRadians 최대 회전 각도 (라디안)
		 * @return 성공 여부
		 */
		bool RotateTowards(Entity entity, const Math::Quaternion& target, Core::float32 maxRadians);

		/**
		 * @brief 회전 구면 선형 보간 (Slerp)
		 * @param entity 대상 Entity
		 * @param target 목표 회전
		 * @param t 보간 계수 (0~1, 자동 클램프)
		 * @return 성공 여부
		 */
		bool SlerpRotation(Entity entity, const Math::Quaternion& target, Core::float32 t);

		/**
		 * @brief 특정 방향을 바라보도록 회전
		 * @param entity 대상 Entity
		 * @param direction 바라볼 방향 (정규화 불필요)
		 * @param up 상향 벡터
		 * @return 성공 여부 (방향이 영벡터면 false)
		 */
		bool LookDirection(
			Entity entity,
			const Math::Vector3& direction,
			const Math::Vector3& up = Math::Vector3::Up()
		);

		//=========================================================================
		// 고수준 API 확장 - Scale
		//=========================================================================

		/**
		 * @brief 상대적 스케일 적용 (현재 스케일에 곱하기)
		 * @param entity 대상 Entity
		 * @param multiplier 스케일 배수
		 * @return 성공 여부
		 */
		bool ScaleBy(Entity entity, const Math::Vector3& multiplier);
		bool ScaleBy(Entity entity, Core::float32 uniformMultiplier);

		/**
		 * @brief 스케일 선형 보간
		 * @param entity 대상 Entity
		 * @param target 목표 스케일
		 * @param t 보간 계수 (0~1, 자동 클램프)
		 * @return 성공 여부
		 */
		bool LerpScale(Entity entity, const Math::Vector3& target, Core::float32 t);

		//=========================================================================
		// 유틸리티 Query API
		//=========================================================================

		/**
		 * @brief 두 Entity 간 거리 계산
		 * @return 거리 (Entity 유효하지 않으면 -1.0f)
		 */
		Core::float32 GetDistanceTo(Entity from, Entity to) const;

		/**
		 * @brief 한 Entity에서 다른 Entity를 향한 방향 벡터
		 * @param from 시작 Entity
		 * @param to 목표 Entity
		 * @param outDirection 결과 방향 벡터 (정규화됨)
		 * @return 성공 여부 (같은 위치면 false, outDirection은 Forward)
		 */
		bool GetDirectionTo(Entity from, Entity to, Math::Vector3& outDirection) const;

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
		// 저수준 읽기 전용 API (Component 직접)
		//=========================================================================

		/// Quaternion에서 Euler 각도 추출
		static Math::Vector3 GetRotationEuler(const TransformComponent& transform);

		/// Local Matrix 계산 (캐시 사용하지 않음, 항상 재계산)
		static Math::Matrix4x4 CalculateLocalMatrix(const TransformComponent& transform);

		/// 캐시된 Local Matrix 반환
		static const Math::Matrix4x4& GetLocalMatrix(const TransformComponent& transform);

		/// 캐시된 World Matrix 반환
		static const Math::Matrix4x4& GetWorldMatrix(const TransformComponent& transform);

		/// World 역전치 행렬 계산 (노멀 변환용)
		static Math::Matrix4x4 GetWorldInvTranspose(const TransformComponent& transform);

		/// 로컬 Forward 벡터 반환
		static Math::Vector3 GetForward(const TransformComponent& transform);

		/// 로컬 Right 벡터 반환
		static Math::Vector3 GetRight(const TransformComponent& transform);

		/// 로컬 Up 벡터 반환
		static Math::Vector3 GetUp(const TransformComponent& transform);

		//=========================================================================
		// Dirty Flag API
		//=========================================================================

		/**
		 * @brief Transform 변경 시 dirty 플래그 설정 + 부모 체인 subtreeDirty 전파
		 *
		 * 대상 Entity의 localDirty + worldDirty를 설정하고,
		 * 부모 체인을 따라 subtreeDirty를 전파합니다.
		 * 상위 노드가 이미 subtreeDirty면 전파를 조기 종료합니다.
		 *
		 * 계층 구조의 중간 노드를 직접 수정할 때 사용합니다.
		 */
		static void MarkDirty(Registry& registry, Entity entity);

	private:
		//=========================================================================
		// 저수준 쓰기 API (내부 전용)
		// - dirty flag를 설정하지 않음
		// - 고수준 API에서 내부적으로 사용
		//=========================================================================

		static void SetRotationEulerInternal(TransformComponent& transform, const Math::Vector3& eulerAngles);
		static void SetRotationEulerInternal(
			TransformComponent& transform,
			Core::float32 pitch,
			Core::float32 yaw,
			Core::float32 roll
		);
		static void RotateInternal(TransformComponent& transform, const Math::Vector3& eulerDelta);
		static void RotateAroundInternal(TransformComponent& transform, const Math::Vector3& axis, Core::float32 angle);

		//=========================================================================
		// 내부 헬퍼 함수
		//=========================================================================

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

	private:
		/// Root Entity 목록 (parent가 Invalid인 HierarchyComponent를 가진 Entity)
		std::vector<Entity> mRootEntities;

		/// 빈 children 벡터 (GetChildren 반환용)
		static const std::vector<Entity> sEmptyChildren;

		//=========================================================================
		// TODO: [OPTIMIZATION] Phase 4+
		// - 현재: DFS 재귀 순회 + subtreeDirty 상향 전파
		// - 최적화: 비트마스크 dirty flag (dirty 종류 4개 이상 시)
		// - 최적화: 명시적 스택 (깊이 100+ 시)
		// - 최적화: Topological Sort 캐싱 (Entity 10,000+ 시)
		// - 최적화: 독립 서브트리 병렬 처리 (Job System 도입 후)
		//=========================================================================
	};

} // namespace ECS
