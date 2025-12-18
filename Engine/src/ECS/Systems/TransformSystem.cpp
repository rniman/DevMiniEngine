#include "pch.h"
// ECS
#include "ECS/Systems/TransformSystem.h"
#include "ECS/Archetype.h"
#include "ECS/Registry.h"
#include "ECS/RegistryView.h"

// Core
#include "Core/Logging/LogMacros.h"

// Math
#include "Math/MathUtils.h"
#include "Math/MathTypes.h"

// Standard Library
#include <algorithm>

namespace ECS
{
	//=============================================================================
	// 정적 멤버 초기화
	//=============================================================================

	const std::vector<Entity> TransformSystem::sEmptyChildren;

	//=============================================================================
	// 생성자
	//=============================================================================

	TransformSystem::TransformSystem(Registry& registry)
		: ISystem(registry)
	{
	}

	//=============================================================================
	// ISystem 인터페이스 구현
	//=============================================================================

	void TransformSystem::Initialize()
	{
		LOG_INFO("[TransformSystem] Initialized");
	}

	void TransformSystem::Update(Core::float32 deltaTime)
	{
		(void)deltaTime;

		// 1. Root Entity부터 DFS 순회하며 계층 구조 업데이트
		for (Entity root : mRootEntities)
		{
			if (!GetRegistry()->IsEntityValid(root))
			{
				continue;
			}

			UpdateHierarchy(root, Math::Matrix4x4::Identity());
		}

		// 2. HierarchyComponent 없는 Entity들 단독 처리
		UpdateStandaloneEntities();
	}

	void TransformSystem::Shutdown()
	{
		mRootEntities.clear();
		LOG_INFO("[TransformSystem] Shutdown");
	}

	//=============================================================================
	// 계층 구조 API
	//=============================================================================

	bool TransformSystem::SetParent(Entity child, Entity parent)
	{
		Registry* registry = GetRegistry();

		// child 유효성 검사
		if (!registry->IsEntityValid(child))
		{
			LOG_WARN("[TransformSystem] SetParent: Invalid child entity");
			return false;
		}

		// child에 HierarchyComponent 필요
		auto* childHierarchy = registry->GetComponent<HierarchyComponent>(child);
		if (!childHierarchy)
		{
			LOG_WARN("[TransformSystem] SetParent: Child entity %u has no HierarchyComponent", child.id);
			return false;
		}

		// 기존 부모에서 제거
		Entity oldParent = childHierarchy->parent;
		if (oldParent.IsValid())
		{
			auto* oldParentHierarchy = registry->GetComponent<HierarchyComponent>(oldParent);
			if (oldParentHierarchy)
			{
				auto& children = oldParentHierarchy->children;
				children.erase(
					std::remove(children.begin(), children.end(), child),
					children.end()
				);
			}
		}
		else
		{
			// 기존에 Root였으면 Root 목록에서 제거
			RemoveRootEntity(child);
		}

		// 새 부모 설정
		if (parent.IsValid())
		{
			// parent 유효성 검사
			if (!registry->IsEntityValid(parent))
			{
				LOG_WARN("[TransformSystem] SetParent: Invalid parent entity");
				childHierarchy->parent = Entity::Invalid();
				AddRootEntity(child);
				return false;
			}

			// parent에 HierarchyComponent 필요
			auto* parentHierarchy = registry->GetComponent<HierarchyComponent>(parent);
			if (!parentHierarchy)
			{
				LOG_WARN("[TransformSystem] SetParent: Parent entity %u has no HierarchyComponent", parent.id);
				childHierarchy->parent = Entity::Invalid();
				AddRootEntity(child);
				return false;
			}

			// 순환 참조 방지 (parent가 child의 자손인지 확인)
			Entity ancestor = parent;
			while (ancestor.IsValid())
			{
				if (ancestor == child)
				{
					LOG_WARN("[TransformSystem] SetParent: Circular hierarchy detected");
					childHierarchy->parent = Entity::Invalid();
					AddRootEntity(child);
					return false;
				}

				auto* ancestorHierarchy = registry->GetComponent<HierarchyComponent>(ancestor);
				ancestor = ancestorHierarchy ? ancestorHierarchy->parent : Entity::Invalid();
			}

			// 새 부모의 children에 추가
			parentHierarchy->children.push_back(child);
			childHierarchy->parent = parent;
		}
		else
		{
			// parent가 Invalid면 Root로 설정
			childHierarchy->parent = Entity::Invalid();
			AddRootEntity(child);
		}

		// child의 worldDirty 설정
		auto* childTransform = registry->GetComponent<TransformComponent>(child);
		if (childTransform)
		{
			childTransform->worldDirty = true;
		}

		return true;
	}

