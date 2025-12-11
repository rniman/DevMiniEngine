/**
 * @file RenderSystem.h
 * @brief 렌더링 데이터를 수집하는 System
 *
 * 생성자 주입 방식으로 Registry를 받습니다.
 * 추가로 ResourceManager도 필요합니다.
 */
#pragma once
#include "ECS/ISystem.h"
#include "Core/Types.h"
#include "Graphics/RenderTypes.h"

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
	 * CameraSystem과 협력하여 Main Camera를 사용하고,
	 * LightingSystem과 협력하여 조명 정보를 수집합니다.
	 */
	class RenderSystem : public ISystem
	{
	public:
		/**
		 * @brief 생성자
		 * @param registry 이 System이 속한 Registry
		 * @param resourceManager 리소스 관리자 (Mesh, Material 조회)
		 */
		RenderSystem(Registry& registry, Framework::ResourceManager* resourceManager);
		~RenderSystem() override = default;

		//=========================================================================
		// ISystem 인터페이스 구현
		//=========================================================================

		void Initialize() override;
		void Update(Core::float32 deltaTime) override;
		void Shutdown() override;

		//=========================================================================
		// 데이터 접근
		//=========================================================================

		/**
		 * @brief 수집된 렌더링 데이터 가져오기
		 * @return 현재 프레임의 렌더링 데이터
		 */
		const Graphics::FrameData& GetFrameData() const { return mFrameData; }

	private:
		Framework::ResourceManager* mResourceManager;
		Graphics::FrameData mFrameData;
	};

} // namespace ECS
