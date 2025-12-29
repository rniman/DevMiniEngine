/**
 * @file ModelViewerApp.cpp
 * @brief ModelViewerApp 구현
 */
#include "ModelViewerApp.h"

 // Framework
#include "Framework/Assets/AssetManager.h"
#include "Framework/Assets/AssetTypes.h"
#include "Framework/Assets/MeshAsset.h"
#include "Framework/Assets/TextureAsset.h"
#include "Framework/Assets/MaterialAsset.h"
#include "Framework/Assets/ModelAsset.h"
#include "Framework/Resources/ResourceId.h"
#include "Framework/Resources/ResourceManager.h"
#include "Framework/DebugUI/ECSInspector.h"

// Core
#include "Core/Logging/LogMacros.h"

// Math
#include "Math/MathTypes.h"
#include "Math/MathUtils.h"
#include "Math/MeshUtils.h"

// Platform
#include "Platform/Input.h"
#include "Platform/PlatformTypes.h"
#include "Platform/Window.h"

// Graphics
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"
#include "Graphics/Primitives/PrimitiveGenerator.h"
#include "Graphics/RenderTypes.h"
#include "Graphics/TextureType.h"

// Graphics - DX12
#include "Graphics/DX12/DX12Device.h"
#include "Graphics/DX12/DX12CommandQueue.h"
#include "Graphics/DX12/DX12Renderer.h"

// ECS
#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/LightComponents.h"
#include "ECS/Components/MaterialComponent.h"
#include "ECS/Components/MeshComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/SystemManager.h"
#include "ECS/Registry.h"
#include "ECS/Systems/CameraSystem.h"
#include "ECS/Systems/LightingSystem.h"
#include "ECS/Systems/RenderSystem.h"
#include "ECS/Systems/TransformSystem.h"

// Third Party
#include <imgui.h>

// Standard Library
#include <cmath>
#include <memory>

//=============================================================================
// 상수
//=============================================================================

namespace
{
	// 카메라 기본값
	constexpr Math::Vector3 DEFAULT_CAMERA_POS = { 0.0f, 0.0f, -5.0f };
	constexpr Math::Vector3 DEFAULT_CAMERA_TARGET = { 0.0f, 0.0f, 0.0f };
	constexpr Math::Vector3 DEFAULT_CAMERA_UP = { 0.0f, 1.0f, 0.0f };
	constexpr Core::float32 DEFAULT_FOV = 60.0f;
	constexpr Core::float32 DEFAULT_NEAR_CLIP = 0.1f;
	constexpr Core::float32 DEFAULT_FAR_CLIP = 100.0f;

	// 조명 기본값
	constexpr Math::Vector3 DEFAULT_LIGHT_DIR = { 0.3f, -1.0f, 0.5f };
	constexpr Math::Vector3 DEFAULT_LIGHT_COLOR = { 1.0f, 0.98f, 0.95f };
	constexpr Core::float32 DEFAULT_LIGHT_INTENSITY = 1.0f;

	// UI 단축키 (F1: Performance, F2: ECS Inspector, F3: Debug Vis 사용 중)
	constexpr Platform::KeyCode KEY_TOGGLE_ASSET_PANEL = Platform::KeyCode::F4;
	constexpr Platform::KeyCode KEY_TOGGLE_SPHERE_PANEL = Platform::KeyCode::F5;
}

//=============================================================================
// 생성자/소멸자
//=============================================================================

ModelViewerApp::ModelViewerApp()
	: Application(GetAppDesc())
{
}

ModelViewerApp::~ModelViewerApp()
{
}

//=============================================================================
// 라이프사이클
//=============================================================================

bool ModelViewerApp::OnInitialize()
{
	LOG_INFO("=== Phase 4.1: Asset Pipeline Demo ===");

	// ResourceManager 생성
	mResourceManager = std::make_unique<Framework::ResourceManager>(
		GetDevice(),
		GetRenderer()
	);

	// AssetManager 생성
	mAssetManager = std::make_unique<Framework::AssetManager>(
		mResourceManager.get()
	);

	if (!mAssetManager->Initialize("Assets/"))
	{
		LOG_ERROR("Failed to initialize AssetManager");
		return false;
	}

	// 렌더링 리소스 초기화
	if (!GetRenderer()->Initialize(
		GetDevice(),
		GetAppDesc().windowWidth,
		GetAppDesc().windowHeight
	))
	{
		LOG_ERROR("Failed to initialize rendering resources");
		return false;
	}

	// ECS 초기화
	InitializeECS();

	LOG_INFO("[ModelViewer] Initialization complete");
	return true;
}

