/**
 * @file ModelViewerApp.h
 * @brief Phase 4.5: Model Loading 데모 애플리케이션 (리팩토링)
 *
 * 다양한 메시 로딩 방식과 멀티 서브메시 비교:
 * - 왼쪽: Procedural Sphere (PrimitiveGenerator + MikkTSpace)
 * - 중앙: Loaded Sphere (ModelLoader + glTF)
 * - 오른쪽: DamagedHelmet (LoadModel + 머티리얼/텍스처 정보)
 * - 상단: MultiMaterialCube (멀티 서브메시 + 개별 머티리얼)
 * - 하단: 2CylinderEngine (계층 구조 테스트)
 *
 * @note Phase 4.5 리팩토링: HierarchyBuilder 분리
 *       - CreateModelHierarchy → HierarchyBuilder::Build
 *       - CreateMaterialsFromModelData → ResourceManager로 이동
 *       - CreateMeshFromNodeIndices → HierarchyBuilder 내부로 이동
 */
#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Framework/Application.h"
#include "Framework/Assets/HierarchyBuilder.h"  // HierarchyBuildResult
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
 * @brief Model Inspector에서 선택 가능한 모델
 */
enum class SelectedModel : int
{
	ProceduralSphere = 0,
	LoadedSphere,
	Helmet,
	Cube,
	Engine,
	Count
};

/**
 * @brief Phase 4.5: Model Loading 데모 애플리케이션
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
		desc.windowTitle = "11_ModelViewer - Phase 4.5 Hierarchy";
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

	/** @brief MultiMaterialCube Entity 생성 (상단) */
	void CreateCubeEntity();

	/** @brief 2CylinderEngine Entity 생성 (하단) */
	void CreateEngineEntity();

	//=========================================================================
	// 메시 설정
	//=========================================================================

	/** @brief PrimitiveGenerator + MikkTSpace로 구 메시 생성 */
	void SetupProceduralSphereMesh(Core::uint32 segments, Core::uint32 rings);

	/** @brief glTF 파일에서 구 메시 로드 */
	void SetupLoadedSphereMesh();

	/** @brief BrickWall 텍스처를 사용하는 공유 머티리얼 설정 */
	void SetupSharedMaterial();

	//=========================================================================
	// 모델 로딩 헬퍼 (레거시 - LoadAndCreateMesh)
	//=========================================================================

	/**
	 * @brief glTF 모델에서 메시를 로드하고 GPU 리소스 생성
	 *
	 * @deprecated 단일 Entity 모델용. 계층 구조 모델은 HierarchyBuilder 사용
	 *
	 * @param modelPath glTF/glb 파일 경로
	 * @param modelName 리소스 이름 (로그 및 ResourceId용)
	 * @param outModelData 로드된 모델 데이터 (출력)
	 * @param outMeshId 생성된 MeshResource ID (출력)
	 * @return 성공 여부
	 */
	bool LoadAndCreateMesh(
		const char* modelPath,
		const std::string& modelName,
		Framework::LoadedModelData& outModelData,
		Framework::ResourceId& outMeshId
	);

	//=========================================================================
	// Debug UI
	//=========================================================================

	void RenderAssetManagerPanel();
	void RenderSphereControlPanel();
	void RenderModelInfoPanel();

	//=========================================================================
	// Model Inspector 헬퍼
	//=========================================================================

	/** @brief 선택된 모델의 LoadedModelData 반환 (없으면 nullptr) */
	const Framework::LoadedModelData* GetSelectedModelData() const;

	/** @brief 선택된 모델의 MeshId 반환 (Entity에서 조회) */
	Framework::ResourceId GetSelectedMeshId() const;

	/** @brief 선택된 모델이 유효한지 (로드 성공) */
	bool IsSelectedModelValid() const;

	/** @brief 선택된 모델 이름 */
	const char* GetSelectedModelName() const;

	// Model Info 패널 섹션별 렌더링
	void RenderModelInfoPanelSummary(const Framework::LoadedModelData* modelData);
	void RenderModelInfoPanelProceduralInfo();
	void RenderModelInfoPanelAssetPipeline(Framework::ResourceId meshId);
	void RenderModelInfoPanelMaterials(const Framework::LoadedModelData* modelData);
	void RenderModelInfoPanelMeshes(const Framework::LoadedModelData* modelData, Framework::ResourceId meshId);
	void RenderModelInfoPanelTextures(const Framework::LoadedModelData* modelData);

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
	Framework::LoadedModelData mLoadedSphereModelData;  // Inspector용

	//=========================================================================
	// DamagedHelmet (오른쪽)
	//=========================================================================
	ECS::Entity mHelmetEntity;
	Framework::ResourceId mHelmetMeshId;
	std::vector<Framework::ResourceId> mHelmetMaterialIds;
	Framework::LoadedModelData mHelmetModelData;

	//=========================================================================
	// MultiMaterialCube (상단) - Phase 4.4 멀티 서브메시 테스트
	//=========================================================================
	ECS::Entity mCubeEntity;
	Framework::ResourceId mCubeMeshId;
	std::vector<Framework::ResourceId> mCubeMaterialIds;
	Framework::LoadedModelData mCubeModelData;
	bool mCubeMeshValid = false;

	//=========================================================================
	// 2CylinderEngine (하단) - Phase 4.5 계층 테스트
	//=========================================================================
	ECS::Entity mEngineEntity;
	std::vector<Framework::ResourceId> mEngineMaterialIds;
	Framework::LoadedModelData mEngineModelData;
	bool mEngineMeshValid = false;

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

	// Model Inspector 선택
	SelectedModel mSelectedModel = SelectedModel::ProceduralSphere;

	// Procedural Mesh 재생성 플래그
	bool mNeedsMeshRebuild = false;

	// 로드 상태 플래그
	bool mLoadedMeshValid = false;
	bool mHelmetMeshValid = false;
};
