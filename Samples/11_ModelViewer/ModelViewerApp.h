/**
 * @file ModelViewerApp.h
 * @brief Phase 4.2: Model Loading 데모 애플리케이션
 *
 * Procedural Mesh vs Loaded Mesh 비교 + glTF 모델 텍스처 정보 테스트:
 * - 왼쪽: Procedural Sphere (PrimitiveGenerator + MikkTSpace)
 * - 중앙: Loaded Sphere (ModelLoader + glTF)
 * - 오른쪽: DamagedHelmet (LoadModel + 머티리얼/텍스처 정보)
 *
 * 텍스처 로딩 지원:
 * - 외부 텍스처: gltf + 별도 이미지 파일 (jpg, png 등)
 * - 임베디드 텍스처: glb 파일 내 포함된 텍스처 (Phase 4.2+)
 * - 폴백 텍스처: 로드 실패 시 1x1 Magenta
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

	/** @brief PrimitiveGenerator + MikkTSpace로 구 메시 생성 */
	void SetupProceduralSphereMesh(Core::uint32 segments, Core::uint32 rings);

	/** @brief glTF 파일에서 구 메시 로드 */
	void SetupLoadedSphereMesh();

	/** @brief BrickWall 텍스처를 사용하는 공유 머티리얼 설정 */
	void SetupSharedMaterial();

	/** @brief DamagedHelmet 메시 로드 (LoadModel API 사용) */
	void SetupHelmetMesh();

	/**
	 * @brief DamagedHelmet 머티리얼 설정
	 *
	 * LoadedModelData에서 텍스처 정보를 읽어 Material에 바인딩합니다.
	 *
	 * 텍스처 로딩 우선순위:
	 * 1. 임베디드 텍스처 (glb 내 포함, HasEmbeddedData() == true)
	 *    - 압축 포맷 (PNG/JPG): LoadTextureFromMemory()
	 *    - 원시 RGBA: CreateTextureFromMemory()
	 * 2. 외부 텍스처 (gltf + 별도 파일)
	 *    - LoadTexture(path)
	 * 3. 폴백 텍스처 (로드 실패 시)
	 *    - GetFallbackTexture() (1x1 Magenta)
	 */
	void SetupHelmetMaterial();

	//=========================================================================
	// Debug UI
	//=========================================================================

	void RenderAssetManagerPanel();
	void RenderSphereControlPanel();
	void RenderModelInfoPanel();

	// Model Info 패널 헬퍼 (섹션별 분리)
	void RenderModelInfoPanelSummary();
	void RenderModelInfoPanelAssetPipeline();
	void RenderModelInfoPanelMaterials();
	void RenderModelInfoPanelMeshes();

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

	//=========================================================================
	// Procedural Sphere (왼쪽)
	//=========================================================================
	ECS::Entity mProceduralSphereEntity;
	Framework::ResourceId mProceduralMeshId;

	//=========================================================================
	// Loaded Sphere (중앙)
	//=========================================================================
	ECS::Entity mLoadedSphereEntity;
	Framework::ResourceId mLoadedMeshId;
	std::unique_ptr<Framework::MeshAsset> mLoadedSphereMeshAsset;

	//=========================================================================
	// DamagedHelmet (오른쪽)
	//=========================================================================
	ECS::Entity mHelmetEntity;
	Framework::ResourceId mHelmetMeshId;
	Framework::ResourceId mHelmetMaterialId;
	std::unique_ptr<Framework::MeshAsset> mHelmetMeshAsset;

	/**
	 * @brief 로드된 모델 데이터 (메시, 머티리얼, 텍스처 정보)
	 *
	 * SetupHelmetMesh()에서 채워지고, SetupHelmetMaterial()에서 사용됩니다.
	 * 임베디드 텍스처의 경우 embeddedData에 바이너리가 저장되어 있습니다.
	 */
	Framework::LoadedModelData mHelmetModelData;

	//=========================================================================
	// 공유 리소스
	//=========================================================================

	/** @brief Procedural/Loaded Sphere가 공유하는 머티리얼 */
	Framework::ResourceId mSharedMaterialId;

	//=========================================================================
	// 애니메이션 & UI 상태
	//=========================================================================

	Core::float32 mRotationAngle = 0.0f;
	Core::float32 mRotationSpeed = 0.5f;

	// Procedural Sphere 파라미터
	Core::uint32 mSphereSegments = 32;
	Core::uint32 mSphereRings = 16;

	// UI 패널 토글
	bool mShowAssetManagerPanel = true;
	bool mShowSphereControlPanel = true;
	bool mShowModelInfoPanel = true;

	// Procedural Mesh 재생성 플래그
	bool mNeedsMeshRebuild = false;

	// 로드 상태 플래그
	bool mLoadedMeshValid = false;
	bool mHelmetMeshValid = false;
};
