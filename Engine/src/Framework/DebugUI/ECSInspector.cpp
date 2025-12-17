#include "pch.h"
#include "Framework/DebugUI/ECSInspector.h"

// ECS
#include "ECS/Registry.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/LightComponents.h"
#include "ECS/Components/MeshComponent.h"
#include "ECS/Components/MaterialComponent.h"

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
			mSelectedEntity = newEntity;
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
			mSelectedEntity = ECS::Entity{};
		}

		if (!canDelete)
		{
			ImGui::EndDisabled();
		}

		ImGui::Separator();

		// 필터
		ImGui::InputTextWithHint("##Filter", "Filter...", mEntityFilter, sizeof(mEntityFilter));
		ImGui::Separator();

		// Entity 목록
		const std::vector<ECS::Entity>& entities = registry->GetAllEntities();

		for (const ECS::Entity& entity : entities)
		{
			if (!entity.IsValid())
			{
				continue;
			}

			char entityName[32];
			snprintf(entityName, sizeof(entityName), "Entity %u", entity.id);

			if (mEntityFilter[0] != '\0')
			{
				if (strstr(entityName, mEntityFilter) == nullptr)
				{
					continue;
				}
			}

			bool isSelected = (mSelectedEntity == entity);
			if (ImGui::Selectable(entityName, isSelected))
			{
				mSelectedEntity = entity;
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
			mSelectedEntity = ECS::Entity{};
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
		}

		// Rotation (Euler)
		Math::Vector3 euler = Math::Vector3EulerFromQuaternion(transform->rotation);
		float rot[3] = {
			Math::RadToDeg(euler.x),
			Math::RadToDeg(euler.y),
			Math::RadToDeg(euler.z)
		};
		if (ImGui::DragFloat3("Rotation", rot, 1.0f, -180.0f, 180.0f))
		{
			transform->rotation = Math::QuaternionFromEuler(
				Math::Vector3(
					Math::DegToRad(rot[0]),
					Math::DegToRad(rot[1]),
					Math::DegToRad(rot[2])
				)
			);
		}

		// Scale
		float scale[3] = { transform->scale.x, transform->scale.y, transform->scale.z };
		if (ImGui::DragFloat3("Scale", scale, 0.01f, 0.01f, 100.0f))
		{
			transform->scale = Math::Vector3(scale[0], scale[1], scale[2]);
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

		const char* projTypes[] = { "Perspective", "Orthographic" };
		int projIndex = static_cast<int>(camera->projectionType);
		if (ImGui::Combo("Projection", &projIndex, projTypes, 2))
		{
			camera->projectionType = static_cast<ECS::ProjectionType>(projIndex);
		}

		if (camera->projectionType == ECS::ProjectionType::Perspective)
		{
			float fovDeg = Math::RadToDeg(camera->fovY);
			if (ImGui::SliderFloat("FOV", &fovDeg, 30.0f, 120.0f))
			{
				camera->fovY = Math::DegToRad(fovDeg);
			}
		}

		ImGui::DragFloat("Near Clip", &camera->nearPlane, 0.01f, 0.001f, camera->farPlane - 0.1f);
		ImGui::DragFloat("Far Clip", &camera->farPlane, 1.0f, camera->nearPlane + 0.1f, 10000.0f);
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

		ImGui::Text("Material ID: 0x%llX", material->materialId.IsValid() ? material->materialId.id : 0);
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

} // namespace Framework
