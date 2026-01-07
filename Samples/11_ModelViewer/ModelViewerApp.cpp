/**
 * @file ModelViewerApp.cpp
 * @brief ModelViewerApp 구현
 */
#include "ModelViewerApp.h"

 // Framework
#include "Framework/Assets/AssetManager.h"
#include "Framework/Assets/AssetTypes.h"
#include "Framework/Assets/MaterialAsset.h"
#include "Framework/Assets/MeshAsset.h"
#include "Framework/Assets/MikkTSpaceCalculator.h"
#include "Framework/Assets/ModelAsset.h"
#include "Framework/Assets/ModelLoader.h"
#include "Framework/Assets/TextureAsset.h"
#include "Framework/DebugUI/ECSInspector.h"
#include "Framework/DebugUI/ImGuiHelper.h"
#include "Framework/Resources/ResourceId.h"
#include "Framework/Resources/ResourceManager.h"

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
#include "Graphics/DX12/DX12CommandQueue.h"
#include "Graphics/DX12/DX12Device.h"
#include "Graphics/DX12/DX12Renderer.h"
#include "Graphics/MaterialResource.h"
#include "Graphics/MeshResource.h"
#include "Graphics/Primitives/PrimitiveGenerator.h"
#include "Graphics/RenderTypes.h"
#include "Graphics/TextureType.h"

// ECS
#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/LightComponents.h"
#include "ECS/Components/MaterialComponent.h"
#include "ECS/Components/MeshComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Registry.h"
#include "ECS/SystemManager.h"
#include "ECS/Systems/CameraSystem.h"
#include "ECS/Systems/LightingSystem.h"
#include "ECS/Systems/RenderSystem.h"
#include "ECS/Systems/TransformSystem.h"

// Third Party
#include <imgui.h>

// Standard Library
#include <algorithm>
#include <cmath>
#include <memory>

//=============================================================================
// 익명 네임스페이스 - 상수 및 헬퍼 함수
//=============================================================================

namespace
{
	//=========================================================================
	// 상수
	//=========================================================================

	// 카메라 기본값 (네 개의 오브젝트를 보기 위해 조정)
	constexpr Math::Vector3 DEFAULT_CAMERA_POS = { 0.0f, 0.0f, -11.0f };
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
	constexpr Math::Vector3 CUBE_POS = { 0.0f, 2.5f, 0.0f };

	// 모델 경로
	constexpr const char* SPHERE_MODEL_PATH = "../../Assets/Models/Sphere.glb";
	// constexpr const char* HELMET_MODEL_PATH = "../../Assets/Models/DamagedHelmet/DamagedHelmet.gltf";
	constexpr const char* HELMET_MODEL_PATH = "../../Assets/Models/DamagedHelmet_Glb/DamagedHelmet.glb";
	constexpr const char* CUBE_MODEL_PATH = "../../Assets/Models/MultiMaterialCube.glb";

	// UI 단축키
	constexpr Platform::KeyCode KEY_TOGGLE_ASSET_PANEL = Platform::KeyCode::F4;
	constexpr Platform::KeyCode KEY_TOGGLE_SPHERE_PANEL = Platform::KeyCode::F5;
	constexpr Platform::KeyCode KEY_TOGGLE_MODEL_PANEL = Platform::KeyCode::F6;

	//=========================================================================
	// UI 헬퍼 함수
	//=========================================================================

