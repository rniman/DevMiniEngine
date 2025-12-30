/**
 * @file ModelViewerApp.cpp
 * @brief ModelViewerApp 구현
 */
#include "ModelViewerApp.h"

 // Framework
#include "Framework/Assets/AssetManager.h"
#include "Framework/Assets/AssetTypes.h"
#include "Framework/Assets/MeshAsset.h"
#include "Framework/Assets/MikkTSpaceCalculator.h"
#include "Framework/Assets/ModelLoader.h"
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
	// 카메라 기본값 (세 개의 오브젝트를 보기 위해 더 멀리)
	constexpr Math::Vector3 DEFAULT_CAMERA_POS = { 0.0f, 0.0f, -10.0f };
	constexpr Math::Vector3 DEFAULT_CAMERA_TARGET = { 0.0f, 0.0f, 0.0f };
	constexpr Math::Vector3 DEFAULT_CAMERA_UP = { 0.0f, 1.0f, 0.0f };
	constexpr Core::float32 DEFAULT_FOV = 60.0f;
	constexpr Core::float32 DEFAULT_NEAR_CLIP = 0.1f;
	constexpr Core::float32 DEFAULT_FAR_CLIP = 100.0f;

	// 조명 기본값
	constexpr Math::Vector3 DEFAULT_LIGHT_DIR = { 0.3f, -1.0f, 0.5f };
	constexpr Math::Vector3 DEFAULT_LIGHT_COLOR = { 1.0f, 0.98f, 0.95f };
	constexpr Core::float32 DEFAULT_LIGHT_INTENSITY = 1.0f;

	// 오브젝트 위치
	constexpr Math::Vector3 PROCEDURAL_SPHERE_POS = { -3.5f, 0.0f, 0.0f };
	constexpr Math::Vector3 LOADED_SPHERE_POS = { 0.0f, 0.0f, 0.0f };
	constexpr Math::Vector3 HELMET_POS = { 3.5f, 0.0f, 0.0f };

	// 모델 경로
	constexpr const char* SPHERE_MODEL_PATH = "../../Assets/Models/Sphere.glb";
	constexpr const char* HELMET_MODEL_PATH = "../../Assets/Models/DamagedHelmet.glb";

	// UI 단축키
	constexpr Platform::KeyCode KEY_TOGGLE_ASSET_PANEL = Platform::KeyCode::F4;
	constexpr Platform::KeyCode KEY_TOGGLE_SPHERE_PANEL = Platform::KeyCode::F5;
	constexpr Platform::KeyCode KEY_TOGGLE_MODEL_PANEL = Platform::KeyCode::F6;
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
	LOG_INFO("=== Phase 4.2: Model Loading Demo ===");

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
	mSystemManager->RegisterSystem<ECS::TransformSystem>();
	mSystemManager->RegisterSystem<ECS::CameraSystem>();
	mSystemManager->RegisterSystem<ECS::LightingSystem>();
	mSystemManager->RegisterSystem<ECS::RenderSystem>(mResourceManager.get());

	// 공유 머티리얼 먼저 생성
	mSharedMaterialId = mResourceManager->CreateMaterial(
		"SharedSphereMaterial",
		L"../../Assets/Shaders/PhongVS.hlsl",
		L"../../Assets/Shaders/PhongPS.hlsl"
	);
	SetupSharedMaterial();

	// Scene 구성
	CreateCameraEntity();
	CreateLightEntities();
	CreateProceduralSphereEntity();
	CreateLoadedSphereEntity();
	CreateHelmetEntity();

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

	mDirectionalLightEntity = mRegistry->CreateEntity();

	ECS::DirectionalLightComponent dirLight;
	dirLight.direction = Math::Normalize(DEFAULT_LIGHT_DIR);
	dirLight.color = DEFAULT_LIGHT_COLOR;
	dirLight.intensity = DEFAULT_LIGHT_INTENSITY;
	dirLight.castsShadow = false;

	mRegistry->AddComponent(mDirectionalLightEntity, dirLight);

	LOG_INFO("[Scene] Directional Light created");
}

//=============================================================================
// Entity 생성
//=============================================================================