	Entity TransformSystem::GetParent(Entity entity) const
	{
		const auto* hierarchy = GetRegistry()->GetComponent<HierarchyComponent>(entity);
		return hierarchy ? hierarchy->parent : Entity::Invalid();
	}

	const std::vector<Entity>& TransformSystem::GetChildren(Entity entity) const
	{
		const auto* hierarchy = GetRegistry()->GetComponent<HierarchyComponent>(entity);
		return hierarchy ? hierarchy->children : sEmptyChildren;
	}

	bool TransformSystem::IsRoot(Entity entity) const
	{
		const auto* hierarchy = GetRegistry()->GetComponent<HierarchyComponent>(entity);
		return hierarchy && !hierarchy->parent.IsValid();
	}

	//=============================================================================
	// 고수준 API (Entity 기반)
	//=============================================================================

	bool TransformSystem::SetPosition(Entity entity, const Math::Vector3& position)
	{
		auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		transform->position = position;
		MarkLocalDirty(*transform);
		return true;
	}

	bool TransformSystem::GetPosition(Entity entity, Math::Vector3& outPosition) const
	{
		const auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		outPosition = transform->position;
		return true;
	}

	bool TransformSystem::SetRotationEuler(Entity entity, const Math::Vector3& eulerAngles)
	{
		auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		SetRotationEuler(*transform, eulerAngles);
		MarkLocalDirty(*transform);
		return true;
	}

	bool TransformSystem::SetRotation(Entity entity, const Math::Quaternion& rotation)
	{
		auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		transform->rotation = rotation.Normalized();
		MarkLocalDirty(*transform);
		return true;
	}

	bool TransformSystem::GetRotationEuler(Entity entity, Math::Vector3& outEuler) const
	{
		const auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		outEuler = GetRotationEuler(*transform);
		return true;
	}

	bool TransformSystem::SetScale(Entity entity, const Math::Vector3& scale)
	{
		auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		transform->scale = scale;
		MarkLocalDirty(*transform);
		return true;
	}

	bool TransformSystem::SetScale(Entity entity, Core::float32 uniformScale)
	{
		return SetScale(entity, Math::Vector3(uniformScale));
	}

	bool TransformSystem::Rotate(Entity entity, const Math::Vector3& eulerDelta)
	{
		auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		Rotate(*transform, eulerDelta);
		MarkLocalDirty(*transform);
		return true;
	}

	bool TransformSystem::RotateAround(Entity entity, const Math::Vector3& axis, Core::float32 angle)
	{
		auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		RotateAround(*transform, axis, angle);
		MarkLocalDirty(*transform);
		return true;
	}

	bool TransformSystem::Translate(Entity entity, const Math::Vector3& delta)
	{
		auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		transform->position += delta;
		MarkLocalDirty(*transform);
		return true;
	}

	bool TransformSystem::GetWorldMatrix(Entity entity, Math::Matrix4x4& outMatrix) const
	{
		const auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		outMatrix = transform->worldMatrix;
		return true;
	}

	bool TransformSystem::GetLocalMatrix(Entity entity, Math::Matrix4x4& outMatrix) const
	{
		const auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		outMatrix = transform->localMatrix;
		return true;
	}

	bool TransformSystem::GetWorldInvTranspose(Entity entity, Math::Matrix4x4& outMatrix) const
	{
		const auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		outMatrix = GetWorldInvTranspose(*transform);
		return true;
	}

	bool TransformSystem::LookAt(Entity entity, const Math::Vector3& target, const Math::Vector3& up)
	{
		auto* transform = GetRegistry()->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return false;
		}

		Math::Vector3 forward = (target - transform->position).Normalized();
		Math::Vector3 right = up.Cross(forward).Normalized();
		Math::Vector3 adjustedUp = forward.Cross(right);

