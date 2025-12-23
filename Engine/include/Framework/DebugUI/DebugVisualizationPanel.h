#pragma once

#include "Core/Types.h"

namespace Graphics
{
	class DebugRenderer;
}

namespace Framework
{
	/**
	 * @brief Debug Visualization 설정 패널
	 *
	 * Light Gizmo 표시 모드, 색상 등을 설정합니다.
	 */
	class DebugVisualizationPanel
	{
	public:
		DebugVisualizationPanel() = default;
		~DebugVisualizationPanel() = default;

		DebugVisualizationPanel(const DebugVisualizationPanel&) = delete;
		DebugVisualizationPanel& operator=(const DebugVisualizationPanel&) = delete;

		void Render(Graphics::DebugRenderer* debugRenderer);

		void SetVisible(bool visible) { mIsVisible = visible; }
		bool IsVisible() const { return mIsVisible; }

	private:
		bool mIsVisible = true;
	};

} // namespace Framework
