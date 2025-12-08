#pragma once
#include "ECS/ISystem.h"
#include "Core/Types.h"
#include "Graphics/RenderTypes.h"

#include <memory>

namespace Framework
{
	class ResourceManager;
}

namespace ECS
{
	/**
	 * @brief 렌더링 데이터를 수집하는 System
	 *
	 * TransformComponent + MeshComponent + MaterialComponent를 가진
	 * Entity들을 찾아서 FrameData를 구성합니다.
	 *
	 * Phase 3.2.1: CameraComponent 통합
	 * - Main Camera Entity를 자동으로 찾아서 사용
	 * - PerspectiveCamera 포인터 의존성 제거
	 * 
	 * Phase 3.3: Lighting System 통합
	 * - DirectionalLight, PointLight 데이터 수집
	 * - LightingSystem과 협력하여 조명 정보 구성
	 */
	class RenderSystem : public ISystem
	{
	public:
		/**
		 * @brief RenderSystem 생성자
		 *
		 * @param resourceManager 리소스 관리자 (Mesh, Material 조회)
		 */
		explicit RenderSystem(Framework::ResourceManager* resourceManager);

		~RenderSystem() override = default;

		// ISystem 인터페이스 구현
		void OnInitialize(Registry& registry) override;
		void OnUpdate(Registry& registry, Core::float32 deltaTime) override;
		void OnShutdown(Registry& registry) override;

		/**
		 * @brief 수집된 렌더링 데이터 가져오기
		 *
		 * OnUpdate에서 수집한 데이터를 반환합니다.
		 *
		 * @return 현재 프레임의 렌더링 데이터
		 */
		const Graphics::FrameData& GetFrameData() const { return mFrameData; }

	private:
		Framework::ResourceManager* mResourceManager;
		Graphics::FrameData mFrameData;
	};

} // namespace ECS