void ModelViewerApp::InitializeECS()
{
	LOG_INFO("[ECS] Initializing Registry...");

	// Registry 생성
	mRegistry = std::make_unique<ECS::Registry>();

	// SystemManager 생성 (참조로 전달)
	mSystemManager = std::make_unique<ECS::SystemManager>(*mRegistry);

	// System 등록 (순서 중요!)
	// Transform → Camera → Lighting → Render
	mSystemManager->RegisterSystem<ECS::TransformSystem>();
	mSystemManager->RegisterSystem<ECS::CameraSystem>();
	mSystemManager->RegisterSystem<ECS::LightingSystem>();
	mSystemManager->RegisterSystem<ECS::RenderSystem>(mResourceManager.get());

	// Scene 구성
	CreateCameraEntity();
	CreateLightEntities();
	CreateSphereEntity();

	LOG_INFO("[ECS] Registry initialized");
}

void ModelViewerApp::CreateCameraEntity()
{
	LOG_INFO("[Scene] Creating Camera Entity...");

	mCameraEntity = mRegistry->CreateEntity();

	ECS::TransformComponent transform;
	ECS::CameraComponent camera;

	camera.projectionType = ECS::ProjectionType::Perspective;

	ECS::CameraSystem::SetFovYDegrees(camera, DEFAULT_FOV);
	ECS::CameraSystem::SetAspectRatio(
		camera,
		static_cast<Core::float32>(GetWindow()->GetWidth()),
		static_cast<Core::float32>(GetWindow()->GetHeight())
	);
	ECS::CameraSystem::SetClipPlanes(camera, DEFAULT_NEAR_CLIP, DEFAULT_FAR_CLIP);
	camera.isMainCamera = true;

	ECS::CameraSystem::SetLookAt(
		transform,
		camera,
		DEFAULT_CAMERA_POS,
		DEFAULT_CAMERA_TARGET,
		DEFAULT_CAMERA_UP
	);

	mRegistry->AddComponent(mCameraEntity, transform);
	mRegistry->AddComponent(mCameraEntity, camera);

	LOG_INFO("[Scene] Camera created (Main Camera)");
}

void ModelViewerApp::CreateLightEntities()
{
	LOG_INFO("[Scene] Creating Light Entities...");

	// Directional Light
	mDirectionalLightEntity = mRegistry->CreateEntity();

	ECS::DirectionalLightComponent dirLight;
	dirLight.direction = Math::Normalize(DEFAULT_LIGHT_DIR);
	dirLight.color = DEFAULT_LIGHT_COLOR;
	dirLight.intensity = DEFAULT_LIGHT_INTENSITY;
	dirLight.castsShadow = false;

	mRegistry->AddComponent(mDirectionalLightEntity, dirLight);

	LOG_INFO("[Scene] Directional Light created");
}

void ModelViewerApp::CreateSphereEntity()
{
	LOG_INFO("[Scene] Creating Sphere Entity...");

	mSphereEntity = mRegistry->CreateEntity();

	// Transform
	ECS::TransformComponent transform;
	transform.position = Math::Vector3(0.0f, 0.0f, 0.0f);
	transform.scale = Math::Vector3(1.0f, 1.0f, 1.0f);
	ECS::TransformSystem::SetRotationEuler(transform, 0.0f, 0.0f, 0.0f);
	mRegistry->AddComponent(mSphereEntity, transform);

	// 공유 리소스 생성
	mSphereMeshId = mResourceManager->CreateMesh("SphereMesh");
	mSphereMaterialId = mResourceManager->CreateMaterial(
		"SphereMaterial",
		L"../../Assets/Shaders/PhongVS.hlsl",
		L"../../Assets/Shaders/PhongPS.hlsl"
	);

	// Mesh Component
	ECS::MeshComponent meshComp;
	meshComp.meshId = mSphereMeshId;
	mRegistry->AddComponent(mSphereEntity, meshComp);

	// Material Component
	ECS::MaterialComponent matComp;
	matComp.materialId = mSphereMaterialId;
	mRegistry->AddComponent(mSphereEntity, matComp);

	// Mesh와 Material 설정
	SetupSphereMesh(mSphereSegments, mSphereRings);
	SetupSphereMaterial();

	LOG_INFO("[Scene] Sphere Entity created (segments=%u, rings=%u)",
		mSphereSegments, mSphereRings);
}

//=============================================================================
// 프로시저럴 메시 생성
//=============================================================================