void ModelViewerApp::CreateProceduralSphereEntity()
{
	LOG_INFO("[Scene] Creating Procedural Sphere Entity...");

	mProceduralSphereEntity = mRegistry->CreateEntity();

	// Transform (왼쪽)
	ECS::TransformComponent transform;
	transform.position = PROCEDURAL_SPHERE_POS;
	transform.scale = Math::Vector3(1.0f, 1.0f, 1.0f);
	ECS::TransformSystem::SetRotationEuler(transform, 0.0f, 0.0f, 0.0f);
	mRegistry->AddComponent(mProceduralSphereEntity, transform);

	// Mesh 리소스 생성
	mProceduralMeshId = mResourceManager->CreateMesh("ProceduralSphereMesh");

	// Mesh Component
	ECS::MeshComponent meshComp;
	meshComp.meshId = mProceduralMeshId;
	mRegistry->AddComponent(mProceduralSphereEntity, meshComp);

	// Material Component (공유)
	ECS::MaterialComponent matComp;
	matComp.materialId = mSharedMaterialId;
	mRegistry->AddComponent(mProceduralSphereEntity, matComp);

	// Mesh 설정
	SetupProceduralSphereMesh(mSphereSegments, mSphereRings);

	LOG_INFO("[Scene] Procedural Sphere created at (%.1f, %.1f, %.1f)",
		PROCEDURAL_SPHERE_POS.x, PROCEDURAL_SPHERE_POS.y, PROCEDURAL_SPHERE_POS.z);
}

void ModelViewerApp::CreateLoadedSphereEntity()
{
	LOG_INFO("[Scene] Creating Loaded Sphere Entity...");

	mLoadedSphereEntity = mRegistry->CreateEntity();

	// Transform (중앙)
	ECS::TransformComponent transform;
	transform.position = LOADED_SPHERE_POS;
	transform.scale = Math::Vector3(1.0f, 1.0f, 1.0f);
	ECS::TransformSystem::SetRotationEuler(transform, 0.0f, 0.0f, 0.0f);
	mRegistry->AddComponent(mLoadedSphereEntity, transform);

	// Mesh 설정 (glTF 로드 → MeshAsset → GPU Mesh)
	// CreateMeshFromAsset()에서 ResourceId 생성
	SetupLoadedSphereMesh();

	// Mesh Component
	ECS::MeshComponent meshComp;
	meshComp.meshId = mLoadedMeshId;
	mRegistry->AddComponent(mLoadedSphereEntity, meshComp);

	// Material Component (공유)
	ECS::MaterialComponent matComp;
	matComp.materialId = mSharedMaterialId;
	mRegistry->AddComponent(mLoadedSphereEntity, matComp);

	LOG_INFO("[Scene] Loaded Sphere created at (%.1f, %.1f, %.1f)",
		LOADED_SPHERE_POS.x, LOADED_SPHERE_POS.y, LOADED_SPHERE_POS.z);
}

void ModelViewerApp::CreateHelmetEntity()
{
	LOG_INFO("[Scene] Creating DamagedHelmet Entity...");

	mHelmetEntity = mRegistry->CreateEntity();

	// Transform (오른쪽)
	ECS::TransformComponent transform;
	transform.position = HELMET_POS;
	transform.scale = Math::Vector3(1.0f, 1.0f, 1.0f);
	ECS::TransformSystem::SetRotationEuler(transform, 0.0f, 0.0f, 0.0f);
	mRegistry->AddComponent(mHelmetEntity, transform);

	// Mesh 설정 (glTF 로드 → MeshAsset → GPU Mesh)
	SetupHelmetMesh();

	// Mesh Component
	ECS::MeshComponent meshComp;
	meshComp.meshId = mHelmetMeshId;
	mRegistry->AddComponent(mHelmetEntity, meshComp);

	// Material Component (공유 - 텍스처는 아직 미적용)
	ECS::MaterialComponent matComp;
	matComp.materialId = mSharedMaterialId;
	mRegistry->AddComponent(mHelmetEntity, matComp);

	LOG_INFO("[Scene] DamagedHelmet created at (%.1f, %.1f, %.1f)",
		HELMET_POS.x, HELMET_POS.y, HELMET_POS.z);
}

//=============================================================================
// 메시 설정
//=============================================================================

