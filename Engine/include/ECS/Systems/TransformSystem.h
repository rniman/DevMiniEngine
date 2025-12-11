/**
 * @file TransformSystem.h
 * @brief Transform 관련 로직을 처리하는 System
 *
 * 생성자 주입 방식으로 Registry를 받습니다.
 * 고수준(Entity 기반)과 저수준(Component 직접) API를 모두 제공합니다.
 */
#pragma once
#include "ECS/ISystem.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Entity.h"
#include "Core/Types.h"

namespace ECS
{
	class Registry;

	/**
	 * @brief Transform System
	 *
	 * Transform 계층 구조 업데이트를 처리합니다.
	 * 고수준(Entity)과 저수준(Component) API를 모두 제공합니다.
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
		void Update(Core::float32 deltaTime) override;
		void Shutdown() override;

		//=========================================================================
		// 고수준 API (Entity 기반)
		//=========================================================================

		bool SetPosition(Entity entity, const Math::Vector3& position);
		bool GetPosition(Entity entity, Math::Vector3& outPosition);

		bool SetRotationEuler(Entity entity, const Math::Vector3& eulerAngles);
		bool SetRotation(Entity entity, const Math::Quaternion& rotation);
		bool GetRotationEuler(Entity entity, Math::Vector3& outEuler);

		bool SetScale(Entity entity, const Math::Vector3& scale);
		bool SetScale(Entity entity, Core::float32 uniformScale);

		bool Rotate(Entity entity, const Math::Vector3& eulerDelta);
		bool RotateAround(Entity entity, const Math::Vector3& axis, Core::float32 angle);
		bool Translate(Entity entity, const Math::Vector3& delta);

		bool GetWorldMatrix(Entity entity, Math::Matrix4x4& outMatrix);
		bool LookAt(Entity entity, const Math::Vector3& target, const Math::Vector3& up = Math::Vector3(0.0f, 1.0f, 0.0f));

		//=========================================================================
		// 저수준 API (Component 직접) - System 내부, 다른 System에서 호출
		//=========================================================================

		static void SetRotationEuler(TransformComponent& transform, const Math::Vector3& eulerAngles);
		static void SetRotationEuler(TransformComponent& transform, Core::float32 pitch, Core::float32 yaw, Core::float32 roll);
		static Math::Vector3 GetRotationEuler(const TransformComponent& transform);

		static void Rotate(TransformComponent& transform, const Math::Vector3& eulerDelta);
		static void RotateAround(TransformComponent& transform, const Math::Vector3& axis, Core::float32 angle);

		static Math::Matrix4x4 GetLocalMatrix(const TransformComponent& transform);
		static Math::Matrix4x4 GetWorldMatrix(const TransformComponent& transform);

		static Math::Vector3 GetForward(const TransformComponent& transform);
		static Math::Vector3 GetRight(const TransformComponent& transform);
		static Math::Vector3 GetUp(const TransformComponent& transform);

		//=========================================================================
		// Phase 3.5 예정 함수 (계층 구조)
		//=========================================================================
		// static Math::Matrix4x4 GetWorldMatrix(Registry& registry, Entity entity);
		// static void MarkDirty(Registry& registry, Entity entity);
		// static void MarkDirtyRecursive(Registry& registry, Entity entity);
		// static void UpdateWorldMatrices(Registry& registry);
	};

} // namespace ECS
