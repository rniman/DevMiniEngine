#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Framework/Application.h"

#include "Core/Types.h"
#include "ECS/Entity.h"
#include "Framework/Resources/ResourceId.h"

#include <memory>
#include <vector>

namespace ECS
{
	class SystemManager;
	class Registry;
}

namespace Framework
{
	class ResourceManager;
	// struct ResourceId;
}

namespace Graphics
{
	struct FrameData;
}

/**
 * @brief Phase 3.3: Phong Shading 데모 애플리케이션
 *
 * ECS 아키텍처 기반으로 Directional Light + Point Lights를 사용한
 * Phong Shading 조명 효과를 시연합니다.
 *
 * - 5x5 그리드로 배치된 회전하는 큐브들
 * - 1개의 Directional Light (태양광)
 * - 4개의 Point Light (색상별, 원형 애니메이션)
 */
class PhongLightingApp : public Framework::Application
{
public:
	PhongLightingApp();
	~PhongLightingApp() override;

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
		desc.applicationName = "PhongLighting";
		desc.windowTitle = "10_PhongLighting - Phase 3.3 Demo";
		desc.windowWidth = 1280;
		desc.windowHeight = 720;
		desc.enableVSync = true;
		desc.enableDebugLayer = true;
		return desc;
	}

	// ECS 초기화
	void InitializeECS();
	void CreateCameraEntity();
	void CreateLightEntities();
	void CreateCubeEntities();
	void SetupSharedMeshData();
	void SetupSharedMaterial();

	// ECS
	std::unique_ptr<ECS::Registry> mRegistry;
	std::unique_ptr<ECS::SystemManager> mSystemManager;
	std::unique_ptr<Framework::ResourceManager> mResourceManager;

	// Entities
	ECS::Entity mCameraEntity;
	ECS::Entity mDirectionalLightEntity;
	std::vector<ECS::Entity> mPointLightEntities;
	std::vector<ECS::Entity> mCubeEntities;

	// 애니메이션
	Core::float32 mRotationAngle = 0.0f;
	Core::float32 mPointLightTime = 0.0f;

	// 공유 리소스 ID (모든 큐브가 같은 메시/머티리얼 사용)
	Framework::ResourceId mSharedMeshId;
	Framework::ResourceId mSharedMaterialId;
};
