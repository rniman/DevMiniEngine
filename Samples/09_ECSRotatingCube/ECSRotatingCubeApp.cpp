#include "ECSRotatingCubeApp.h"

// Framework
#include "Framework/Resources/ResourceId.h"
#include "Framework/Resources/ResourceManager.h"

// Core Engine
#include "Core/Logging/LogMacros.h"

// Math
#include "Math/MathTypes.h"
#include "Math/MathUtils.h"

// Platform
#include "Platform/Window.h"

// Graphics
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"
#include "Graphics/RenderTypes.h"
#include "Graphics/TextureType.h"

// Graphics - DX12
#include "Graphics/DX12/DX12Device.h"
#include "Graphics/DX12/DX12Renderer.h"

// ECS
#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/MaterialComponent.h"
#include "ECS/Components/MeshComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/SystemManager.h"
#include "ECS/Registry.h"
#include "ECS/RegistryView.h"
#include "ECS/Systems/CameraSystem.h"
#include "ECS/Systems/RenderSystem.h"
#include "ECS/Systems/TransformSystem.h"

// Standard Library
#include <memory>

ECSRotatingCubeApp::ECSRotatingCubeApp()
	: Application(GetAppDesc())
{
}

ECSRotatingCubeApp::~ECSRotatingCubeApp()
{
	OnShutdown();
}

bool ECSRotatingCubeApp::OnInitialize()
{
	LOG_INFO("[ECSRotatingCube] Initializing...");

	// 리소스 매니저 생성
	mResourceManager = std::make_unique<Framework::ResourceManager>(
		GetDevice(),
		GetRenderer()
	);

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

	LOG_INFO("[ECSRotatingCube] Initialization complete");

	return true;
}

void ECSRotatingCubeApp::InitializeECS()
{
	LOG_INFO("[ECS] Initializing Registry...");

	// Registry 생성
	mRegistry = std::make_unique<ECS::Registry>();

	// SystemManager 생성
	mSystemManager = std::make_unique<ECS::SystemManager>(mRegistry.get());

	// System 등록
	mSystemManager->RegisterSystem<ECS::RenderSystem>(mResourceManager.get());

	// Camera Entity 생성
	CreateCameraEntity();

	// Cube Entity 생성
	CreateCubeEntity();

	LOG_INFO("[ECS] Registry initialized");
}

void ECSRotatingCubeApp::CreateCameraEntity()
{
	LOG_INFO("[ECS] Creating Camera Entity...");

	// 1. Entity 생성
	mCameraEntity = mRegistry->CreateEntity();
	LOG_DEBUG(
		"[ECS] Created Camera Entity (ID: %u, Version: %u)",
		mCameraEntity.id, mCameraEntity.version
	);

	// 2. Transform Component 추가
	ECS::TransformComponent transform;
	// LookAt 설정 (원점을 바라봄)
	ECS::CameraSystem::SetLookAt(
		transform,
		Math::Vector3(0.0f, 10.0f, -20.0f),  // position
		Math::Vector3(0.0f, 0.0f, 0.0f),      // target
		Math::Vector3(0.0f, 1.0f, 0.0f)       // up
	);
	mRegistry->AddComponent(mCameraEntity, transform);
	LOG_DEBUG("[ECS] Added TransformComponent to Camera");

	// 3. Camera Component 추가
	ECS::CameraComponent camera;
	camera.projectionType = ECS::ProjectionType::Perspective;
	ECS::CameraSystem::SetFovYDegrees(camera, 60.0f);
	ECS::CameraSystem::SetAspectRatio(
		camera,
		static_cast<Core::float32>(GetWindow()->GetWidth()),
		static_cast<Core::float32>(GetWindow()->GetHeight())
	);
	ECS::CameraSystem::SetClipPlanes(camera, 0.1f, 1000.0f);
	camera.isMainCamera = true;  // Main Camera로 설정

	mRegistry->AddComponent(mCameraEntity, camera);
	LOG_DEBUG("[ECS] Added CameraComponent (Main Camera)");

	LOG_INFO("[ECS] Camera Entity created successfully");
}

