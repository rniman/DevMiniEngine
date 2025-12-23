#include "pch.h"
#include "Framework/DebugUI/DebugVisualizationPanel.h"
#include "Graphics/DebugDraw/DebugRenderer.h"
#include "Graphics/DebugDraw/DebugTypes.h"
#include <imgui.h>

namespace Framework
{
	void DebugVisualizationPanel::Render(Graphics::DebugRenderer* debugRenderer)
	{
		if (!mIsVisible || !debugRenderer)
		{
			return;
		}

		ImGui::SetNextWindowPos(ImVec2(630, 220), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(280, 250), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Debug Visualization", &mIsVisible))
		{
			ImGui::End();
			return;
		}

		auto& settings = debugRenderer->GetSettings();

		// 표시 모드
		ImGui::Text("Display Mode");

		int displayMode = static_cast<int>(settings.displayMode);
		if (ImGui::RadioButton("All", &displayMode, static_cast<int>(Graphics::DebugDisplayMode::All)))
		{
			settings.displayMode = Graphics::DebugDisplayMode::All;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Selected Only", &displayMode, static_cast<int>(Graphics::DebugDisplayMode::SelectedOnly)))
		{
			settings.displayMode = Graphics::DebugDisplayMode::SelectedOnly;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("None", &displayMode, static_cast<int>(Graphics::DebugDisplayMode::None)))
		{
			settings.displayMode = Graphics::DebugDisplayMode::None;
		}

		ImGui::Separator();

		// 개별 토글 (All 모드일 때만)
		if (settings.displayMode == Graphics::DebugDisplayMode::All)
		{
			ImGui::Text("Light Gizmos");
			ImGui::Checkbox("Directional Lights", &settings.showDirectionalLights);
			ImGui::Checkbox("Point Lights", &settings.showPointLights);
			ImGui::Separator();
		}

		// 렌더링 옵션
		ImGui::Text("Rendering");
		ImGui::Checkbox("Depth Test", &settings.depthTestEnabled);
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("ON: Occluded by objects\nOFF: Always visible (X-Ray)");
		}

		ImGui::Separator();

		// 색상 설정
		ImGui::Text("Colors");
		ImGui::ColorEdit3("Directional", &settings.directionalLightColor.x, ImGuiColorEditFlags_NoInputs);
		ImGui::ColorEdit3("Point", &settings.pointLightColor.x, ImGuiColorEditFlags_NoInputs);
		ImGui::ColorEdit3("Selected", &settings.selectedColor.x, ImGuiColorEditFlags_NoInputs);

		ImGui::Separator();

		// 선택 정보
		ECS::Entity selectedEntity = debugRenderer->GetSelectedEntity();
		if (selectedEntity.IsValid())
		{
			ImGui::Text("Selected: Entity %u", selectedEntity.id);
		}
		else
		{
			ImGui::TextDisabled("No entity selected");
		}

		ImGui::End();
	}

} // namespace Framework