	/**
	 * @brief DXGI 포맷을 문자열로 변환
	 */
	const char* DXGIFormatToString(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM:      return "R8G8B8A8_UNORM";
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return "R8G8B8A8_SRGB";
		case DXGI_FORMAT_B8G8R8A8_UNORM:      return "B8G8R8A8_UNORM";
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return "B8G8R8A8_SRGB";
		case DXGI_FORMAT_BC1_UNORM:           return "BC1_UNORM";
		case DXGI_FORMAT_BC1_UNORM_SRGB:      return "BC1_SRGB";
		case DXGI_FORMAT_BC3_UNORM:           return "BC3_UNORM";
		case DXGI_FORMAT_BC3_UNORM_SRGB:      return "BC3_SRGB";
		case DXGI_FORMAT_BC5_UNORM:           return "BC5_UNORM";
		case DXGI_FORMAT_BC7_UNORM:           return "BC7_UNORM";
		case DXGI_FORMAT_BC7_UNORM_SRGB:      return "BC7_SRGB";
		default:                              return "Unknown";
		}
	}

	/**
	 * @brief 텍스처 상태 표시 (임베디드/외부/실패) + TextureAsset 메타데이터
	 *
	 * @param tex LoadedTextureInfo (ModelLoader에서)
	 * @param resourceManager ResourceManager (TextureAsset 조회용, nullable)
	 */
	void TextTextureStatus(
		const Framework::LoadedTextureInfo& tex,
		Framework::ResourceManager* resourceManager = nullptr)
	{
		using namespace Framework;

		const char* typeName = Graphics::TextureTypeToString(tex.type);

		// 텍스처 상태 표시
		if (tex.HasEmbeddedData())
		{
			ImGui::TextColored(UIColor::Highlight, "[%s] %s (embedded)", typeName, tex.path.c_str());
		}
		else if (tex.isEmbedded)
		{
			ImGui::TextColored(UIColor::Warning, "[%s] %s (embedded, no data)", typeName, tex.path.c_str());
		}
		else
		{
			ImGui::BulletText("[%s] %s", typeName, tex.path.c_str());
		}

		// TextureAsset 메타데이터 표시
		if (resourceManager)
		{
			ResourceId texId = resourceManager->FindTextureByPath(tex.path);
			if (texId.IsValid())
			{
				const TextureAsset* asset = resourceManager->GetTextureAsset(texId);
				if (asset)
				{
					ImGui::SameLine();
					ImGui::TextColored(
						UIColor::Disabled,
						"(%ux%u, %s%s)",
						asset->GetWidth(),
						asset->GetHeight(),
						DXGIFormatToString(asset->GetFormat()),
						asset->IsSRGB() ? ", sRGB" : ""
					);
				}
			}
		}
	}

} // anonymous namespace

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
	LOG_INFO("=== Phase 4.4: Multi-Submesh Demo ===");

	// ResourceManager 생성
	mResourceManager = std::make_unique<Framework::ResourceManager>(
		GetDevice(),
		GetRenderer()
	);

	// AssetManager 생성
	mAssetManager = std::make_unique<Framework::AssetManager>(
		mResourceManager.get()
	);

	mResourceManager->SetAssetManager(mAssetManager.get());

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

	// Entity 생성
	CreateCameraEntity();
	CreateLightEntities();
	CreateProceduralSphereEntity();
	CreateLoadedSphereEntity();
	CreateHelmetEntity();
	CreateCubeEntity();

	LOG_INFO("[ECS] Registry initialized");
}

void ModelViewerApp::CreateCameraEntity()
{
	LOG_INFO("[ModelViewerApp] Creating Camera Entity...");

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

	LOG_INFO("[ModelViewerApp] Camera created (Main Camera)");
}

void ModelViewerApp::CreateLightEntities()
{
	LOG_INFO("[ModelViewerApp] Creating Light Entities...");

	mDirectionalLightEntity = mRegistry->CreateEntity();

	ECS::DirectionalLightComponent dirLight;
	dirLight.direction = Math::Normalize(DEFAULT_LIGHT_DIR);
	dirLight.color = DEFAULT_LIGHT_COLOR;
	dirLight.intensity = DEFAULT_LIGHT_INTENSITY;
	dirLight.castsShadow = false;

	mRegistry->AddComponent(mDirectionalLightEntity, dirLight);

	LOG_INFO("[ModelViewerApp] Directional Light created");
}

//=============================================================================
// Entity 생성
//=============================================================================

void ModelViewerApp::CreateProceduralSphereEntity()
{
	LOG_INFO("[ModelViewerApp] Creating Procedural Sphere Entity...");

	mProceduralSphereEntity = mRegistry->CreateEntity();

	// Transform (왼쪽)
	ECS::TransformComponent transform;
	transform.position = PROCEDURAL_SPHERE_POS;
	mRegistry->AddComponent(mProceduralSphereEntity, transform);

	// Mesh 리소스 생성
	mProceduralMeshId = mResourceManager->CreateMesh("ProceduralSphereMesh");

	// Mesh Component
	ECS::MeshComponent meshComp;
	meshComp.meshId = mProceduralMeshId;
	mRegistry->AddComponent(mProceduralSphereEntity, meshComp);

	// Material Component (공유)
	ECS::MaterialComponent matComp;
	matComp.SetSingleMaterial(mSharedMaterialId);
	mRegistry->AddComponent(mProceduralSphereEntity, matComp);

	// Mesh 설정
	SetupProceduralSphereMesh(mSphereSegments, mSphereRings);

	LOG_INFO(
		"[ModelViewerApp] Procedural Sphere created at (%.1f, %.1f, %.1f)",
		PROCEDURAL_SPHERE_POS.x, PROCEDURAL_SPHERE_POS.y, PROCEDURAL_SPHERE_POS.z
	);
}

