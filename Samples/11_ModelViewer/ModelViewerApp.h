/**
 * @file ModelViewerApp.h
 * @brief Phase 4.2: Model Loading 데모 애플리케이션
 *
 * Procedural Mesh vs Loaded Mesh 비교 + glTF 모델 텍스처 정보 테스트:
 * - 왼쪽: Procedural Sphere (PrimitiveGenerator + MikkTSpace)
 * - 중앙: Loaded Sphere (ModelLoader + glTF)
 * - 오른쪽: DamagedHelmet (LoadModel + 머티리얼/텍스처 정보)
 */
#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Framework/Application.h"
#include "Framework/Assets/MeshAsset.h"
#include "Framework/Assets/ModelLoader.h"

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
	class AssetManager;
}

namespace Graphics
{
	struct FrameData;
}

/**
 * @brief Phase 4.2: Model Loading 데모 애플리케이션
 */
class ModelViewerApp : public Framework::Application
{
public:
	ModelViewerApp();
	~ModelViewerApp() override;

protected:
	// Application 라이프사이클 오버라이드
	bool OnInitialize() override;
	void OnUpdate(Core::float32 deltaTime) override;
	void OnRender() override;
	void OnShutdown() override;
	void OnRenderDebugUI() override;

private:
	static Framework::ApplicationDesc GetAppDesc()
	{
		Framework::ApplicationDesc desc;
		desc.applicationName = "ModelViewer";
		desc.windowTitle = "11_ModelViewer - Phase 4.2 Model Loading";
		desc.windowWidth = 1280;
		desc.windowHeight = 720;
		desc.enableVSync = true;
		desc.enableDebugLayer = true;
		return desc;
	}

	//=========================================================================
	// 초기화
	//=========================================================================

	void InitializeECS();
	void CreateCameraEntity();
	void CreateLightEntities();

	/** @brief Procedural Sphere Entity 생성 (왼쪽) */
	void CreateProceduralSphereEntity();

	/** @brief Loaded Sphere Entity 생성 (중앙) */
	void CreateLoadedSphereEntity();

	/** @brief DamagedHelmet Entity 생성 (오른쪽) */
	void CreateHelmetEntity();

	//=========================================================================
	// 메시 설정
	//=========================================================================

	void SetupProceduralSphereMesh(Core::uint32 segments, Core::uint32 rings);
	void SetupLoadedSphereMesh();
	void SetupHelmetMesh();
	void SetupSharedMaterial();

	//=========================================================================
	// Debug UI
	//=========================================================================

	void RenderAssetManagerPanel();
	void RenderSphereControlPanel();
	void RenderModelInfoPanel();

private:

	// 리소스 관리
	std::unique_ptr<Framework::ResourceManager> mResourceManager;
	std::unique_ptr<Framework::AssetManager> mAssetManager;

	// ECS
	std::unique_ptr<ECS::Registry> mRegistry;
	std::unique_ptr<ECS::SystemManager> mSystemManager;

	// Entities
	ECS::Entity mCameraEntity;
	ECS::Entity mDirectionalLightEntity;

	// Procedural Sphere (왼쪽)
	ECS::Entity mProceduralSphereEntity;
	Framework::ResourceId mProceduralMeshId;

	// Loaded Sphere (중앙)
	ECS::Entity mLoadedSphereEntity;
	Framework::ResourceId mLoadedMeshId;
	std::unique_ptr<Framework::MeshAsset> mLoadedSphereMeshAsset;

	// DamagedHelmet (오른쪽)
	ECS::Entity mHelmetEntity;
	Framework::ResourceId mHelmetMeshId;
	std::unique_ptr<Framework::MeshAsset> mHelmetMeshAsset;
	Framework::LoadedModelData mHelmetModelData;  // 머티리얼/텍스처 정보 저장

	// 공유 머티리얼
	Framework::ResourceId mSharedMaterialId;

	// 애니메이션
	Core::float32 mRotationAngle = 0.0f;
	Core::float32 mRotationSpeed = 0.5f;

	// Procedural Sphere 파라미터
	Core::uint32 mSphereSegments = 32;
	Core::uint32 mSphereRings = 16;

	// UI 상태
	bool mShowAssetManagerPanel = true;
	bool mShowSphereControlPanel = true;
	bool mShowModelInfoPanel = true;
	bool mNeedsMeshRebuild = false;

	// Loaded Mesh 상태
	bool mLoadedMeshValid = false;
	bool mHelmetMeshValid = false;
};