void ModelViewerApp::SetupSphereMesh(Core::uint32 segments, Core::uint32 rings)
{
	auto* mesh = mResourceManager->GetMesh(mSphereMeshId);
	if (!mesh)
	{
		LOG_ERROR("[Mesh] Sphere mesh not found!");
		return;
	}

	// PrimitiveGenerator로 구 생성
	auto sphereData = Graphics::PrimitiveGenerator::GenerateSphere(1.0f, segments, rings);

	// Tangent 계산
	std::vector<Math::Vector3> tangents;
	Math::CalculateTangents(
		sphereData.positions,
		sphereData.normals,
		sphereData.texCoords,
		sphereData.indices,
		tangents
	);

	// StandardVertex 구조체로 변환
	std::vector<Graphics::StandardVertex> vertices;
	vertices.reserve(sphereData.positions.size());

	for (Core::size_t i = 0; i < sphereData.positions.size(); ++i)
	{
		Graphics::StandardVertex v;
		v.position = sphereData.positions[i];
		v.normal = sphereData.normals[i];
		v.texCoord = sphereData.texCoords[i];
		v.tangent = tangents[i];
		vertices.push_back(v);
	}

	// GPU 버퍼 생성
	mesh->InitializeStandard(
		GetDevice()->GetDevice(),
		GetDevice()->GetCommandQueue(),
		GetDevice()->GetCommandContext(GetRenderer()->GetCurrentFrameIndex()),
		vertices.data(),
		static_cast<Core::uint32>(vertices.size()),
		sphereData.indices.data(),
		static_cast<Core::uint32>(sphereData.indices.size())
	);

	LOG_DEBUG("[Mesh] Sphere mesh created: %zu vertices, %zu indices",
		vertices.size(), sphereData.indices.size());
}

void ModelViewerApp::SetupSphereMaterial()
{
	auto* material = mResourceManager->GetMaterial(mSphereMaterialId);
	if (!material)
	{
		LOG_ERROR("[Material] Sphere material not found!");
		return;
	}

	// 텍스처 로드 및 바인딩
	auto albedoTexId = mResourceManager->LoadTexture("../../Assets/Textures/BrickWall17_1K_BaseColor.png");
	if (albedoTexId.IsValid())
	{
		material->SetTexture(Graphics::TextureType::Albedo, albedoTexId);
	}

	// Descriptor 할당 (SetTexture 후에 호출해야 실제 SRV 생성됨)
	if (!material->AllocateDescriptors(
		GetDevice()->GetDevice(),
		GetRenderer()->GetSrvDescriptorHeap(),
		mResourceManager.get()
	))
	{
		LOG_ERROR("[Material] Failed to allocate descriptors");
		return;
	}

	LOG_INFO("[Material] Sphere material setup complete");
}

//=============================================================================
// Update / Render
//=============================================================================

void ModelViewerApp::OnUpdate(Core::float32 deltaTime)
{
	// Mesh 재생성 (지연 적용)
	if (mNeedsMeshRebuild)
	{
		GetDevice()->GetCommandQueue()->WaitForIdle();
		SetupSphereMesh(mSphereSegments, mSphereRings);
		mNeedsMeshRebuild = false;
	}

	// 패널 토글 (F4: Asset Manager, F5: Sphere Control)
	if (GetWindow()->GetInput().IsKeyPressed(KEY_TOGGLE_ASSET_PANEL))
	{
		mShowAssetManagerPanel = !mShowAssetManagerPanel;
	}
	if (GetWindow()->GetInput().IsKeyPressed(KEY_TOGGLE_SPHERE_PANEL))
	{
		mShowSphereControlPanel = !mShowSphereControlPanel;
	}

	auto* transformSystem = mSystemManager->GetSystem<ECS::TransformSystem>();

	// Sphere 회전
	mRotationAngle += mRotationSpeed * deltaTime;

	if (transformSystem)
	{
		transformSystem->SetRotationEuler(
			mSphereEntity,
			Math::Vector3(0.0f, mRotationAngle, 0.0f)
		);
	}

	// SystemManager 업데이트
	mSystemManager->UpdateSystems(deltaTime);
}

void ModelViewerApp::OnRender()
{
	ECS::RenderSystem* renderSystem = mSystemManager->GetSystem<ECS::RenderSystem>();
	if (renderSystem)
	{
		const Graphics::FrameData& frameData = renderSystem->GetFrameData();

		GetRenderer()->RenderScene(frameData);
		GetRenderer()->RenderDebug(frameData);
	}
}

void ModelViewerApp::OnShutdown()
{
	LOG_INFO("[ModelViewer] Shutting down...");

	GetDevice()->GetCommandQueue()->WaitForIdle();

	// Entities 정리
	if (mSphereEntity.IsValid() && mRegistry)
	{
		mRegistry->DestroyEntity(mSphereEntity);
	}

	if (mDirectionalLightEntity.IsValid() && mRegistry)
	{
		mRegistry->DestroyEntity(mDirectionalLightEntity);
	}

	if (mCameraEntity.IsValid() && mRegistry)
	{
		mRegistry->DestroyEntity(mCameraEntity);
	}

	// 시스템 정리
	mSystemManager.reset();
	mRegistry.reset();

	// Asset/Resource 정리
	if (mAssetManager)
	{
		mAssetManager->Shutdown();
		mAssetManager.reset();
	}

	mResourceManager.reset();

	LOG_INFO("[ModelViewer] Shutdown complete");
}

