#pragma once
#include "ECS/ISystem.h"
#include "Graphics/RenderTypes.h"

#include <memory>

namespace Framework
{
	class ResourceManager;
}

namespace Graphics
{
	class PerspectiveCamera;
}

namespace ECS
{
	/**
	 * @brief 렌더링 데이터를 수집하는 System
	 *
	 * TransformComponent + MeshComponent + MaterialComponent를 가진
	 * Entity들을 찾아서 FrameData를 구성합니다.
	 */
	class RenderSystem : public ISystem
	{
	public:
		/**
		 * @brief RenderSystem 생성자
		 *
		 * @param resourceManager 리소스 관리자 (Mesh, Material 조회)
		 * @param camera 렌더링에 사용할 카메라
		 */
		RenderSystem(
			Framework::ResourceManager* resourceManager,
			Graphics::PerspectiveCamera* camera
		);

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

		/**
		 * @brief 카메라 설정
		 *
		 * @param camera 새로운 카메라 포인터
		 */
		void SetCamera(Graphics::PerspectiveCamera* camera) { mCamera = camera; }

	private:
		Framework::ResourceManager* mResourceManager;
		Graphics::PerspectiveCamera* mCamera;
		Graphics::FrameData mFrameData;
	};

} // namespace ECS
