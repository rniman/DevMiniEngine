/**
 * @file ModelViewerApp.h
 * @brief Phase 4.5: Model Loading 데모 애플리케이션
 *
 * 다양한 메시 로딩 방식과 멀티 서브메시 비교:
 * - 왼쪽: Procedural Sphere (PrimitiveGenerator + MikkTSpace)
 * - 중앙: Loaded Sphere (ModelLoader + glTF)
 * - 오른쪽: DamagedHelmet (LoadModel + 머티리얼/텍스처 정보)
 * - 상단: MultiMaterialCube (멀티 서브메시 + 개별 머티리얼)
 * - 하단: 2CylinderEngine (계층 구조 테스트)
 *
 * 텍스처 로딩 지원:
 * - 외부 텍스처: gltf + 별도 이미지 파일 (jpg, png 등)
 * - 임베디드 텍스처: glb 파일 내 포함된 텍스처
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

//=============================================================================
// 모델 계층 생성 결과 (Phase 4.5)
//=============================================================================

/**
 * @brief glTF 노드 계층 → ECS Entity 변환 결과
 *
 * CreateModelHierarchy() 함수의 반환 타입입니다.
 * 루트 Entity, 전체 Entity 목록, 렌더링 대상 Entity 목록을 포함합니다.
 *
 * @note 추후 Framework 레이어로 이동 예정
 */
struct ModelHierarchyResult
{
	/// 계층 구조의 루트 Entity
	ECS::Entity rootEntity;

	/// 생성된 모든 Entity (루트 포함)
	std::vector<ECS::Entity> allEntities;

	/// MeshComponent를 가진 Entity만 (렌더링 대상)
	std::vector<ECS::Entity> renderableEntities;

	/// 유효성 검사
	bool IsValid() const { return rootEntity.IsValid(); }

	/// 초기화
	void Clear()
	{
		rootEntity = ECS::Entity::Invalid();
		allEntities.clear();
		renderableEntities.clear();
	}

	/// 생성된 Entity 수
	Core::size_t GetTotalCount() const { return allEntities.size(); }

	/// 렌더링 대상 Entity 수
	Core::size_t GetRenderableCount() const { return renderableEntities.size(); }
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
	// 모델 로딩 헬퍼 (중복 코드 제거)
	//=========================================================================

	/**
	 * @brief glTF 모델에서 메시를 로드하고 GPU 리소스 생성
	 *
	 * LoadedModelData의 모든 메시를 하나로 병합하고 서브메시 정보를 생성합니다.
	 * MeshAsset을 통해 GPU 버퍼를 생성합니다.
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

	/**
	 * @brief LoadedModelData에서 머티리얼들을 생성
	 *
	 * 각 머티리얼에 대해:
	 * 1. MaterialResource 생성
	 * 2. 텍스처 로드 (임베디드/외부/폴백)
	 * 3. Descriptor 할당
	 *
	 * @param modelData 로드된 모델 데이터
	 * @param materialNamePrefix 머티리얼 이름 접두어
	 * @param outMaterialIds 생성된 머티리얼 ID 목록 (출력)
	 */
	void CreateMaterialsFromModelData(
		Framework::LoadedModelData& modelData,
		const std::string& materialNamePrefix,
		std::vector<Framework::ResourceId>& outMaterialIds
	);

	//=========================================================================
	// 계층 모델 로딩 (Phase 4.5)
	//=========================================================================

	/**
	 * @brief glTF 노드 계층을 ECS Entity 계층으로 변환
	 *
	 * @param modelData 로드된 모델 데이터
	 * @param materialIds 미리 생성된 Material ID 목록
	 * @param rootPosition 루트 Entity의 월드 위치 (기본: 원점)
	 * @return 생성된 Entity 정보
	 */
	ModelHierarchyResult CreateModelHierarchy(
		const Framework::LoadedModelData& modelData,
		const std::vector<Framework::ResourceId>& materialIds,
		const Math::Vector3& rootPosition = Math::Vector3::Zero()
	);

	/**
	 * @brief 특정 노드의 메시들을 병합하여 GPU 리소스 생성
	 *
	 * 노드가 참조하는 meshIndices의 메시들을 하나로 병합하고
	 * 서브메시 정보를 생성합니다.
	 * MeshAsset 소유권은 AssetManager로 이전됩니다.
	 *
	 * @param meshIndices 병합할 메시 인덱스 목록
	 * @param modelData 원본 모델 데이터
	 * @param meshName 생성할 리소스 이름
	 * @return 생성된 MeshResource ID (실패 시 Invalid)
	 */
	Framework::ResourceId CreateMeshFromNodeIndices(
		const std::vector<Core::uint32>& meshIndices,
		const Framework::LoadedModelData& modelData,
		const std::string& meshName
	);

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
	void RenderModelInfoPanelTextures();

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

	// Procedural Mesh 재생성 플래그
	bool mNeedsMeshRebuild = false;

	// 로드 상태 플래그
	bool mLoadedMeshValid = false;
	bool mHelmetMeshValid = false;
};
