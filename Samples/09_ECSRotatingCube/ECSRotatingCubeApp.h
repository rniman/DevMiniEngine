#pragma once
#include "Framework/Application.h"

#include "Core/Types.h"
#include "ECS/Entity.h"
#include "Graphics/Camera/PerspectiveCamera.h"

#include <memory>

namespace ECS
{
	class SystemManager;
	class Registry;
}

namespace Framework
{
	class ResourceManager;
}

namespace Graphics
{
	struct FrameData;
}

/**
 * @brief ECS 아키텍처 기반 회전하는 큐브 샘플
 *
 * Phase 3: ECS Core 시연
 * - Registry를 통한 Entity 관리
 * - Component 기반 데이터 저장
 * - TransformSystem을 통한 회전
 * - ResourceId 기반 리소스 참조
 */
class ECSRotatingCubeApp : public Framework::Application
{
public:
	ECSRotatingCubeApp();

	~ECSRotatingCubeApp() override;

protected:
	// Application 라이프사이클 오버라이드
	bool OnInitialize() override;
	void OnUpdate(Core::float32 deltaTime) override;
	void OnRender() override;
	void OnShutdown() override;

private:
	static Framework::ApplicationDesc GetAppDesc()
	{
		Framework::ApplicationDesc desc;
		desc.applicationName = "ECSRotatingCube";
		desc.windowTitle = "09_ECSRotatingCube - DevMiniEngine";
		desc.windowWidth = 1280;
		desc.windowHeight = 720;
		desc.enableVSync = true;
		desc.enableDebugLayer = true;
		return desc;
	}

	// ECS 초기화
	void InitializeECS();
	void CreateCubeEntity();
	void SetupMeshData();
	void SetupMaterial();
	void CollectRenderData(Graphics::FrameData& outFrameData);

	// ECS
	std::unique_ptr<ECS::Registry> mRegistry;
	std::unique_ptr<ECS::SystemManager> mSystemManager; 

	// Entity
	ECS::Entity mCubeEntity;

	// 카메라
	Graphics::PerspectiveCamera mCamera;

	// 회전 속도 (도/초)
	Core::float32 mRotationSpeed = 90.0f;

	std::unique_ptr<Framework::ResourceManager> mResourceManager;
};