void ModelViewerApp::CreateLoadedSphereEntity()
{
	LOG_INFO("[ModelViewerApp] Creating Loaded Sphere Entity...");

	mLoadedSphereEntity = mRegistry->CreateEntity();

	// Transform (중앙)
	ECS::TransformComponent transform;
	transform.position = LOADED_SPHERE_POS;
	mRegistry->AddComponent(mLoadedSphereEntity, transform);

	// Mesh 설정 (glTF 로드 → MeshAsset → GPU Mesh)
	SetupLoadedSphereMesh();

	// Mesh Component
	ECS::MeshComponent meshComp;
	meshComp.meshId = mLoadedMeshId;
	mRegistry->AddComponent(mLoadedSphereEntity, meshComp);

	// Material Component (공유)
	ECS::MaterialComponent matComp;
	matComp.SetSingleMaterial(mSharedMaterialId);
	mRegistry->AddComponent(mLoadedSphereEntity, matComp);

	LOG_INFO(
		"[ModelViewerApp] Loaded Sphere created at (%.1f, %.1f, %.1f)",
		LOADED_SPHERE_POS.x, LOADED_SPHERE_POS.y, LOADED_SPHERE_POS.z
	);
}

void ModelViewerApp::CreateHelmetEntity()
{
	LOG_INFO("[ModelViewerApp] Creating DamagedHelmet Entity...");

	mHelmetEntity = mRegistry->CreateEntity();

	// Transform (오른쪽)
	ECS::TransformComponent transform;
	transform.position = HELMET_POS;
	mRegistry->AddComponent(mHelmetEntity, transform);

	// 공통 헬퍼로 메시 로드
	mHelmetMeshValid = LoadAndCreateMesh(
		HELMET_MODEL_PATH,
		"Helmet",
		mHelmetModelData,
		mHelmetMeshAsset,
		mHelmetMeshId
	);

	if (mHelmetMeshValid)
	{
		// 공통 헬퍼로 머티리얼 생성
		CreateMaterialsFromModelData(
			mHelmetModelData,
			mHelmetModelData.name + "_Mat",
			mHelmetMaterialIds
		);
	}

	// Mesh Component
	ECS::MeshComponent meshComp;
	meshComp.meshId = mHelmetMeshId;
	mRegistry->AddComponent(mHelmetEntity, meshComp);

	// Material Component (멀티 서브메시용)
	ECS::MaterialComponent matComp;
	for (size_t i = 0; i < mHelmetMaterialIds.size() && i < ECS::MaterialComponent::MAX_MATERIALS; ++i)
	{
		matComp.SetMaterial(static_cast<Core::uint32>(i), mHelmetMaterialIds[i]);
	}
	mRegistry->AddComponent(mHelmetEntity, matComp);

	LOG_INFO(
		"[ModelViewerApp] DamagedHelmet created at (%.1f, %.1f, %.1f)",
		HELMET_POS.x, HELMET_POS.y, HELMET_POS.z
	);
}

void ModelViewerApp::CreateCubeEntity()
{
	LOG_INFO("[ModelViewerApp] Creating MultiMaterialCube Entity...");

	mCubeEntity = mRegistry->CreateEntity();

	// Transform (상단)
	ECS::TransformComponent transform;
	transform.position = CUBE_POS;
	mRegistry->AddComponent(mCubeEntity, transform);

	// 공통 헬퍼로 메시 로드
	mCubeMeshValid = LoadAndCreateMesh(
		CUBE_MODEL_PATH,
		"Cube",
		mCubeModelData,
		mCubeMeshAsset,
		mCubeMeshId
	);

	if (mCubeMeshValid)
	{
		// 공통 헬퍼로 머티리얼 생성
		CreateMaterialsFromModelData(
			mCubeModelData,
			"CubeMat",
			mCubeMaterialIds
		);
	}

	// Mesh Component
	ECS::MeshComponent meshComp;
	meshComp.meshId = mCubeMeshId;
	mRegistry->AddComponent(mCubeEntity, meshComp);

	// Material Component (멀티 서브메시용)
	ECS::MaterialComponent matComp;
	for (size_t i = 0; i < mCubeMaterialIds.size() && i < ECS::MaterialComponent::MAX_MATERIALS; ++i)
	{
		matComp.SetMaterial(static_cast<Core::uint32>(i), mCubeMaterialIds[i]);
	}
	mRegistry->AddComponent(mCubeEntity, matComp);

	LOG_INFO(
		"[ModelViewerApp] MultiMaterialCube created at (%.1f, %.1f, %.1f)",
		CUBE_POS.x, CUBE_POS.y, CUBE_POS.z
	);
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

	LOG_DEBUG(
		"[Mesh] Procedural sphere mesh created: %zu vertices, %zu indices",
		vertices.size(), sphereData.indices.size()
	);
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

	LOG_INFO(
		"[Mesh] Loaded sphere mesh via Asset Pipeline (V:%u, I:%u)",
		mLoadedSphereMeshAsset->GetVertexCount(),
		mLoadedSphereMeshAsset->GetIndexCount()
	);
}

