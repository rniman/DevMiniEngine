#include "pch.h"
#include "Framework/Scene/Scene.h"
#include "Framework/Scene/GameObject.h"
#include "Core/Logging/LogMacros.h"
#include "Math/MathTypes.h"
#include "Math/MathUtils.h"

namespace Framework
{
	Scene::Scene()
	{
		LOG_INFO("Scene created");

		// 기본 카메라 설정
		mMainCamera.SetLookAt(
			Math::Vector3(0.0f, 10.0f, -20.0f),
			Math::Vector3(0.0f, 0.0f, 0.0f),
			Math::Vector3(0.0f, 1.0f, 0.0f)
		);
	}

	Scene::~Scene()
	{
		ClearGameObjects();
		LOG_INFO("Scene destroyed");
	}

	GameObject* Scene::CreateGameObject(const std::string& name)
	{
		// 중복 이름 확인
		if (mGameObjectMap.find(name) != mGameObjectMap.end())
		{
			LOG_WARN("GameObject '%s' already exists", name.c_str());
			return mGameObjectMap[name];
		}

		// 새 게임 오브젝트 생성
		auto gameObject = std::make_unique<GameObject>(name);
		GameObject* ptr = gameObject.get();

		mGameObjects.push_back(std::move(gameObject));
		mGameObjectMap[name] = ptr;

		LOG_DEBUG("Created GameObject: %s", name.c_str());
		return ptr;
	}

	GameObject* Scene::FindGameObject(const std::string& name)
	{
		auto it = mGameObjectMap.find(name);
		if (it != mGameObjectMap.end())
		{
			return it->second;
		}
		return nullptr;
	}

	bool Scene::RemoveGameObject(const std::string& name)
	{
		auto mapIt = mGameObjectMap.find(name);
		if (mapIt == mGameObjectMap.end())
		{
			return false;
		}

		GameObject* target = mapIt->second;
		mGameObjectMap.erase(mapIt);

		// 벡터에서도 제거
		auto vecIt = std::find_if(mGameObjects.begin(), mGameObjects.end(),
			[target](const std::unique_ptr<GameObject>& obj) {
				return obj.get() == target;
			});

		if (vecIt != mGameObjects.end())
		{
			mGameObjects.erase(vecIt);
			LOG_DEBUG("Removed GameObject: %s", name.c_str());
			return true;
		}

		return false;
	}

	void Scene::ClearGameObjects()
	{
		mGameObjectMap.clear();
		mGameObjects.clear();
		LOG_INFO("All GameObjects cleared");
	}

	void Scene::Update(Core::float32 deltaTime)
	{
		if (!mIsActive)
		{
			return;
		}

		// 카메라 업데이트
		mMainCamera.UpdateViewMatrix();
		mMainCamera.UpdateProjectionMatrix();

		// 모든 게임 오브젝트 업데이트
		for (auto& gameObject : mGameObjects)
		{
			if (gameObject->IsActive())
			{
				gameObject->Update(deltaTime);
			}
		}
	}

	void Scene::CollectRenderData(Graphics::FrameData& outFrameData) const
	{
		if (!mIsActive)
		{
			return;
		}

		// 프레임 데이터 초기화
		outFrameData.Clear();

		// 카메라 정보 설정
		outFrameData.viewMatrix = mMainCamera.GetViewMatrix();
		outFrameData.projectionMatrix = mMainCamera.GetProjectionMatrix();
		outFrameData.cameraPosition = mMainCamera.GetPosition();

		// 렌더링 아이템 수집
		for (const auto& gameObject : mGameObjects)
		{
			if (!gameObject->IsActive())
			{
				continue;
			}

			auto mesh = gameObject->GetMesh();
			auto material = gameObject->GetMaterial();

			if (!mesh || !material)
			{
				continue;
			}

			// MVP 행렬 계산
			Math::Matrix4x4 world = gameObject->GetWorldMatrix();
			Math::Matrix4x4 view = outFrameData.viewMatrix;
			Math::Matrix4x4 proj = outFrameData.projectionMatrix;

			Math::Matrix4x4 mvp = Math::MatrixMultiply(world, view);
			mvp = Math::MatrixMultiply(mvp, proj);

			// 렌더 아이템 생성
			Graphics::RenderItem item;
			item.mesh = mesh.get();
			item.material = material.get();
			item.worldMatrix = world;
			item.mvpMatrix = Math::MatrixTranspose(mvp);  // HLSL용 전치

			// TODO: 향후 머티리얼 속성에 따라 분류
			// if (material->IsTransparent())
			//     outFrameData.transparentItems.push_back(item);
			// else
			outFrameData.opaqueItems.push_back(item);
		}

		// TODO: 향후 정렬 추가
		// - 불투명: 앞에서 뒤로 (Early-Z 최적화)
		// - 투명: 뒤에서 앞으로 (올바른 블렌딩)
	}

} // namespace Framework
