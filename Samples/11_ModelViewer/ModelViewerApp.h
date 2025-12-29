/**
 * @file ModelViewerApp.h
 * @brief Phase 4.1: Asset Pipeline 데모 애플리케이션
 *
 * AssetManager 기본 기능 테스트:
 * - Asset 로딩/언로딩
 * - 기본 Asset 폴백
 * - 프로시저럴 Sphere 렌더링
 */
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
	class AssetManager;
}

namespace Graphics
{
	struct FrameData;
}

/**
 * @brief Phase 4.1: Asset Pipeline 데모 애플리케이션
 *
 * AssetManager의 기본 기능을 테스트합니다:
 * - Asset 로딩 API
 * - 기본 Asset (Default Assets)
 * - Asset 상태 확인
 * - ImGui를 통한 Asset 정보 표시
 *
 * 렌더링:
 * - 프로시저럴 UV Sphere
 * - Phong Shading (기존 셰이더 재사용)
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
		desc.windowTitle = "11_ModelViewer - Phase 4.1 Asset Pipeline";
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
	void CreateSphereEntity();

	//=========================================================================
	// 프로시저럴 메시 생성
	//=========================================================================

	/**
	 * @brief UV Sphere 메시 데이터 생성
	 * @param segments 수평 분할 수 (경도)
	 * @param rings 수직 분할 수 (위도)
	 */
	void SetupSphereMesh(Core::uint32 segments, Core::uint32 rings);
	void SetupSphereMaterial();

	//=========================================================================
	// Debug UI
	//=========================================================================

	void RenderAssetManagerPanel();
	void RenderSphereControlPanel();

	//=========================================================================
	// 멤버 변수
	//=========================================================================

	// 리소스 관리
	std::unique_ptr<Framework::ResourceManager> mResourceManager;
	std::unique_ptr<Framework::AssetManager> mAssetManager;

	// ECS
	std::unique_ptr<ECS::Registry> mRegistry;
	std::unique_ptr<ECS::SystemManager> mSystemManager;

	// Entities
	ECS::Entity mCameraEntity;
	ECS::Entity mDirectionalLightEntity;
	ECS::Entity mSphereEntity;

	// 공유 리소스 ID
	Framework::ResourceId mSphereMeshId;
	Framework::ResourceId mSphereMaterialId;

	// 애니메이션
	Core::float32 mRotationAngle = 0.0f;
	Core::float32 mRotationSpeed = 0.5f;

	// Sphere 파라미터
	Core::uint32 mSphereSegments = 32;
	Core::uint32 mSphereRings = 16;

	// UI 상태
	bool mShowAssetManagerPanel = true;
	bool mShowSphereControlPanel = true;
	bool mNeedsMeshRebuild = false;
};
