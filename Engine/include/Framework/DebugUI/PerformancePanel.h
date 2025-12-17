#pragma once

#include "Core/Types.h"
#include <array>

namespace ECS
{
	class Registry;
}

namespace Framework
{
	/**
	 * @brief 성능 모니터링 패널
	 *
	 * FPS, Frame Time, Entity/Component 통계를 실시간으로 표시합니다.
	 */
	class PerformancePanel
	{
	public:
		PerformancePanel() = default;
		~PerformancePanel() = default;

		// 복사/이동 방지
		PerformancePanel(const PerformancePanel&) = delete;
		PerformancePanel& operator=(const PerformancePanel&) = delete;

		/**
		 * @brief 프레임 데이터 업데이트
		 * @param deltaTime 이번 프레임 델타 타임 (초)
		 */
		void Update(Core::float32 deltaTime);

		/**
		 * @brief 패널 렌더링
		 * @param registry ECS Registry (Entity 통계용, nullptr 허용)
		 */
		void Render(ECS::Registry* registry = nullptr);

		/**
		 * @brief 패널 표시 여부 설정
		 */
		void SetVisible(bool visible) { mIsVisible = visible; }
		bool IsVisible() const { return mIsVisible; }

	private:
		void RenderFrameTimeGraph();
		void RenderStatistics(ECS::Registry* registry);

	private:
		// Frame Time 히스토리
		static constexpr Core::uint32 HISTORY_SIZE = 120;
		std::array<Core::float32, HISTORY_SIZE> mFrameTimeHistory = {};
		Core::uint32 mHistoryIndex = 0;

		// 통계
		Core::float32 mCurrentFPS = 0.0f;
		Core::float32 mCurrentFrameTime = 0.0f;
		Core::float32 mAverageFrameTime = 0.0f;
		Core::float32 mMinFrameTime = FLT_MAX;
		Core::float32 mMaxFrameTime = 0.0f;

		// 상태
		bool mIsVisible = true;
	};

} // namespace Framework