void ECSRotatingCubeApp::CreateCubeEntity()
{
	LOG_INFO("[ECS] Creating Cube Entity...");

	// 1. Entity 생성
	mCubeEntity = mRegistry->CreateEntity();
	LOG_DEBUG(
		"[ECS] Created Entity (ID: %u, Version: %u)",
		mCubeEntity.id, mCubeEntity.version
	);

	// 2. Transform Component 추가
	ECS::TransformComponent transform;
	transform.position = { 0.0f, 0.0f, 0.0f };
	transform.scale = { 1.0f, 1.0f, 1.0f };
	ECS::TransformSystem::SetRotationEuler(transform,
		Math::DegToRad(0.0f),
		Math::DegToRad(45.0f),
		Math::DegToRad(0.0f)
	);
	mRegistry->AddComponent(mCubeEntity, transform);
	LOG_DEBUG("[ECS] Added TransformComponent");

	// 3. Mesh Component 추가
	ECS::MeshComponent meshComp;
	meshComp.meshId = mResourceManager->CreateMesh("Cube");
	mRegistry->AddComponent(mCubeEntity, meshComp);
	LOG_DEBUG("[ECS] Added MeshComponent (ID: 0x%llX)", meshComp.meshId.id);

	// 4. Material Component 추가
	ECS::MaterialComponent matComp;
	matComp.materialId = mResourceManager->CreateMaterial(
		"BasicMaterial",
		L"TexturedShader.hlsl",
		L"TexturedShader.hlsl"
	);
	mRegistry->AddComponent(mCubeEntity, matComp);
	LOG_DEBUG("[ECS] Added MaterialComponent (ID: 0x%llX)", matComp.materialId.id);

	// 5. Mesh 데이터 설정
	SetupMeshData();

	// 6. Material 설정 (텍스처 로드 및 Descriptor 할당)
	SetupMaterial();

	LOG_INFO("[ECS] Cube Entity created successfully");
}