//=============================================================================
// Debug UI
//=============================================================================

void ModelViewerApp::OnRenderDebugUI()
{
	// ECS Inspector
	Framework::ECSInspector* inspector = GetECSInspector();
	if (inspector)
	{
		inspector->Render(mRegistry.get());
	}

	// Asset Manager 패널
	if (mShowAssetManagerPanel)
	{
		RenderAssetManagerPanel();
	}

	// Sphere Control 패널
	if (mShowSphereControlPanel)
	{
		RenderSphereControlPanel();
	}
}

void ModelViewerApp::RenderAssetManagerPanel()
{
	ImGui::SetNextWindowSize(ImVec2(350, 300), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Asset Manager (F4)", &mShowAssetManagerPanel))
	{
		if (!mAssetManager)
		{
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "AssetManager not initialized");
			ImGui::End();
			return;
		}

		// 기본 정보
		ImGui::Text("Asset Root: %s", mAssetManager->GetAssetRoot().c_str());
		ImGui::Text("Loaded Assets: %u", mAssetManager->GetLoadedAssetCount());
		ImGui::Separator();

		// 기본 Asset ID 표시
		if (ImGui::CollapsingHeader("Default Assets", ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto meshId = mAssetManager->GetDefaultAssetId<Framework::MeshAsset>();
			auto texId = mAssetManager->GetDefaultAssetId<Framework::TextureAsset>();
			auto matId = mAssetManager->GetDefaultAssetId<Framework::MaterialAsset>();
			auto modelId = mAssetManager->GetDefaultAssetId<Framework::ModelAsset>();

			ImGui::Text("Mesh:     0x%llX", meshId.id);
			ImGui::Text("Texture:  0x%llX", texId.id);
			ImGui::Text("Material: 0x%llX", matId.id);
			ImGui::Text("Model:    0x%llX", modelId.id);
		}

		// 로드된 Asset 목록
		if (ImGui::CollapsingHeader("Loaded Assets", ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto infos = mAssetManager->GetLoadedAssetInfos(true);

			ImGui::BeginChild("AssetList", ImVec2(0, 120), true);
			for (const auto& info : infos)
			{
				const char* typeName = Framework::AssetTypeToString(info.type);
				const char* stateName = Framework::AssetStateToString(info.state);

				bool isDefault = mAssetManager->IsDefaultAsset(
					mAssetManager->FindByPath(info.path)
				);

				if (isDefault)
				{
					ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f),
						"[%s] %s", typeName, info.path.c_str());
				}
				else
				{
					ImGui::Text("[%s] %s", typeName, info.path.c_str());
				}

				ImGui::SameLine();
				ImGui::TextDisabled("(ref=%u, %s)", info.refCount, stateName);
			}
			ImGui::EndChild();
		}

		// 작업 버튼
		ImGui::Separator();
		if (ImGui::Button("Unload Unused"))
		{
			Core::uint32 count = mAssetManager->UnloadUnusedAssets();
			LOG_INFO("[ModelViewer] Unloaded %u unused assets", count);
		}
	}
	ImGui::End();
}

void ModelViewerApp::RenderSphereControlPanel()
{
	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Sphere Control (F5)", &mShowSphereControlPanel))
	{
		// Sphere 설정
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderFloat("Rotation Speed", &mRotationSpeed, 0.0f, 3.0f);
		}

		// Sphere 해상도 변경
		if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen))
		{
			int segments = static_cast<int>(mSphereSegments);
			int rings = static_cast<int>(mSphereRings);

			bool changed = false;
			changed |= ImGui::SliderInt("Segments", &segments, 8, 64);
			changed |= ImGui::SliderInt("Rings", &rings, 4, 32);

			if (changed)
			{
				mSphereSegments = static_cast<Core::uint32>(segments);
				mSphereRings = static_cast<Core::uint32>(rings);
				mNeedsMeshRebuild = true;
			}
		}

		// Material 설정
		if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto* material = mResourceManager->GetMaterial(mSphereMaterialId);
			if (material)
			{
				ImGui::Text("Shader: PhongVS/PS");
				ImGui::Text("Textures: %u", material->GetTextureCount());
				ImGui::Text("Descriptors: %s",
					material->HasAllocatedDescriptors() ? "Allocated" : "Not Allocated");
			}
		}
	}
	ImGui::End();
}
