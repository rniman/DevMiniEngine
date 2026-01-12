#include "pch.h"
#include "Framework/DebugUI/ECSInspector.h"

// ECS
#include "ECS/Registry.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/LightComponents.h"
#include "ECS/Components/MeshComponent.h"
#include "ECS/Components/MaterialComponent.h"
#include "ECS/Components/HierarchyComponent.h"
#include "ECS/Systems/TransformSystem.h"

// Math
#include "Math/MathUtils.h"

// ThirdParty - ImGui
#include <imgui.h>

namespace Framework
{
	void ECSInspector::Render(ECS::Registry* registry)
	{
		if (!mIsVisible || !registry)
		{
			return;
		}

		// Entity List 패널
		ImGui::SetNextWindowPos(ImVec2(10, 220), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(250, 300), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Entity List", &mIsVisible))
		{
			RenderEntityList(registry);
		}
		ImGui::End();

		// Component Inspector 패널
		ImGui::SetNextWindowPos(ImVec2(270, 220), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Inspector"))
		{
			RenderComponentInspector(registry);
		}
		ImGui::End();

		// 예약된 Component 삭제 처리
		if (mSelectedEntity.IsValid() && registry->IsEntityValid(mSelectedEntity))
		{
			if (mPendingDeleteTransform)
			{
				registry->RemoveComponent<ECS::TransformComponent>(mSelectedEntity);
				mPendingDeleteTransform = false;
			}
			if (mPendingDeleteCamera)
			{
				registry->RemoveComponent<ECS::CameraComponent>(mSelectedEntity);
				mPendingDeleteCamera = false;
			}
			if (mPendingDeleteDirLight)
			{
				registry->RemoveComponent<ECS::DirectionalLightComponent>(mSelectedEntity);
				mPendingDeleteDirLight = false;
			}
			if (mPendingDeletePointLight)
			{
				registry->RemoveComponent<ECS::PointLightComponent>(mSelectedEntity);
				mPendingDeletePointLight = false;
			}
			if (mPendingDeleteMesh)
			{
				registry->RemoveComponent<ECS::MeshComponent>(mSelectedEntity);
				mPendingDeleteMesh = false;
			}
			if (mPendingDeleteMaterial)
			{
				registry->RemoveComponent<ECS::MaterialComponent>(mSelectedEntity);
				mPendingDeleteMaterial = false;
			}
		}
	}

	void ECSInspector::RenderEntityList(ECS::Registry* registry)
	{
		// Entity 생성 버튼
		if (ImGui::Button("+ Create Entity"))
		{
			ECS::Entity newEntity = registry->CreateEntity();
			SelectEntity(newEntity);
		}

		ImGui::SameLine();

		// Entity 삭제 버튼
		bool canDelete = mSelectedEntity.IsValid() && registry->IsEntityValid(mSelectedEntity);
		if (!canDelete)
		{
			ImGui::BeginDisabled();
		}

		if (ImGui::Button("- Delete"))
		{
			registry->DestroyEntity(mSelectedEntity);
			SelectEntity(ECS::Entity{});
		}

		if (!canDelete)
		{
			ImGui::EndDisabled();
		}

		ImGui::Separator();

		// 뷰 모드 토글
		ImGui::Checkbox("Hierarchy View", &mShowHierarchyView);

		// 필터
		ImGui::InputTextWithHint("##Filter", "Filter...", mEntityFilter, sizeof(mEntityFilter));
		ImGui::Separator();

		if (mShowHierarchyView)
		{
			// =========================================
			// 계층 뷰 모드
			// =========================================

			// 1. 루트 Entity들 (부모 없는 HierarchyComponent)
			std::vector<ECS::Entity> rootEntities = CollectRootEntities(registry);

			if (!rootEntities.empty())
			{
				ImGui::TextDisabled("-- Hierarchy --");
				for (ECS::Entity root : rootEntities)
				{
					RenderEntityTreeNode(registry, root);
				}
			}

			// 2. 계층 없는 Entity들 (HierarchyComponent 없음)
			std::vector<ECS::Entity> flatEntities = CollectFlatEntities(registry);

			if (!flatEntities.empty())
			{
				ImGui::Separator();
				ImGui::TextDisabled("-- Standalone --");

				for (ECS::Entity entity : flatEntities)
				{
					std::string displayName = GetEntityDisplayName(registry, entity);

					// 필터 적용
					if (mEntityFilter[0] != '\0')
					{
						if (displayName.find(mEntityFilter) == std::string::npos)
						{
							continue;
						}
					}

					bool isSelected = (mSelectedEntity == entity);
					if (ImGui::Selectable(displayName.c_str(), isSelected))
					{
						if (isSelected)
						{
							SelectEntity(ECS::Entity{});
						}
						else
						{
							SelectEntity(entity);
						}
					}
				}
			}
		}
		else
		{
			// =========================================
			// 플랫 뷰 모드 (기존 방식)
			// =========================================

			const std::vector<ECS::Entity>& entities = registry->GetAllEntities();

			for (const ECS::Entity& entity : entities)
			{
				if (!entity.IsValid())
				{
					continue;
				}

				std::string displayName = GetEntityDisplayName(registry, entity);

				// 필터 적용
				if (mEntityFilter[0] != '\0')
				{
					if (displayName.find(mEntityFilter) == std::string::npos)
					{
						continue;
					}
				}

				bool isSelected = (mSelectedEntity == entity);
				if (ImGui::Selectable(displayName.c_str(), isSelected))
				{
					if (isSelected)
					{
						SelectEntity(ECS::Entity{});
					}
					else
					{
						SelectEntity(entity);
					}
				}
			}
		}
	}


	void ECSInspector::RenderComponentInspector(ECS::Registry* registry)
	{
		if (!mSelectedEntity.IsValid())
		{
			ImGui::TextDisabled("No entity selected");
			return;
		}

		if (!registry->IsEntityValid(mSelectedEntity))
		{
			ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Invalid Entity");
			SelectEntity(ECS::Entity{});
			return;
		}

		ImGui::Text("Entity %u (v%u)", mSelectedEntity.id, mSelectedEntity.version);
		ImGui::Separator();

		RenderAddComponentMenu(registry, mSelectedEntity);
		ImGui::Separator();

		if (registry->HasComponent<ECS::TransformComponent>(mSelectedEntity))
		{
			RenderTransformComponent(registry, mSelectedEntity);
		}

		if (registry->HasComponent<ECS::CameraComponent>(mSelectedEntity))
		{
			RenderCameraComponent(registry, mSelectedEntity);
		}

		if (registry->HasComponent<ECS::DirectionalLightComponent>(mSelectedEntity))
		{
			RenderDirectionalLightComponent(registry, mSelectedEntity);
		}

		if (registry->HasComponent<ECS::PointLightComponent>(mSelectedEntity))
		{
			RenderPointLightComponent(registry, mSelectedEntity);
		}

		if (registry->HasComponent<ECS::MeshComponent>(mSelectedEntity))
		{
			RenderMeshComponent(registry, mSelectedEntity);
		}

		if (registry->HasComponent<ECS::MaterialComponent>(mSelectedEntity))
		{
			RenderMaterialComponent(registry, mSelectedEntity);
		}
	}

	void ECSInspector::SelectEntity(ECS::Entity entity)
	{
		// 동일한 Entity면 무시
		if (mSelectedEntity == entity)
		{
			return;
		}

		mSelectedEntity = entity;

		// 콜백 호출
		if (mSelectionChangedCallback)
		{
			mSelectionChangedCallback(entity);
		}
	}

	void ECSInspector::RenderTransformComponent(ECS::Registry* registry, ECS::Entity entity)
	{
		bool isOpen = ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap);

		ImGui::SameLine(ImGui::GetWindowWidth() - 25);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
		if (ImGui::SmallButton("X##Transform"))
		{
			mPendingDeleteTransform = true;
		}
		ImGui::PopStyleColor(2);

		if (!isOpen)
		{
			return;
		}

		ECS::TransformComponent* transform = registry->GetComponent<ECS::TransformComponent>(entity);
		if (!transform)
		{
			return;
		}

		// Position
		float pos[3] = { transform->position.x, transform->position.y, transform->position.z };
		if (ImGui::DragFloat3("Position", pos, 0.1f))
		{
			transform->position = Math::Vector3(pos[0], pos[1], pos[2]);
			ECS::TransformSystem::MarkDirty(*transform);
		}

		// Rotation - eulerHint 사용 (Quaternion 변환 불안정성 방지)
		float rot[3] = {
			Math::RadToDeg(transform->eulerHint.x),
			Math::RadToDeg(transform->eulerHint.y),
			Math::RadToDeg(transform->eulerHint.z)
		};

		if (ImGui::DragFloat3("Rotation", rot, 1.0f))
		{
			transform->eulerHint = Math::Vector3(
				Math::DegToRad(rot[0]),
				Math::DegToRad(rot[1]),
				Math::DegToRad(rot[2])
			);
			transform->rotation = Math::QuaternionFromEuler(transform->eulerHint);
			ECS::TransformSystem::MarkDirty(*transform);
		}

		// Scale
		float scale[3] = { transform->scale.x, transform->scale.y, transform->scale.z };
		if (ImGui::DragFloat3("Scale", scale, 0.01f, 0.01f, 100.0f))
		{
			transform->scale = Math::Vector3(scale[0], scale[1], scale[2]);
			ECS::TransformSystem::MarkDirty(*transform);
		}
	}

	void ECSInspector::RenderCameraComponent(ECS::Registry* registry, ECS::Entity entity)
	{
		bool isOpen = ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_AllowOverlap);

		ImGui::SameLine(ImGui::GetWindowWidth() - 25);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
		if (ImGui::SmallButton("X##Camera"))
		{
			mPendingDeleteCamera = true;
		}
		ImGui::PopStyleColor(2);

		if (!isOpen)
		{
			return;
		}

		ECS::CameraComponent* camera = registry->GetComponent<ECS::CameraComponent>(entity);
		if (!camera)
		{
			return;
		}

		// Projection Type
		const char* projTypes[] = { "Perspective", "Orthographic" };
		int projIndex = static_cast<int>(camera->projectionType);
		if (ImGui::Combo("Projection", &projIndex, projTypes, 2))
		{
			camera->projectionType = static_cast<ECS::ProjectionType>(projIndex);
		}

		// FOV (Perspective only)
		if (camera->projectionType == ECS::ProjectionType::Perspective)
		{
			float fovDeg = Math::RadToDeg(camera->fovY);
			if (ImGui::SliderFloat("FOV", &fovDeg, 30.0f, 120.0f))
			{
				camera->fovY = Math::DegToRad(fovDeg);
			}
		}

		// Clip Planes
		ImGui::DragFloat("Near Clip", &camera->nearPlane, 0.01f, 0.001f, camera->farPlane - 0.1f);
		ImGui::DragFloat("Far Clip", &camera->farPlane, 1.0f, camera->nearPlane + 0.1f, 10000.0f);

		ImGui::Separator();

		// Up Mode
		const char* upModes[] = { "World Up", "Local Up" };
		int upModeIndex = static_cast<int>(camera->upMode);
		if (ImGui::Combo("Up Mode", &upModeIndex, upModes, 2))
		{
			camera->upMode = static_cast<ECS::CameraUpMode>(upModeIndex);
		}

		// Up Mode 설명
		if (camera->upMode == ECS::CameraUpMode::WorldUp)
		{
			ImGui::TextDisabled("FPS/TPS style - Y axis fixed");
		}
		else
		{
			ImGui::TextDisabled("Flight sim style - roll enabled");
		}

		ImGui::Separator();

		// Main Camera
		ImGui::Checkbox("Main Camera", &camera->isMainCamera);
	}