void ECSRotatingCubeApp::SetupMeshData()
{
	// Mesh Component에서 ResourceId 가져오기
	auto* meshComp = mRegistry->GetComponent<ECS::MeshComponent>(mCubeEntity);
	if (!meshComp)
	{
		LOG_ERROR("[ECS] MeshComponent not found!");
		return;
	}

	// ResourceManager에서 실제 Mesh 포인터 조회
	auto* mesh = mResourceManager->GetMesh(meshComp->meshId);
	if (!mesh)
	{
		LOG_ERROR("[ECS] Mesh not found for ID: 0x%llX", meshComp->meshId.id);
		return;
	}

	// 큐브 정점 데이터 (위치 + 색상)
	Graphics::TexturedVertex vertices[] =
	{
		// 1. Front face (앞면)
		{ Math::Vector3(-2.0f,  2.0f, -2.0f), Math::Vector2(0.0f, 0.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 0
		{ Math::Vector3(2.0f,  2.0f, -2.0f), Math::Vector2(1.0f, 0.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 1
		{ Math::Vector3(-2.0f, -2.0f, -2.0f), Math::Vector2(0.0f, 1.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 2
		{ Math::Vector3(2.0f, -2.0f, -2.0f), Math::Vector2(1.0f, 1.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 3

		// 2. Back face (뒷면)
		{ Math::Vector3(2.0f,  2.0f,  2.0f), Math::Vector2(0.0f, 0.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 4
		{ Math::Vector3(-2.0f,  2.0f,  2.0f), Math::Vector2(1.0f, 0.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 5
		{ Math::Vector3(2.0f, -2.0f,  2.0f), Math::Vector2(0.0f, 1.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 6
		{ Math::Vector3(-2.0f, -2.0f,  2.0f), Math::Vector2(1.0f, 1.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 7

		// 3. Top face (윗면)
		{ Math::Vector3(-2.0f,  2.0f,  2.0f), Math::Vector2(0.0f, 0.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 8
		{ Math::Vector3(2.0f,  2.0f,  2.0f), Math::Vector2(1.0f, 0.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 9
		{ Math::Vector3(-2.0f,  2.0f, -2.0f), Math::Vector2(0.0f, 1.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 10
		{ Math::Vector3(2.0f,  2.0f, -2.0f), Math::Vector2(1.0f, 1.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 11

		// 4. Bottom face (아랫면)
		{ Math::Vector3(2.0f, -2.0f,  2.0f), Math::Vector2(0.0f, 0.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 12
		{ Math::Vector3(-2.0f, -2.0f,  2.0f), Math::Vector2(1.0f, 0.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 13
		{ Math::Vector3(2.0f, -2.0f, -2.0f), Math::Vector2(0.0f, 1.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 14
		{ Math::Vector3(-2.0f, -2.0f, -2.0f), Math::Vector2(1.0f, 1.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 15

		// 5. Left face (왼쪽면)
		{ Math::Vector3(-2.0f,  2.0f,  2.0f), Math::Vector2(0.0f, 0.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 16
		{ Math::Vector3(-2.0f,  2.0f, -2.0f), Math::Vector2(1.0f, 0.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 17
		{ Math::Vector3(-2.0f, -2.0f,  2.0f), Math::Vector2(0.0f, 1.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 18
		{ Math::Vector3(-2.0f, -2.0f, -2.0f), Math::Vector2(1.0f, 1.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 19

		// 6. Right face (오른쪽면)
		{ Math::Vector3(2.0f,  2.0f, -2.0f), Math::Vector2(0.0f, 0.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 20
		{ Math::Vector3(2.0f,  2.0f,  2.0f), Math::Vector2(1.0f, 0.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 21
		{ Math::Vector3(2.0f, -2.0f, -2.0f), Math::Vector2(0.0f, 1.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 22
		{ Math::Vector3(2.0f, -2.0f,  2.0f), Math::Vector2(1.0f, 1.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }  // 23
	};

	Core::uint16 indices[] = {
		// 1. Front face
		0, 1, 2,
		1, 3, 2,

		// 2. Back face
		4, 5, 6,
		5, 7, 6,

		// 3. Top face
		8, 9, 10,
		9, 11, 10,

		// 4. Bottom face
		12, 13, 14,
		13, 15, 14,

		// 5. Left face
		16, 17, 18,
		17, 19, 18,

		// 6. Right face
		20, 21, 22,
		21, 23, 22
	};

	// Mesh에 데이터 설정
	bool HasMesh = mesh->InitializeTextured(
		GetDevice()->GetDevice(),
		GetDevice()->GetGraphicsQueue(),
		GetDevice()->GetCommandContext(GetRenderer()->GetCurrentFrameIndex()),
		vertices,
		24,
		indices,
		36
	);

	if (!HasMesh)
	{
		LOG_INFO("[Mesh] Fail Cube mesh data set");
	}

	LOG_INFO("[Mesh] Cube mesh data set");
}

void ECSRotatingCubeApp::SetupMaterial()
{
	// Material Component에서 ResourceId 가져오기
	auto* matComp = mRegistry->GetComponent<ECS::MaterialComponent>(mCubeEntity);
	if (!matComp)
	{
		LOG_ERROR("[ECS] MaterialComponent not found!");
		return;
	}

	// ResourceManager에서 실제 Material 포인터 조회
	auto* material = mResourceManager->GetMaterial(matComp->materialId);
	if (!material)
	{
		LOG_ERROR("[ECS] Material not found for ID: 0x%llX", matComp->materialId.id);
		return;
	}

	// Texture 로드 및 Material에 설정
	Framework::ResourceId diffuseId = mResourceManager->LoadTexture(
		"../../Assets/Textures/BrickWall17_1K_BaseColor.png"
	);
	Framework::ResourceId normalId = mResourceManager->LoadTexture(
		"../../Assets/Textures/BrickWall17_1K_Normal.png"
	);

	if (diffuseId.IsValid())
	{
		material->SetTexture(Graphics::TextureType::Albedo, diffuseId);
		LOG_DEBUG("[Material] Set Albedo texture: 0x%llX", diffuseId.id);
	}

	if (normalId.IsValid())
	{
		material->SetTexture(Graphics::TextureType::Normal, normalId);
		LOG_DEBUG("[Material] Set Normal texture: 0x%llX", normalId.id);
	}

	// 추가 텍스처 (선택적)
	// Framework::ResourceId normalId = mResourceManager->LoadTexture(L"...");
	// material->SetTexture(Graphics::TextureType::Normal, normalId);

	// Descriptor 할당 (GPU에 텍스처 바인딩 준비)
	if (!material->AllocateDescriptors(
		GetDevice()->GetDevice(),
		GetRenderer()->GetSrvDescriptorHeap(),
		mResourceManager.get()
	))
	{
		LOG_ERROR("[Material] Failed to allocate descriptors");
		return;
	}

	LOG_DEBUG("[Material] Material setup complete");
}

void ECSRotatingCubeApp::OnUpdate(Core::float32 deltaTime)
{
	// 1. 카메라 업데이트 (View/Projection 행렬)
	ECS::CameraSystem::UpdateAllCameras(*mRegistry);

	// 2. 큐브 회전 업데이트
	auto* transform = mRegistry->GetComponent<ECS::TransformComponent>(mCubeEntity);
	if (transform)
	{
		// Y축 기준 회전 (TransformSystem 사용)
		Math::Vector3 yAxisRotation = {
			0.0f,
			Math::DegToRad(mRotationSpeed) * deltaTime,
			0.0f
		};
		ECS::TransformSystem::Rotate(*transform, yAxisRotation);
	}

	// 3. SystemManager 업데이트 (RenderSystem 등)
	mSystemManager->UpdateSystems(deltaTime);
}

void ECSRotatingCubeApp::OnRender()
{
	auto* renderSystem = mSystemManager->GetSystem<ECS::RenderSystem>();
	if (renderSystem)
	{
		const Graphics::FrameData& frameData = renderSystem->GetFrameData();

		// 렌더러에 제출
		GetRenderer()->RenderFrame(frameData);
	}
}

void ECSRotatingCubeApp::OnShutdown()
{
	LOG_INFO("[ECSRotatingCube] Shutting down...");

	// Entity 정리
	if (mCameraEntity.IsValid() && mRegistry)
	{
		mRegistry->DestroyEntity(mCameraEntity);
	}

	if (mCubeEntity.IsValid() && mRegistry)
	{
		mRegistry->DestroyEntity(mCubeEntity);
	}

	// 시스템 정리
	mSystemManager.reset();
	mRegistry.reset();
	mResourceManager.reset();

	LOG_INFO("[ECSRotatingCube] Shutdown complete");
}
