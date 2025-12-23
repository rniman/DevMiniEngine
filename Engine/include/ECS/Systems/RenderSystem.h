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
	 * Renderable Entity들을 순회하여 FrameData를 구성합니다.
	 * CameraSystem, LightingSystem과 협력합니다.
	 */
	class RenderSystem : public ISystem
	{
	public:
		RenderSystem(Registry& registry, Framework::ResourceManager* resourceManager);
		~RenderSystem() override = default;

		//=====================================================================
		// ISystem 인터페이스
		//=====================================================================

		void Initialize() override;
		void Update(Core::float32 deltaTime) override;
		void Shutdown() override;

		//=====================================================================
		// 데이터 접근
		//=====================================================================

		const Graphics::FrameData& GetFrameData() const { return mFrameData; }

	private:
		Framework::ResourceManager* mResourceManager;
		Graphics::FrameData mFrameData;
	};

} // namespace ECS