	void ECSInspector::RenderDirectionalLightComponent(ECS::Registry* registry, ECS::Entity entity)
	{
		bool isOpen = ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_AllowOverlap);

		ImGui::SameLine(ImGui::GetWindowWidth() - 25);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
		if (ImGui::SmallButton("X##DirLight"))
		{
			mPendingDeleteDirLight = true;
		}
		ImGui::PopStyleColor(2);

		if (!isOpen)
		{
			return;
		}

		ECS::DirectionalLightComponent* light = registry->GetComponent<ECS::DirectionalLightComponent>(entity);
		if (!light)
		{
			return;
		}

		float dir[3] = { light->direction.x, light->direction.y, light->direction.z };
		if (ImGui::DragFloat3("Direction", dir, 0.01f, -1.0f, 1.0f))
		{
			light->direction = Math::Normalize(Math::Vector3(dir[0], dir[1], dir[2]));
		}

		float color[3] = { light->color.x, light->color.y, light->color.z };
		if (ImGui::ColorEdit3("Color", color))
		{
			light->color = Math::Vector3(color[0], color[1], color[2]);
		}

		ImGui::SliderFloat("Intensity", &light->intensity, 0.0f, 10.0f);
	}

	void ECSInspector::RenderPointLightComponent(ECS::Registry* registry, ECS::Entity entity)
	{
		bool isOpen = ImGui::CollapsingHeader("Point Light", ImGuiTreeNodeFlags_AllowOverlap);

		ImGui::SameLine(ImGui::GetWindowWidth() - 25);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
		if (ImGui::SmallButton("X##PointLight"))
		{
			mPendingDeletePointLight = true;
		}
		ImGui::PopStyleColor(2);

		if (!isOpen)
		{
			return;
		}

		ECS::PointLightComponent* light = registry->GetComponent<ECS::PointLightComponent>(entity);
		if (!light)
		{
			return;
		}

		float color[3] = { light->color.x, light->color.y, light->color.z };
		if (ImGui::ColorEdit3("Color", color))
		{
			light->color = Math::Vector3(color[0], color[1], color[2]);
		}

		ImGui::SliderFloat("Intensity", &light->intensity, 0.0f, 20.0f);
		ImGui::SliderFloat("Range", &light->range, 0.1f, 100.0f);

		if (ImGui::TreeNode("Attenuation"))
		{
			ImGui::DragFloat("Constant", &light->constant, 0.01f, 0.0f, 2.0f);
			ImGui::DragFloat("Linear", &light->linear, 0.001f, 0.0f, 1.0f);
			ImGui::DragFloat("Quadratic", &light->quadratic, 0.0001f, 0.0f, 0.1f);
			ImGui::TreePop();
		}
	}

	void ECSInspector::RenderMeshComponent(ECS::Registry* registry, ECS::Entity entity)
	{
		bool isOpen = ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_AllowOverlap);

		ImGui::SameLine(ImGui::GetWindowWidth() - 25);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
		if (ImGui::SmallButton("X##Mesh"))
		{
			mPendingDeleteMesh = true;
		}
		ImGui::PopStyleColor(2);

		if (!isOpen)
		{
			return;
		}

		ECS::MeshComponent* mesh = registry->GetComponent<ECS::MeshComponent>(entity);
		if (!mesh)
		{
			return;
		}

		ImGui::Text("Mesh ID: 0x%llX", mesh->meshId.IsValid() ? mesh->meshId.id : 0);
		ImGui::TextDisabled("(Read-only)");
	}

	void ECSInspector::RenderMaterialComponent(ECS::Registry* registry, ECS::Entity entity)
	{
		bool isOpen = ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_AllowOverlap);

		ImGui::SameLine(ImGui::GetWindowWidth() - 25);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
		if (ImGui::SmallButton("X##Material"))
		{
			mPendingDeleteMaterial = true;
		}
		ImGui::PopStyleColor(2);

		if (!isOpen)
		{
			return;
		}

		ECS::MaterialComponent* material = registry->GetComponent<ECS::MaterialComponent>(entity);
		if (!material)
		{
			return;
		}

		ImGui::Text("Material Count: %u / %u", material->count, ECS::MaterialComponent::MAX_MATERIALS);
		ImGui::Separator();

		for (Core::uint32 i = 0; i < material->count; ++i)
		{
			ImGui::Text(
				"[%u] ID: 0x%llX",
				i,
				material->materialIds[i].IsValid() ? material->materialIds[i].id : 0
			);
		}

		ImGui::TextDisabled("(Read-only)");

	}

	void ECSInspector::RenderAddComponentMenu(ECS::Registry* registry, ECS::Entity entity)
	{
		if (ImGui::Button("+ Add Component"))
		{
			ImGui::OpenPopup("AddComponentPopup");
		}

		if (ImGui::BeginPopup("AddComponentPopup"))
		{
			if (!registry->HasComponent<ECS::TransformComponent>(entity))
			{
				if (ImGui::MenuItem("Transform"))
				{
					registry->AddComponent(entity, ECS::TransformComponent{});
				}
			}

			if (!registry->HasComponent<ECS::CameraComponent>(entity))
			{
				if (ImGui::MenuItem("Camera"))
				{
					registry->AddComponent(entity, ECS::CameraComponent{});
				}
			}

			if (!registry->HasComponent<ECS::DirectionalLightComponent>(entity))
			{
				if (ImGui::MenuItem("Directional Light"))
				{
					registry->AddComponent(entity, ECS::DirectionalLightComponent{});
				}
			}

			if (!registry->HasComponent<ECS::PointLightComponent>(entity))
			{
				if (ImGui::MenuItem("Point Light"))
				{
					registry->AddComponent(entity, ECS::PointLightComponent{});
				}
			}

			if (!registry->HasComponent<ECS::MeshComponent>(entity))
			{
				if (ImGui::MenuItem("Mesh"))
				{
					registry->AddComponent(entity, ECS::MeshComponent{});
				}
			}

			if (!registry->HasComponent<ECS::MaterialComponent>(entity))
			{
				if (ImGui::MenuItem("Material"))
				{
					registry->AddComponent(entity, ECS::MaterialComponent{});
				}
			}

			ImGui::EndPopup();
		}
	}

	void ECSInspector::RenderEntityTreeNode(ECS::Registry* registry, ECS::Entity entity)
	{
		if (!entity.IsValid() || !registry->IsEntityValid(entity))
		{
			return;
		}

		// HierarchyComponent 가져오기
		ECS::HierarchyComponent* hierarchy = registry->GetComponent<ECS::HierarchyComponent>(entity);
		bool hasChildren = hierarchy && !hierarchy->children.empty();

		// 표시 이름
		std::string displayName = GetEntityDisplayName(registry, entity);

		// 필터 적용 (자식이 있으면 일단 보여줌)
		if (mEntityFilter[0] != '\0' && !hasChildren)
		{
			if (displayName.find(mEntityFilter) == std::string::npos)
			{
				return;
			}
		}

		// TreeNode 플래그 설정
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

		if (!hasChildren)
		{
			// 자식 없으면 리프 노드 (화살표 없음)
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}

		if (mSelectedEntity == entity)
		{
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		// TreeNode 렌더링
		bool isOpen = ImGui::TreeNodeEx(
			reinterpret_cast<void*>(static_cast<intptr_t>(entity.id)),
			flags,
			"%s",
			displayName.c_str()
		);

		// 클릭 처리
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
		{
			if (mSelectedEntity == entity)
			{
				SelectEntity(ECS::Entity{});
			}
			else
			{
				SelectEntity(entity);
			}
		}

		// 자식 렌더링 (펼쳐진 경우)
		if (isOpen && hasChildren)
		{
			for (ECS::Entity child : hierarchy->children)
			{
				RenderEntityTreeNode(registry, child);  // 재귀!
			}
			ImGui::TreePop();
		}
	}

	std::vector<ECS::Entity> ECSInspector::CollectRootEntities(ECS::Registry* registry)
	{
		std::vector<ECS::Entity> roots;

		const std::vector<ECS::Entity>& allEntities = registry->GetAllEntities();

		for (const ECS::Entity& entity : allEntities)
		{
			if (!entity.IsValid())
			{
				continue;
			}

			// HierarchyComponent가 있는 Entity만
			ECS::HierarchyComponent* hierarchy = registry->GetComponent<ECS::HierarchyComponent>(entity);
			if (!hierarchy)
			{
				continue;
			}

			// 부모가 없으면 루트
			if (!hierarchy->parent.IsValid())
			{
				roots.push_back(entity);
			}
		}

		return roots;
	}

	std::vector<ECS::Entity> ECSInspector::CollectFlatEntities(ECS::Registry* registry)
	{
		std::vector<ECS::Entity> flatEntities;

		const std::vector<ECS::Entity>& allEntities = registry->GetAllEntities();

		for (const ECS::Entity& entity : allEntities)
		{
			if (!entity.IsValid())
			{
				continue;
			}

			// HierarchyComponent가 없는 Entity
			if (!registry->HasComponent<ECS::HierarchyComponent>(entity))
			{
				flatEntities.push_back(entity);
			}
		}

		return flatEntities;
	}

	std::string ECSInspector::GetEntityDisplayName(ECS::Registry* registry, ECS::Entity entity)
	{
		// 기본: "Entity N"
		char buffer[64];
		snprintf(buffer, sizeof(buffer), "Entity %u", entity.id);

		// TODO: 나중에 NameComponent 추가하면 여기서 이름 표시
		// ECS::NameComponent* name = registry->GetComponent<ECS::NameComponent>(entity);
		// if (name && !name->name.empty())
		// {
		//     snprintf(buffer, sizeof(buffer), "%s (E%u)", name->name.c_str(), entity.id);
		// }

		return std::string(buffer);
	}

} // namespace Framework
