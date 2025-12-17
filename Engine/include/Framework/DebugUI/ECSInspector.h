#pragma once

#include "Core/Types.h"
#include "ECS/Entity.h"

namespace ECS
{
	class Registry;
}

namespace Framework
{
	/**
	 * @brief ECS Inspector 패널
	 *
	 * Entity 목록 조회, Component 편집 기능을 제공합니다.
	 */
	class ECSInspector
	{
	public:
		ECSInspector() = default;
		~ECSInspector() = default;

		ECSInspector(const ECSInspector&) = delete;
		ECSInspector& operator=(const ECSInspector&) = delete;

		/**
		 * @brief Inspector 렌더링
		 * @param registry ECS Registry (nullptr이면 렌더링 스킵)
		 */
		void Render(ECS::Registry* registry);

		void SetVisible(bool visible) { mIsVisible = visible; }
		bool IsVisible() const { return mIsVisible; }

		ECS::Entity GetSelectedEntity() const { return mSelectedEntity; }

	private:
		void RenderEntityList(ECS::Registry* registry);
		void RenderComponentInspector(ECS::Registry* registry);

		// Component별 편집 UI
		void RenderTransformComponent(ECS::Registry* registry, ECS::Entity entity);
		void RenderCameraComponent(ECS::Registry* registry, ECS::Entity entity);
		void RenderDirectionalLightComponent(ECS::Registry* registry, ECS::Entity entity);
		void RenderPointLightComponent(ECS::Registry* registry, ECS::Entity entity);
		void RenderMeshComponent(ECS::Registry* registry, ECS::Entity entity);
		void RenderMaterialComponent(ECS::Registry* registry, ECS::Entity entity);

		// Component 추가/삭제 UI
		void RenderAddComponentMenu(ECS::Registry* registry, ECS::Entity entity);

	private:
		ECS::Entity mSelectedEntity;
		bool mIsVisible = true;
		char mEntityFilter[64] = "";

		// Component 삭제 예약 (렌더링 중 삭제 방지)
		bool mPendingDeleteTransform = false;
		bool mPendingDeleteCamera = false;
		bool mPendingDeleteDirLight = false;
		bool mPendingDeletePointLight = false;
		bool mPendingDeleteMesh = false;
		bool mPendingDeleteMaterial = false;
	};

} // namespace Framework
