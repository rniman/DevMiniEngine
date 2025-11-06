#pragma once

#include "Core/Types.h"
#include "Graphics/Camera/PerspectiveCamera.h"
#include "Graphics/RenderTypes.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>

namespace Framework
{
	class GameObject;

	class Scene
	{
	public:
		Scene();
		~Scene();

		// GameObject 관리
		GameObject* CreateGameObject(const std::string& name);
		GameObject* FindGameObject(const std::string& name);
		bool RemoveGameObject(const std::string& name);
		void ClearGameObjects();

		// 카메라
		Graphics::PerspectiveCamera* GetMainCamera() { return &mMainCamera; }
		const Graphics::PerspectiveCamera* GetMainCamera() const { return &mMainCamera; }

		// 업데이트
		void Update(Core::float32 deltaTime);

		/**
		 * @brief 렌더링 데이터 수집 (렌더링하지 않음!)
		 * @param outFrameData 수집된 렌더링 데이터를 저장할 구조체
		 */
		void CollectRenderData(Graphics::FrameData& outFrameData) const;

		// 활성화 상태
		void SetActive(bool active) { mIsActive = active; }
		bool IsActive() const { return mIsActive; }

	private:
		bool mIsActive = true;

		// 게임 오브젝트
		std::vector<std::unique_ptr<GameObject>> mGameObjects;
		std::unordered_map<std::string, GameObject*> mGameObjectMap;

		// 카메라
		Graphics::PerspectiveCamera mMainCamera;
	};

} // namespace Framework