void ModelViewerApp::SetupSharedMaterial()
{
	auto* material = mResourceManager->GetMaterial(mSharedMaterialId);
	if (!material)
	{
		LOG_ERROR("[Material] Shared material not found!");
		return;
	}

	// Albedo 텍스처 로드
	auto albedoTexId = mResourceManager->LoadTexture(
		"../../Assets/Textures/BrickWall17_1K_BaseColor.png",
		Graphics::TextureType::Albedo
	);

	if (albedoTexId.IsValid())
	{
		material->SetTexture(Graphics::TextureType::Albedo, albedoTexId);
		LOG_INFO("[Material] Loaded Albedo texture");
	}
	else
	{
		LOG_WARN("[Material] Failed to load Albedo texture - using default");
	}

	// Normal 텍스처 로드
	auto normalTexId = mResourceManager->LoadTexture(
		"../../Assets/Textures/BrickWall17_1K_Normal.png",
		Graphics::TextureType::Normal
	);

	if (normalTexId.IsValid())
	{
		material->SetTexture(Graphics::TextureType::Normal, normalTexId);
		LOG_INFO("[Material] Loaded Normal texture");
	}
	else
	{
		LOG_WARN("[Material] Failed to load Normal texture - using default");
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

	LOG_INFO("[Material] Shared material setup complete (textures: %u)", material->GetTextureCount());
}

//=============================================================================
// 모델 로딩 헬퍼 (중복 코드 제거)
//=============================================================================

bool ModelViewerApp::LoadAndCreateMesh(
	const char* modelPath,
	const std::string& modelName,
	Framework::LoadedModelData& outModelData,
	std::unique_ptr<Framework::MeshAsset>& outMeshAsset,
	Framework::ResourceId& outMeshId)
{
	// 모델 로드
	if (!Framework::ModelLoader::LoadModel(modelPath, outModelData))
	{
		LOG_ERROR("[%s] Failed to load model: %s", modelName.c_str(), modelPath);
		return false;
	}

	if (outModelData.meshes.empty())
	{
		LOG_ERROR("[%s] Model has no meshes", modelName.c_str());
		return false;
	}

	// 모든 메시를 하나로 병합 + 서브메시 정보 생성
	std::vector<Graphics::StandardVertex> allVertices;
	std::vector<Core::uint32> allIndices;
	std::vector<Graphics::SubmeshInfo> submeshes;

	Math::Vector3 aabbMin = { FLT_MAX, FLT_MAX, FLT_MAX };
	Math::Vector3 aabbMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

	Core::uint32 baseVertex = 0;
	Core::uint32 baseIndex = 0;

	for (size_t i = 0; i < outModelData.meshes.size(); ++i)
	{
		auto& meshData = outModelData.meshes[i];

		// 서브메시 정보 생성
		Graphics::SubmeshInfo submesh;
		submesh.startIndex = baseIndex;
		submesh.indexCount = static_cast<Core::uint32>(meshData.indices.size());
		submesh.baseVertex = baseVertex;
		submesh.materialIndex = static_cast<Core::uint32>(i);
		submeshes.push_back(submesh);

		// 버텍스 추가
		allVertices.insert(
			allVertices.end(),
			meshData.vertices.begin(),
			meshData.vertices.end()
		);

		// 인덱스 추가
		allIndices.insert(
			allIndices.end(),
			meshData.indices.begin(),
			meshData.indices.end()
		);

		// AABB 병합
		aabbMin.x = std::min(aabbMin.x, meshData.aabbMin.x);
		aabbMin.y = std::min(aabbMin.y, meshData.aabbMin.y);
		aabbMin.z = std::min(aabbMin.z, meshData.aabbMin.z);
		aabbMax.x = std::max(aabbMax.x, meshData.aabbMax.x);
		aabbMax.y = std::max(aabbMax.y, meshData.aabbMax.y);
		aabbMax.z = std::max(aabbMax.z, meshData.aabbMax.z);

		LOG_INFO(
			"[%s] Submesh[%zu]: startIdx=%u, idxCount=%u, baseVtx=%u, mat=%u",
			modelName.c_str(), i,
			submesh.startIndex, submesh.indexCount, submesh.baseVertex, submesh.materialIndex
		);

		baseVertex += static_cast<Core::uint32>(meshData.vertices.size());
		baseIndex += static_cast<Core::uint32>(meshData.indices.size());
	}

	// MeshAsset 생성 및 데이터 설정
	outMeshAsset = std::make_unique<Framework::MeshAsset>();
	outMeshAsset->SetVertices(std::move(allVertices));
	outMeshAsset->SetIndices(std::move(allIndices));
	outMeshAsset->SetSubmeshes(std::move(submeshes));
	outMeshAsset->SetAABB(aabbMin, aabbMax);
	outMeshAsset->SetDataPolicy(Framework::MeshDataPolicy::ReleaseAfterUpload);

	// ResourceManager를 통해 GPU Mesh 생성
	std::string meshResourceName = modelName + "Mesh";
	outMeshId = mResourceManager->CreateMeshFromAsset(meshResourceName, outMeshAsset.get());

	if (!outMeshId.IsValid())
	{
		LOG_ERROR("[%s] Failed to create GPU mesh from asset", modelName.c_str());
		return false;
	}

	LOG_INFO(
		"[%s] Merged %zu meshes: V=%u, I=%u, Submeshes=%zu",
		modelName.c_str(),
		outModelData.meshes.size(),
		outMeshAsset->GetVertexCount(),
		outMeshAsset->GetIndexCount(),
		outMeshAsset->GetSubmeshCount()
	);

	// 머티리얼 정보 로그
	for (size_t i = 0; i < outModelData.materials.size(); ++i)
	{
		const auto& mat = outModelData.materials[i];
		LOG_INFO("[%s] Material %zu: %s (%zu textures)",
			modelName.c_str(), i, mat.name.c_str(), mat.textures.size());

		for (const auto& tex : mat.textures)
		{
			LOG_INFO("  - %s: %s",
				Graphics::TextureTypeToString(tex.type),
				tex.path.c_str());
		}
	}

	return true;
}

void ModelViewerApp::CreateMaterialsFromModelData(
	Framework::LoadedModelData& modelData,
	const std::string& materialNamePrefix,
	std::vector<Framework::ResourceId>& outMaterialIds)
{
	outMaterialIds.clear();

	if (modelData.materials.empty())
	{
		LOG_WARN("[Material] No material data in model");
		return;
	}

	Framework::ResourceId fallbackTexId = mResourceManager->GetFallbackTexture();

	for (size_t matIndex = 0; matIndex < modelData.materials.size(); ++matIndex)
	{
		auto& matData = modelData.materials[matIndex];

		// 빈 머티리얼 건너뛰기
		if (matData.textures.empty())
		{
			LOG_DEBUG("[Material] Skipping empty material[%zu]", matIndex);
			outMaterialIds.push_back(Framework::ResourceId::Invalid());
			continue;
		}

		// Material 이름 생성
		std::string matName = materialNamePrefix + "_" + std::to_string(matIndex);

		Framework::ResourceId matId = mResourceManager->CreateMaterial(
			matName,
			L"../../Assets/Shaders/PhongVS.hlsl",
			L"../../Assets/Shaders/PhongPS.hlsl"
		);

		auto* material = mResourceManager->GetMaterial(matId);
		if (!material)
		{
			LOG_ERROR("[Material] Failed to create material: %s", matName.c_str());
			outMaterialIds.push_back(Framework::ResourceId::Invalid());
			continue;
		}

		LOG_INFO("[Material] Loading textures for material[%zu]: %s", matIndex, matData.name.c_str());

		// 텍스처 로드
		for (auto& texInfo : matData.textures)
		{
			Framework::ResourceId texId;

			if (texInfo.HasEmbeddedData())
			{
				// 임베디드 텍스처
				std::string texName = modelData.name + "_" + texInfo.path;

				if (texInfo.isCompressed)
				{
					texId = mResourceManager->LoadTextureFromMemory(
						texName,
						texInfo.embeddedData.data(),
						static_cast<Core::uint32>(texInfo.embeddedData.size()),
						texInfo.type
					);
				}
				else
				{
					texId = mResourceManager->CreateTextureFromMemory(
						texName,
						texInfo.embeddedData.data(),
						texInfo.width,
						texInfo.height,
						texInfo.type
					);
				}

				if (texId.IsValid())
				{
					material->SetTexture(texInfo.type, texId);
					texInfo.path = texName;
					LOG_INFO("[Material] Loaded embedded %s", texInfo.path.c_str());
				}
			}
			else if (!texInfo.isEmbedded)
			{
				// 외부 텍스처
				texId = mResourceManager->LoadTexture(texInfo.path, texInfo.type);
				if (texId.IsValid())
				{
					material->SetTexture(texInfo.type, texId);
					LOG_INFO(
						"[Material] Loaded %s: %s",
						Graphics::TextureTypeToString(texInfo.type),
						texInfo.path.c_str()
					);
				}
			}

			// 실패 시 폴백
			if (!texId.IsValid() && fallbackTexId.IsValid())
			{
				material->SetTexture(texInfo.type, fallbackTexId);
			}
		}

		// Descriptor 할당
		if (!material->AllocateDescriptors(
			GetDevice()->GetDevice(),
			GetRenderer()->GetSrvDescriptorHeap(),
			mResourceManager.get()
		))
		{
			LOG_ERROR("[Material] Failed to allocate descriptors for: %s", matName.c_str());
		}

		outMaterialIds.push_back(matId);
		LOG_INFO("[Material] Created material[%zu]: %s (ID: 0x%llX)", matIndex, matName.c_str(), matId.id);
	}

	LOG_INFO("[Material] Total %zu materials created", outMaterialIds.size());
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
	Math::Vector3 rotation = { 0.0f, mRotationAngle, 0.0f };

	if (transformSystem)
	{
		// 모든 렌더러블 Entity 회전
		transformSystem->SetRotationEuler(mProceduralSphereEntity, rotation);
		transformSystem->SetRotationEuler(mLoadedSphereEntity, rotation);
		transformSystem->SetRotationEuler(mHelmetEntity, rotation);
		//transformSystem->SetRotationEuler(mCubeEntity, rotation);
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

	// Entity 일괄 정리
	if (mRegistry)
	{
		ECS::Entity entitiesToDestroy[] = {
			mProceduralSphereEntity,
			mLoadedSphereEntity,
			mHelmetEntity,
			mCubeEntity,
			mDirectionalLightEntity,
			mCameraEntity
		};

		for (auto& entity : entitiesToDestroy)
		{
			if (entity.IsValid())
			{
				mRegistry->DestroyEntity(entity);
			}
		}
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
// Debug UI 메인
//=============================================================================

void ModelViewerApp::OnRenderDebugUI()
{
	// ECS Inspector
	if (Framework::ECSInspector* inspector = GetECSInspector())
	{
		inspector->Render(mRegistry.get());
	}

	// 각 패널 렌더링
	if (mShowAssetManagerPanel)
	{
		RenderAssetManagerPanel();
	}
	if (mShowSphereControlPanel)
	{
		RenderSphereControlPanel();
	}
	if (mShowModelInfoPanel)
	{
		RenderModelInfoPanel();
	}
}

//=============================================================================
// Asset Manager 패널
//=============================================================================

void ModelViewerApp::RenderAssetManagerPanel()
{
	using namespace Framework;

	ImGui::SetNextWindowSize(ImVec2(350, 300), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("Asset Manager (F4)", &mShowAssetManagerPanel))
	{
		ImGui::End();
		return;
	}

	if (!mAssetManager)
	{
		ImGui::TextColored(UIColor::Error, "AssetManager not initialized");
		ImGui::End();
		return;
	}

	// 요약 정보
	ImGui::Text("Asset Root: %s", mAssetManager->GetAssetRoot().c_str());
	ImGui::Text("Loaded Assets: %u", mAssetManager->GetLoadedAssetCount());
	ImGui::Separator();

	// 기본 Asset ID
	if (ImGuiBeginSection("Default Assets"))
	{
		ImGui::Text("Mesh:     0x%llX", mAssetManager->GetDefaultAssetId<MeshAsset>().id);
		ImGui::Text("Texture:  0x%llX", mAssetManager->GetDefaultAssetId<TextureAsset>().id);
		ImGui::Text("Material: 0x%llX", mAssetManager->GetDefaultAssetId<MaterialAsset>().id);
		ImGui::Text("Model:    0x%llX", mAssetManager->GetDefaultAssetId<ModelAsset>().id);
	}

	// 로드된 Asset 목록
	if (ImGuiBeginSection("Loaded Assets"))
	{
		auto infos = mAssetManager->GetLoadedAssetInfos(true);

		ImGui::BeginChild("AssetList", ImVec2(0, 120), true);
		for (const auto& info : infos)
		{
			const char* typeName = AssetTypeToString(info.type);
			const char* stateName = AssetStateToString(info.state);
			bool isDefault = mAssetManager->IsDefaultAsset(mAssetManager->FindByPath(info.path));

			// 기본 Asset은 파란색
			if (isDefault)
			{
				ImGui::TextColored(UIColor::Info, "[%s] %s", typeName, info.path.c_str());
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

	// 유틸리티 버튼
	ImGui::Separator();
	if (ImGui::Button("Unload Unused"))
	{
		Core::uint32 count = mAssetManager->UnloadUnusedAssets();
		LOG_INFO("[ModelViewer] Unloaded %u unused assets", count);
	}

	ImGui::End();
}

//=============================================================================
// Sphere Control 패널
//=============================================================================

void ModelViewerApp::RenderSphereControlPanel()
{
	using namespace Framework;

	ImGui::SetNextWindowSize(ImVec2(320, 280), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("Sphere Comparison (F5)", &mShowSphereControlPanel))
	{
		ImGui::End();
		return;
	}

	// 오브젝트 상태
	if (ImGuiBeginSection("Objects"))
	{
		ImGui::Text("Left:   Procedural Sphere");
		ImGuiTextStatus("Center", mLoadedMeshValid, "Loaded Sphere");
		ImGuiTextStatus("Right", mHelmetMeshValid, "DamagedHelmet");
		ImGuiTextStatus("Top", mCubeMeshValid, "MultiMaterialCube");
	}

	// 회전 속도
	if (ImGuiBeginSection("Transform"))
	{
		ImGui::SliderFloat("Rotation Speed", &mRotationSpeed, 0.0f, 3.0f);
	}

	// Procedural Mesh 파라미터
	if (ImGuiBeginSection("Procedural Mesh"))
	{
		int segments = static_cast<int>(mSphereSegments);
		int rings = static_cast<int>(mSphereRings);

		bool changed = false;
		changed |= ImGuiSliderIntClamped("Segments", &segments, 8, 64);
		changed |= ImGuiSliderIntClamped("Rings", &rings, 4, 32);

		if (changed)
		{
			mSphereSegments = static_cast<Core::uint32>(segments);
			mSphereRings = static_cast<Core::uint32>(rings);
			mNeedsMeshRebuild = true;
		}

		// 현재 메시 정보
		ImGui::Separator();
		Core::uint32 vertCount = (mSphereSegments + 1) * (mSphereRings + 1);
		Core::uint32 triCount = mSphereSegments * mSphereRings * 2;
		ImGui::TextDisabled("Vertices: %u, Triangles: %u", vertCount, triCount);
	}

	ImGui::End();
}

//=============================================================================
// Model Info 패널 - 헬퍼 함수
//=============================================================================

void ModelViewerApp::RenderModelInfoPanelSummary()
{
	ImGui::Text("Model: %s", mHelmetModelData.name.c_str());
	ImGui::Text(
		"Meshes: %zu | Materials: %zu | Nodes: %zu",
		mHelmetModelData.meshes.size(),
		mHelmetModelData.materials.size(),
		mHelmetModelData.nodes.size()
	);
	ImGui::Text(
		"Vertices: %u | Indices: %u",
		mHelmetModelData.GetTotalVertexCount(),
		mHelmetModelData.GetTotalIndexCount()
	);
	ImGui::Text(
		"Textures: %u (Embedded: %u)",
		mHelmetModelData.GetTotalTextureCount(),
		mHelmetModelData.GetEmbeddedTextureCount()
	);
}

void ModelViewerApp::RenderModelInfoPanelAssetPipeline()
{
	using namespace Framework;

	if (!ImGuiBeginSection("Asset Pipeline"))
	{
		return;
	}

	// 소스 데이터 상태
	bool hasSourceData = mHelmetMeshAsset && mHelmetMeshAsset->HasSourceData();
	ImGuiTextStatus("Source Data", hasSourceData, "Retained", "Released");

	if (!mHelmetMeshAsset)
	{
		return;
	}

	// 인덱스 포맷
	ImGui::Text(
		"Index Format: %s",
		mHelmetMeshAsset->CanUse16BitIndices() ? "16-bit" : "32-bit"
	);

	// Bounding 정보
	ImGuiTextVector3("AABB Min", mHelmetMeshAsset->GetAABBMin());
	ImGuiTextVector3("AABB Max", mHelmetMeshAsset->GetAABBMax());
	ImGui::Text("Bounding Sphere: R=%.2f", mHelmetMeshAsset->GetBoundingSphereRadius());
}

void ModelViewerApp::RenderModelInfoPanelMaterials()
{
	using namespace Framework;

	if (!ImGuiBeginSection("Materials"))
	{
		return;
	}

	for (size_t i = 0; i < mHelmetModelData.materials.size(); ++i)
	{
		const auto& mat = mHelmetModelData.materials[i];
		ImGui::PushID(static_cast<int>(i));

		if (ImGui::TreeNode("MaterialNode", "Material %zu: %s", i, mat.name.c_str()))
		{
			// PBR 파라미터
			ImGuiTextVector4("Base Color", mat.baseColorFactor);
			ImGui::Text("Metallic: %.2f | Roughness: %.2f", mat.metallicFactor, mat.roughnessFactor);
			ImGuiTextVector3("Emissive", mat.emissiveFactor);

			// 텍스처 목록
			if (!mat.textures.empty())
			{
				ImGui::Separator();
				ImGui::Text("Textures (%zu):", mat.textures.size());
				ImGui::Indent();
				for (const auto& tex : mat.textures)
				{
					TextTextureStatus(tex, mResourceManager.get());
				}
				ImGui::Unindent();
			}

			ImGui::TreePop();
		}

		ImGui::PopID();
	}
}

void ModelViewerApp::RenderModelInfoPanelMeshes()
{
	using namespace Framework;

	if (!ImGuiBeginSection("Meshes", false))  // 기본 접힘
	{
		return;
	}

	// 첫 번째 메시 (MeshAsset에서 정보 - 데이터가 move됨)
	if (mHelmetMeshAsset && !mHelmetModelData.meshes.empty())
	{
		ImGui::Text(
			"Mesh 0: %s (%u verts, %u indices)",
			mHelmetModelData.meshes[0].name.c_str(),
			mHelmetMeshAsset->GetVertexCount(),
			mHelmetMeshAsset->GetIndexCount()
		);
	}

	// 나머지 메시
	for (size_t i = 1; i < mHelmetModelData.meshes.size(); ++i)
	{
		const auto& mesh = mHelmetModelData.meshes[i];
		ImGui::Text(
			"Mesh %zu: %s (%zu verts, %zu indices)",
			i,
			mesh.name.c_str(),
			mesh.vertices.size(),
			mesh.indices.size()
		);
	}
}

void ModelViewerApp::RenderModelInfoPanelTextures()
{
	using namespace Framework;

	if (!ImGuiBeginSection("Loaded Textures", false))  // 기본 접힘
	{
		return;
	}

	if (!mResourceManager)
	{
		ImGui::TextColored(UIColor::Error, "ResourceManager not available");
		return;
	}

	// 모든 머티리얼의 텍스처 수집
	Core::uint32 textureCount = 0;
	for (const auto& mat : mHelmetModelData.materials)
	{
		for (const auto& tex : mat.textures)
		{
			ResourceId texId = mResourceManager->FindTextureByPath(tex.path);
			if (!texId.IsValid())
			{
				continue;
			}

			const TextureAsset* asset = mResourceManager->GetTextureAsset(texId);
			if (!asset)
			{
				continue;
			}

			++textureCount;

			const char* typeName = Graphics::TextureTypeToString(tex.type);
			const char* formatName = DXGIFormatToString(asset->GetFormat());

			// 테이블 형태로 표시
			if (textureCount == 1)
			{
				// 헤더
				ImGui::Columns(4, "TextureTable", true);
				ImGui::Separator();
				ImGui::Text("Type"); ImGui::NextColumn();
				ImGui::Text("Size"); ImGui::NextColumn();
				ImGui::Text("Format"); ImGui::NextColumn();
				ImGui::Text("Color Space"); ImGui::NextColumn();
				ImGui::Separator();
			}

			// 데이터 행
			ImGui::Text("%s", typeName); ImGui::NextColumn();
			ImGui::Text("%ux%u", asset->GetWidth(), asset->GetHeight()); ImGui::NextColumn();
			ImGui::Text("%s", formatName); ImGui::NextColumn();

			if (asset->IsSRGB())
			{
				ImGui::TextColored(UIColor::Highlight, "sRGB");
			}
			else
			{
				ImGui::Text("Linear");
			}
			ImGui::NextColumn();
		}
	}

	if (textureCount > 0)
	{
		ImGui::Columns(1);
		ImGui::Separator();
		ImGui::Text("Total: %u textures", textureCount);
	}
	else
	{
		ImGui::TextColored(UIColor::Disabled, "No textures loaded");
	}
}

//=============================================================================
// Model Info 패널 - 메인
//=============================================================================

void ModelViewerApp::RenderModelInfoPanel()
{
	using namespace Framework;

	ImGui::SetNextWindowSize(ImVec2(400, 450), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("Model Info (F6)", &mShowModelInfoPanel))
	{
		ImGui::End();
		return;
	}

	// 모델 로드 실패 시
	if (!mHelmetMeshValid)
	{
		ImGui::TextColored(UIColor::Error, "DamagedHelmet not loaded");
		ImGui::TextColored(UIColor::Disabled, "Place DamagedHelmet.glb in Assets/Models/");
		ImGui::End();
		return;
	}

	// 각 섹션 렌더링
	RenderModelInfoPanelSummary();
	ImGui::Separator();
	RenderModelInfoPanelAssetPipeline();
	RenderModelInfoPanelMaterials();
	RenderModelInfoPanelTextures();
	RenderModelInfoPanelMeshes();

	ImGui::End();
}
