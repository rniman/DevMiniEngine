#include "pch.h"
#include "Framework/DebugUI/PerformancePanel.h"

// ECS
#include "ECS/Registry.h"

// ThirdParty - ImGui
#include <imgui.h>


namespace Framework
{
	void PerformancePanel::Update(Core::float32 deltaTime)
	{
		// Frame Time 기록 (밀리초)
		Core::float32 frameTimeMs = deltaTime * 1000.0f;
		mFrameTimeHistory[mHistoryIndex] = frameTimeMs;
		mHistoryIndex = (mHistoryIndex + 1) % HISTORY_SIZE;

		// 현재 값 업데이트
		mCurrentFrameTime = frameTimeMs;
		mCurrentFPS = (deltaTime > 0.0f) ? (1.0f / deltaTime) : 0.0f;

		// Min/Max 업데이트
		mMinFrameTime = std::min(frameTimeMs, mMinFrameTime);
		mMaxFrameTime = std::max(frameTimeMs, mMaxFrameTime);

		// 평균 계산
		Core::float32 sum = std::accumulate(mFrameTimeHistory.begin(), mFrameTimeHistory.end(), 0.0f);
		mAverageFrameTime = sum / static_cast<Core::float32>(HISTORY_SIZE);
	}

	void PerformancePanel::Render(ECS::Registry* registry)
	{
		if (!mIsVisible)
		{
			return;
		}

		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Performance", &mIsVisible))
		{
			RenderFrameTimeGraph();
			ImGui::Separator();
			RenderStatistics(registry);
		}
		ImGui::End();
	}

	void PerformancePanel::RenderFrameTimeGraph()
	{
		// FPS / Frame Time 텍스트
		ImGui::Text("FPS: %.1f", mCurrentFPS);
		ImGui::Text("Frame Time: %.2f ms", mCurrentFrameTime);
		ImGui::Text("Average: %.2f ms", mAverageFrameTime);
		ImGui::Text("Min: %.2f ms  Max: %.2f ms", mMinFrameTime, mMaxFrameTime);

		// 리셋 버튼
		ImGui::SameLine();
		if (ImGui::SmallButton("Reset"))
		{
			mMinFrameTime = FLT_MAX;
			mMaxFrameTime = 0.0f;
		}

		// Frame Time 그래프
		ImGui::PlotLines(
			"##FrameTime",
			mFrameTimeHistory.data(),
			static_cast<int>(HISTORY_SIZE),
			static_cast<int>(mHistoryIndex),
			nullptr,
			0.0f,
			33.3f,  // 30 FPS 기준 상한
			ImVec2(0, 50)
		);
	}

	void PerformancePanel::RenderStatistics(ECS::Registry* registry)
	{
		if (registry)
		{
			ImGui::Text("Entities: %u", registry->GetEntityCount());
			// TODO: Component 통계 추가
			// ImGui::Text("Components: %u", registry->GetComponentCount());
		}
		else
		{
			ImGui::TextDisabled("No Registry connected");
		}
	}

} // namespace Framework