		Math::Matrix4x4 rotMatrix = Math::Matrix4x4::Identity();
		rotMatrix.m[0][0] = right.x;
		rotMatrix.m[0][1] = right.y;
		rotMatrix.m[0][2] = right.z;
		rotMatrix.m[1][0] = adjustedUp.x;
		rotMatrix.m[1][1] = adjustedUp.y;
		rotMatrix.m[1][2] = adjustedUp.z;
		rotMatrix.m[2][0] = forward.x;
		rotMatrix.m[2][1] = forward.y;
		rotMatrix.m[2][2] = forward.z;

		transform->rotation = Math::QuaternionFromRotationMatrix(rotMatrix);
		MarkLocalDirty(*transform);
		return true;
	}

	void TransformSystem::ForceUpdateWorldMatrix(Entity entity)
	{
		Registry* registry = GetRegistry();

		auto* transform = registry->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return;
		}

		// Local 업데이트
		UpdateLocalMatrix(*transform);

		// Parent 체인을 따라 올라가며 World Matrix 수집
		const auto* hierarchy = registry->GetComponent<HierarchyComponent>(entity);
		if (hierarchy && hierarchy->parent.IsValid())
		{
			// Parent의 World Matrix가 필요하므로 재귀적으로 업데이트
			ForceUpdateWorldMatrix(hierarchy->parent);

			const auto* parentTransform = registry->GetComponent<TransformComponent>(hierarchy->parent);
			if (parentTransform)
			{
				transform->worldMatrix = transform->localMatrix * parentTransform->worldMatrix;
			}
			else
			{
				transform->worldMatrix = transform->localMatrix;
			}
		}
		else
		{
			// Root이거나 Hierarchy 없음
			transform->worldMatrix = transform->localMatrix;
		}

		transform->worldDirty = false;
	}

	//=============================================================================
	// 저수준 API (Component 직접) - 정적
	//=============================================================================

	void TransformSystem::SetRotationEuler(TransformComponent& transform, const Math::Vector3& eulerAngles)
	{
		transform.rotation = Math::QuaternionFromEuler(eulerAngles);
	}

	void TransformSystem::SetRotationEuler(
		TransformComponent& transform,
		Core::float32 pitch,
		Core::float32 yaw,
		Core::float32 roll
	)
	{
		transform.rotation = Math::QuaternionFromEuler(pitch, yaw, roll);
	}

	Math::Vector3 TransformSystem::GetRotationEuler(const TransformComponent& transform)
	{
		return transform.rotation.ToEuler();
	}

	void TransformSystem::Rotate(TransformComponent& transform, const Math::Vector3& eulerDelta)
	{
		Math::Quaternion delta = Math::QuaternionFromEuler(eulerDelta);
		transform.rotation = (transform.rotation * delta).Normalized();
	}

	void TransformSystem::RotateAround(TransformComponent& transform, const Math::Vector3& axis, Core::float32 angle)
	{
		Math::Quaternion delta = Math::QuaternionFromAxisAngle(axis, angle);
		transform.rotation = (transform.rotation * delta).Normalized();
	}

	Math::Matrix4x4 TransformSystem::CalculateLocalMatrix(const TransformComponent& transform)
	{
		Math::Matrix4x4 scaleMatrix = Math::MatrixScaling(transform.scale);
		Math::Matrix4x4 rotationMatrix = Math::MatrixRotationQuaternion(transform.rotation);
		Math::Matrix4x4 translationMatrix = Math::MatrixTranslation(transform.position);

		return scaleMatrix * rotationMatrix * translationMatrix;
	}

	const Math::Matrix4x4& TransformSystem::GetLocalMatrix(const TransformComponent& transform)
	{
		return transform.localMatrix;
	}

	const Math::Matrix4x4& TransformSystem::GetWorldMatrix(const TransformComponent& transform)
	{
		return transform.worldMatrix;
	}

	Math::Matrix4x4 TransformSystem::GetWorldInvTranspose(const TransformComponent& transform)
	{
		// 균등 스케일 체크
		bool isUniform =
			std::abs(transform.scale.x - transform.scale.y) < Math::EPSILON &&
			std::abs(transform.scale.y - transform.scale.z) < Math::EPSILON;

		if (isUniform)
		{
			// 균등 스케일: 역전치 불필요
			return transform.worldMatrix;
		}

		// 비균등 스케일: 역전치 계산
		return Math::MatrixTranspose(Math::MatrixInverse(transform.worldMatrix));
	}

	Math::Vector3 TransformSystem::GetForward(const TransformComponent& transform)
	{
		return transform.rotation.GetForward();
	}

	Math::Vector3 TransformSystem::GetRight(const TransformComponent& transform)
	{
		return transform.rotation.GetRight();
	}

	Math::Vector3 TransformSystem::GetUp(const TransformComponent& transform)
	{
		return transform.rotation.GetUp();
	}

	//=============================================================================
	// 내부 헬퍼 함수
	//=============================================================================

	void TransformSystem::AddRootEntity(Entity entity)
	{
		// 이미 있는지 확인
		auto it = std::find(mRootEntities.begin(), mRootEntities.end(), entity);
		if (it == mRootEntities.end())
		{
			mRootEntities.push_back(entity);
		}
	}

	void TransformSystem::RemoveRootEntity(Entity entity)
	{
		mRootEntities.erase(
			std::remove(mRootEntities.begin(), mRootEntities.end(), entity),
			mRootEntities.end()
		);
	}

	void TransformSystem::UpdateLocalMatrix(TransformComponent& transform)
	{
		if (transform.localDirty)
		{
			transform.localMatrix = CalculateLocalMatrix(transform);
			transform.localDirty = false;
			transform.worldDirty = true;  // Local 변경 시 World도 갱신 필요
		}
	}

	void TransformSystem::UpdateHierarchy(Entity entity, const Math::Matrix4x4& parentWorldMatrix)
	{
		Registry* registry = GetRegistry();

		auto* transform = registry->GetComponent<TransformComponent>(entity);
		if (!transform)
		{
			return;
		}

		// dirty 아닌 서브트리는 스킵
		if (!transform->localDirty && !transform->worldDirty)
		{
			// 자식들도 dirty 아니면 전체 스킵
			const auto* hierarchy = registry->GetComponent<HierarchyComponent>(entity);
			if (hierarchy)
			{
				bool anyChildDirty = false;
				for (Entity child : hierarchy->children)
				{
					auto* childTransform = registry->GetComponent<TransformComponent>(child);
					if (childTransform && (childTransform->localDirty || childTransform->worldDirty))
					{
						anyChildDirty = true;
						break;
					}
				}

				if (!anyChildDirty)
				{
					return;  // 이 서브트리 전체 스킵
				}
			}
			else
			{
				return;  // 자식 없고 dirty 아니면 스킵
			}
		}

		// Local 업데이트
		UpdateLocalMatrix(*transform);

		// World 업데이트
		if (transform->worldDirty)
		{
			transform->worldMatrix = transform->localMatrix * parentWorldMatrix;
			transform->worldDirty = false;
		}

		// 자식 처리
		const auto* hierarchy = registry->GetComponent<HierarchyComponent>(entity);
		if (hierarchy)
		{
			for (Entity child : hierarchy->children)
			{
				if (!registry->IsEntityValid(child))
				{
					continue;
				}

				// 자식의 worldDirty 설정 (부모가 업데이트되었으므로)
				auto* childTransform = registry->GetComponent<TransformComponent>(child);
				if (childTransform)
				{
					childTransform->worldDirty = true;
				}

				// 재귀 호출
				UpdateHierarchy(child, transform->worldMatrix);
			}
		}
	}

	void TransformSystem::UpdateStandaloneEntities()
	{
		Registry* registry = GetRegistry();

		auto view = TransformOnlyArchetype::CreateView(*registry);
		for (Entity entity : view)
		{
			// HierarchyComponent가 있으면 이미 계층 구조에서 처리됨
			if (registry->HasComponent<HierarchyComponent>(entity))
			{
				continue;
			}

			auto* transform = registry->GetComponent<TransformComponent>(entity);
			if (!transform)
			{
				continue;
			}

			// dirty 아니면 스킵
			if (!transform->localDirty && !transform->worldDirty)
			{
				continue;
			}

			// Local 업데이트
			UpdateLocalMatrix(*transform);

			// World = Local (계층 없음)
			if (transform->worldDirty)
			{
				transform->worldMatrix = transform->localMatrix;
				transform->worldDirty = false;
			}
		}
	}

	void TransformSystem::MarkLocalDirty(TransformComponent& transform)
	{
		transform.localDirty = true;
		transform.worldDirty = true;
	}

} // namespace ECS