void ModelViewerApp::SetupProceduralSphereMesh(Core::uint32 segments, Core::uint32 rings)
{
	auto* mesh = mResourceManager->GetMesh(mProceduralMeshId);
	if (!mesh)
	{
		LOG_ERROR("[Mesh] Procedural sphere mesh not found!");
		return;
	}

	// PrimitiveGenerator로 구 생성
	auto sphereData = Graphics::PrimitiveGenerator::GenerateSphere(1.0f, segments, rings);

	// MikkTSpace로 Tangent 계산
	std::vector<Math::Vector4> tangents;
	Framework::MikkTSpaceCalculator::Calculate(
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

	LOG_DEBUG("[Mesh] Procedural sphere mesh created: %zu vertices, %zu indices",
		vertices.size(), sphereData.indices.size());
}

void ModelViewerApp::SetupLoadedSphereMesh()
{
	// glTF 모델 로드 (단일 메시)
	Framework::LoadedMeshData meshData;
	if (!Framework::ModelLoader::LoadMesh(SPHERE_MODEL_PATH, meshData))
	{
		LOG_ERROR("[Mesh] Failed to load sphere model: %s", SPHERE_MODEL_PATH);
		mLoadedMeshValid = false;
		return;
	}

	// MeshAsset 생성 및 데이터 설정
	mLoadedSphereMeshAsset = std::make_unique<Framework::MeshAsset>();

	// MeshAsset에 데이터 설정 (같은 타입이므로 직접 이동)
	mLoadedSphereMeshAsset->SetVertices(std::move(meshData.vertices));
	mLoadedSphereMeshAsset->SetIndices(std::move(meshData.indices));
	mLoadedSphereMeshAsset->SetSubmeshes(std::move(meshData.submeshes));
	mLoadedSphereMeshAsset->SetAABB(meshData.aabbMin, meshData.aabbMax);
	mLoadedSphereMeshAsset->SetDataPolicy(Framework::MeshDataPolicy::ReleaseAfterUpload);

	// ResourceManager를 통해 GPU Mesh 생성
	mLoadedMeshId = mResourceManager->CreateMeshFromAsset("LoadedSphereMesh", mLoadedSphereMeshAsset.get());

	if (!mLoadedMeshId.IsValid())
	{
		LOG_ERROR("[Mesh] Failed to create GPU mesh from asset");
		mLoadedMeshValid = false;
		return;
	}

	mLoadedMeshValid = true;

	LOG_INFO("[Mesh] Loaded sphere mesh via Asset Pipeline (V:%u, I:%u, AABB: [%.2f,%.2f,%.2f]-[%.2f,%.2f,%.2f])",
		mLoadedSphereMeshAsset->GetVertexCount(),
		mLoadedSphereMeshAsset->GetIndexCount(),
		mLoadedSphereMeshAsset->GetAABBMin().x,
		mLoadedSphereMeshAsset->GetAABBMin().y,
		mLoadedSphereMeshAsset->GetAABBMin().z,
		mLoadedSphereMeshAsset->GetAABBMax().x,
		mLoadedSphereMeshAsset->GetAABBMax().y,
		mLoadedSphereMeshAsset->GetAABBMax().z);
}

void ModelViewerApp::SetupHelmetMesh()
{
	// 전체 모델 로드 (메시 + 머티리얼 + 텍스처 경로)
	if (!Framework::ModelLoader::LoadModel(HELMET_MODEL_PATH, mHelmetModelData))
	{
		LOG_ERROR("[Mesh] Failed to load helmet model: %s", HELMET_MODEL_PATH);
		mHelmetMeshValid = false;
		return;
	}

	if (mHelmetModelData.meshes.empty())
	{
		LOG_ERROR("[Mesh] Helmet model has no meshes");
		mHelmetMeshValid = false;
		return;
	}

	// 첫 번째 메시 데이터
	auto& meshData = mHelmetModelData.meshes[0];

	// MeshAsset 생성 및 데이터 설정
	mHelmetMeshAsset = std::make_unique<Framework::MeshAsset>();

	// MeshAsset에 데이터 설정 (같은 타입이므로 직접 이동)
	mHelmetMeshAsset->SetVertices(std::move(meshData.vertices));
	mHelmetMeshAsset->SetIndices(std::move(meshData.indices));
	mHelmetMeshAsset->SetSubmeshes(std::move(meshData.submeshes));
	mHelmetMeshAsset->SetAABB(meshData.aabbMin, meshData.aabbMax);
	mHelmetMeshAsset->SetDataPolicy(Framework::MeshDataPolicy::ReleaseAfterUpload);

	// ResourceManager를 통해 GPU Mesh 생성
	mHelmetMeshId = mResourceManager->CreateMeshFromAsset("HelmetMesh", mHelmetMeshAsset.get());

	if (!mHelmetMeshId.IsValid())
	{
		LOG_ERROR("[Mesh] Failed to create GPU mesh from helmet asset");
		mHelmetMeshValid = false;
		return;
	}

	mHelmetMeshValid = true;

	LOG_INFO("[Mesh] Loaded helmet mesh via Asset Pipeline (V:%u, I:%u)",
		mHelmetMeshAsset->GetVertexCount(),
		mHelmetMeshAsset->GetIndexCount());

	// 머티리얼 정보 로그
	for (size_t i = 0; i < mHelmetModelData.materials.size(); ++i)
	{
		const auto& mat = mHelmetModelData.materials[i];
		LOG_INFO("[Material %zu] %s: %zu textures", i, mat.name.c_str(), mat.textures.size());
		for (const auto& tex : mat.textures)
		{
			LOG_INFO("  - %s: %s",
				Graphics::TextureTypeToString(tex.type),
				tex.path.c_str());
		}
	}
}

void ModelViewerApp::SetupSharedMaterial()
{
	auto* material = mResourceManager->GetMaterial(mSharedMaterialId);
	if (!material)
	{
		LOG_ERROR("[Material] Shared material not found!");
		return;
	}

	// 텍스처 로드 및 바인딩
	auto albedoTexId = mResourceManager->LoadTexture("../../Assets/Textures/BrickWall17_1K_BaseColor.png");
	if (albedoTexId.IsValid())
	{
		material->SetTexture(Graphics::TextureType::Albedo, albedoTexId);
	}

	// Descriptor 할당
	if (!material->AllocateDescriptors(
		GetDevice()->GetDevice(),
		GetRenderer()->GetSrvDescriptorHeap(),
		mResourceManager.get()
	))
	{
		LOG_ERROR("[Material] Failed to allocate descriptors");
		return;
	}

	LOG_INFO("[Material] Shared material setup complete");
}

//=============================================================================
// Update / Render
//=============================================================================

void ModelViewerApp::OnUpdate(Core::float32 deltaTime)
{
	// Procedural Mesh 재생성 (지연 적용)
	if (mNeedsMeshRebuild)
	{
		GetDevice()->GetCommandQueue()->WaitForIdle();
		SetupProceduralSphereMesh(mSphereSegments, mSphereRings);
		mNeedsMeshRebuild = false;
	}

	// 패널 토글
	if (GetWindow()->GetInput().IsKeyPressed(KEY_TOGGLE_ASSET_PANEL))
	{
		mShowAssetManagerPanel = !mShowAssetManagerPanel;
	}
	if (GetWindow()->GetInput().IsKeyPressed(KEY_TOGGLE_SPHERE_PANEL))
	{
		mShowSphereControlPanel = !mShowSphereControlPanel;
	}
	if (GetWindow()->GetInput().IsKeyPressed(KEY_TOGGLE_MODEL_PANEL))
	{
		mShowModelInfoPanel = !mShowModelInfoPanel;
	}

	auto* transformSystem = mSystemManager->GetSystem<ECS::TransformSystem>();

	// 회전 업데이트
	mRotationAngle += mRotationSpeed * deltaTime;

	if (transformSystem)
	{
		// Procedural Sphere 회전
		transformSystem->SetRotationEuler(
			mProceduralSphereEntity,
			Math::Vector3(0.0f, mRotationAngle, 0.0f)
		);

		// Loaded Sphere 회전
		transformSystem->SetRotationEuler(
			mLoadedSphereEntity,
			Math::Vector3(0.0f, mRotationAngle, 0.0f)
		);

		// Helmet 회전
		transformSystem->SetRotationEuler(
			mHelmetEntity,
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
	if (mProceduralSphereEntity.IsValid() && mRegistry)
	{
		mRegistry->DestroyEntity(mProceduralSphereEntity);
	}

	if (mLoadedSphereEntity.IsValid() && mRegistry)
	{
		mRegistry->DestroyEntity(mLoadedSphereEntity);
	}

	if (mHelmetEntity.IsValid() && mRegistry)
	{
		mRegistry->DestroyEntity(mHelmetEntity);
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

	// Model Info 패널
	if (mShowModelInfoPanel)
	{
		RenderModelInfoPanel();
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

		ImGui::Text("Asset Root: %s", mAssetManager->GetAssetRoot().c_str());
		ImGui::Text("Loaded Assets: %u", mAssetManager->GetLoadedAssetCount());
		ImGui::Separator();

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
	ImGui::SetNextWindowSize(ImVec2(320, 280), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Sphere Comparison (F5)", &mShowSphereControlPanel))
	{
		if (ImGui::CollapsingHeader("Objects", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Text("Left: Procedural Sphere");
			ImGui::Text("Center: Loaded Sphere (%s)", mLoadedMeshValid ? "OK" : "Failed");
			ImGui::Text("Right: DamagedHelmet (%s)", mHelmetMeshValid ? "OK" : "Failed");
		}

		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderFloat("Rotation Speed", &mRotationSpeed, 0.0f, 3.0f);
		}

		if (ImGui::CollapsingHeader("Procedural Mesh", ImGuiTreeNodeFlags_DefaultOpen))
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
	}
	ImGui::End();
}

void ModelViewerApp::RenderModelInfoPanel()
{
	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Model Info (F6)", &mShowModelInfoPanel))
	{
		if (!mHelmetMeshValid)
		{
			ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "DamagedHelmet not loaded");
			ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1),
				"Place DamagedHelmet.glb in Assets/Models/");
			ImGui::End();
			return;
		}

		// 모델 요약
		ImGui::Text("Model: %s", mHelmetModelData.name.c_str());
		ImGui::Text("Meshes: %zu", mHelmetModelData.meshes.size());
		ImGui::Text("Materials: %zu", mHelmetModelData.materials.size());
		ImGui::Text("Nodes: %zu", mHelmetModelData.nodes.size());
		ImGui::Text("Total Vertices: %u", mHelmetModelData.GetTotalVertexCount());
		ImGui::Text("Total Indices: %u", mHelmetModelData.GetTotalIndexCount());
		ImGui::Text("Total Textures: %u", mHelmetModelData.GetTotalTextureCount());
		ImGui::Separator();

		// Asset Pipeline 정보
		if (ImGui::CollapsingHeader("Asset Pipeline", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Text("MeshAsset Source Data: %s",
				mHelmetMeshAsset && mHelmetMeshAsset->HasSourceData() ? "Retained" : "Released");

			if (mHelmetMeshAsset)
			{
				ImGui::Text("Index Format: %s",
					mHelmetMeshAsset->CanUse16BitIndices() ? "16-bit" : "32-bit");

				// Bounding Box 정보
				const auto& aabbMin = mHelmetMeshAsset->GetAABBMin();
				const auto& aabbMax = mHelmetMeshAsset->GetAABBMax();
				ImGui::Text("AABB Min: (%.2f, %.2f, %.2f)", aabbMin.x, aabbMin.y, aabbMin.z);
				ImGui::Text("AABB Max: (%.2f, %.2f, %.2f)", aabbMax.x, aabbMax.y, aabbMax.z);
				ImGui::Text("Bounding Sphere R: %.2f", mHelmetMeshAsset->GetBoundingSphereRadius());
			}
		}

		// 머티리얼 목록
		if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (size_t i = 0; i < mHelmetModelData.materials.size(); ++i)
			{
				const auto& mat = mHelmetModelData.materials[i];

				if (ImGui::TreeNode(reinterpret_cast<void*>(i), "Material %zu: %s", i, mat.name.c_str()))
				{
					// PBR 값
					ImGui::Text(
						"Base Color: (%.2f, %.2f, %.2f, %.2f)",
						mat.baseColorFactor.x,
						mat.baseColorFactor.y,
						mat.baseColorFactor.z,
						mat.baseColorFactor.w
					);
					ImGui::Text("Metallic: %.2f", mat.metallicFactor);
					ImGui::Text("Roughness: %.2f", mat.roughnessFactor);
					ImGui::Text(
						"Emissive: (%.2f, %.2f, %.2f)",
						mat.emissiveFactor.x,
						mat.emissiveFactor.y,
						mat.emissiveFactor.z
					);

					// 텍스처 목록
					if (!mat.textures.empty())
					{
						ImGui::Separator();
						ImGui::Text("Textures (%zu):", mat.textures.size());
						for (const auto& tex : mat.textures)
						{
							const char* typeName = Graphics::TextureTypeToString(tex.type);
							ImGui::BulletText("[%s] %s", typeName, tex.path.c_str());
						}
					}

					ImGui::TreePop();
				}
			}
		}

		// 메시 목록
		if (ImGui::CollapsingHeader("Meshes"))
		{
			// 첫 번째 메시는 MeshAsset에서 정보 가져오기 (데이터가 move됨)
			if (mHelmetMeshAsset)
			{
				ImGui::Text("Mesh 0: %s (%u verts, %u indices)",
					mHelmetModelData.meshes[0].name.c_str(),
					mHelmetMeshAsset->GetVertexCount(),
					mHelmetMeshAsset->GetIndexCount());
			}

			// 나머지 메시 (있다면)
			for (size_t i = 1; i < mHelmetModelData.meshes.size(); ++i)
			{
				const auto& mesh = mHelmetModelData.meshes[i];
				ImGui::Text("Mesh %zu: %s (%zu verts, %zu indices)",
					i, mesh.name.c_str(),
					mesh.vertices.size(), mesh.indices.size());
			}
		}
	}
	ImGui::End();
}
