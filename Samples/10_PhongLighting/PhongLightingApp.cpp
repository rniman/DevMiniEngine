#include "PhongLightingApp.h"

// Framework
#include "Framework/Resources/ResourceId.h"
#include "Framework/Resources/ResourceManager.h"
#include "Framework/DebugUI/ECSInspector.h"

// Core Engine
#include "Core/Logging/LogMacros.h"

// Math
#include "Math/MathTypes.h"
#include "Math/MathUtils.h"
#include "Math/MeshUtils.h"

// Platform
#include "Platform/Window.h"

// Graphics
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"
#include "Graphics/RenderTypes.h"
#include "Graphics/TextureType.h"

// Graphics - DX12
#include "Graphics/DX12/DX12Device.h"
#include "Graphics/DX12/DX12CommandQueue.h"
#include "Graphics/DX12/DX12Renderer.h"

// ECS
#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/HierarchyComponent.h"
#include "ECS/Components/LightComponents.h"
#include "ECS/Components/MaterialComponent.h"
#include "ECS/Components/MeshComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/SystemManager.h"
#include "ECS/Registry.h"
#include "ECS/RegistryView.h"
#include "ECS/Systems/CameraSystem.h"
#include "ECS/Systems/LightingSystem.h"
#include "ECS/Systems/RenderSystem.h"
#include "ECS/Systems/TransformSystem.h"

// Standard Library
#include <memory>

PhongLightingApp::PhongLightingApp()
	: Application(GetAppDesc())
{
}

PhongLightingApp::~PhongLightingApp()
{
}

bool PhongLightingApp::OnInitialize()
{
	LOG_INFO("=== Phase 3.3 + 3.5: Phong Lighting + Hierarchy Demo ===");

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

	LOG_INFO("[PhongLighting] Initialization complete");

	return true;
}

void PhongLightingApp::InitializeECS()
{
	LOG_INFO("[ECS] Initializing Registry...");

	// Registry 생성
	mRegistry = std::make_unique<ECS::Registry>();

	// SystemManager 생성 (참조로 전달)
	mSystemManager = std::make_unique<ECS::SystemManager>(*mRegistry);

	// System 등록 (순서 중요!)
	// Transform → Camera → Lighting → Render
	// Registry는 SystemManager가 자동으로 전달
	mSystemManager->RegisterSystem<ECS::TransformSystem>();
	mSystemManager->RegisterSystem<ECS::CameraSystem>();
	mSystemManager->RegisterSystem<ECS::LightingSystem>();
	mSystemManager->RegisterSystem<ECS::RenderSystem>(mResourceManager.get());

	// Scene 구성
	CreateCameraEntity();
	CreateLightEntities();
	CreateCubeEntities();

	// Phase 3.5: 계층 구조 테스트 Entity 생성
	CreateHierarchyTestEntities();

	LOG_INFO("[ECS] Registry initialized");
}

void PhongLightingApp::CreateCameraEntity()
{
	LOG_INFO("[Scene] Creating Camera Entity...");

	// 1. Entity 생성
	mCameraEntity = mRegistry->CreateEntity();
	LOG_DEBUG(
		"[ECS] Created Camera Entity (ID: %u, Version: %u)",
		mCameraEntity.id, mCameraEntity.version
	);

	ECS::TransformComponent transform;
	ECS::CameraComponent camera;

	camera.projectionType = ECS::ProjectionType::Perspective;

	// 저수준 API (정적 함수) - Component 직접 조작
	ECS::CameraSystem::SetFovYDegrees(camera, 60.0f);
	ECS::CameraSystem::SetAspectRatio(
		camera,
		static_cast<Core::float32>(GetWindow()->GetWidth()),
		static_cast<Core::float32>(GetWindow()->GetHeight())
	);
	ECS::CameraSystem::SetClipPlanes(camera, 0.1f, 1000.0f);
	camera.isMainCamera = true;

	// SetLookAt 호출 (저수준 API - Transform과 Camera 모두 전달)
	ECS::CameraSystem::SetLookAt(
		transform,
		camera,
		Math::Vector3(0.0f, 3.0f, -3.0f),
		Math::Vector3(0.0f, 0.0f, 0.0f),
		Math::Vector3(0.0f, 1.0f, 0.0f)
	);

	// Component 추가
	mRegistry->AddComponent(mCameraEntity, transform);
	mRegistry->AddComponent(mCameraEntity, camera);

	LOG_INFO("[Scene] Camera created (Main Camera)");
}

void PhongLightingApp::CreateLightEntities()
{
	LOG_INFO("[Scene] Creating Light Entities...");

	// 1. Directional Light (태양광)
	mDirectionalLightEntity = mRegistry->CreateEntity();

	ECS::DirectionalLightComponent dirLight;
	dirLight.direction = Math::Normalize(Math::Vector3(0.3f, -1.0f, 0.5f));
	dirLight.color = Math::Vector3(1.0f, 0.95f, 0.9f);  // 따뜻한 햇빛
	dirLight.intensity = 0.8f;
	dirLight.castsShadow = false;

	mRegistry->AddComponent(mDirectionalLightEntity, dirLight);
	LOG_INFO("[Scene] Directional Light created");

	// 2. Point Lights (4개, 각기 다른 색상)
	struct PointLightSetup
	{
		Math::Vector3 position;
		Math::Vector3 color;
		Core::float32 intensity;
	};

	PointLightSetup pointLightSetups[] = {
		{ Math::Vector3(-10.0f, 5.0f, 0.0f),  Math::Vector3(1.0f, 0.0f, 0.0f), 5.0f },  // 빨간색
		{ Math::Vector3(10.0f, 5.0f, 0.0f),   Math::Vector3(0.0f, 1.0f, 0.0f), 5.0f },  // 녹색
		{ Math::Vector3(0.0f, 5.0f, -10.0f),  Math::Vector3(0.0f, 0.0f, 1.0f), 5.0f },  // 파란색
		{ Math::Vector3(0.0f, 5.0f, 10.0f),   Math::Vector3(1.0f, 1.0f, 0.0f), 5.0f },  // 노란색
	};

	for (const auto& setup : pointLightSetups)
	{
		ECS::Entity lightEntity = mRegistry->CreateEntity();

		// Transform (위치)
		ECS::TransformComponent transform;
		transform.position = setup.position;
		mRegistry->AddComponent(lightEntity, transform);

		// Point Light
		ECS::PointLightComponent pointLight;
		pointLight.color = setup.color;
		pointLight.intensity = setup.intensity;
		pointLight.range = 5.0f;
		pointLight.constant = 1.0f;
		pointLight.linear = 0.045f;
		pointLight.quadratic = 0.0075f;

		mRegistry->AddComponent(lightEntity, pointLight);

		mPointLightEntities.push_back(lightEntity);
	}

	LOG_INFO("[Scene] Created %zu Point Lights", mPointLightEntities.size());
}

void PhongLightingApp::CreateCubeEntities()
{
	LOG_INFO("[Scene] Creating Cube Entities...");

	// 공유 리소스 생성
	mSharedMeshId = mResourceManager->CreateMesh("CubeMesh");
	mSharedMaterialId = mResourceManager->CreateMaterial(
		"PhongMaterial",
		L"PhongVS.hlsl",
		L"PhongPS.hlsl"
	);

	// 그리드 형태로 큐브 배치
	constexpr Core::int32 gridSize = 4;
	constexpr Core::float32 spacing = 10.0f;

	for (Core::int32 x = -gridSize / 2; x <= gridSize / 2; ++x)
	{
		for (Core::int32 z = -gridSize / 2; z <= gridSize / 2; ++z)
		{
			// 1. Entity 생성
			ECS::Entity cubeEntity = mRegistry->CreateEntity();

			// 2. Transform Component
			ECS::TransformComponent transform;
			transform.position = Math::Vector3(
				static_cast<Core::float32>(x) * spacing,
				0.0f,
				static_cast<Core::float32>(z) * spacing
			);
			transform.scale = Math::Vector3(0.8f, 0.8f, 0.8f);

			// 저수준 API (정적 함수)
			ECS::TransformSystem::SetRotationEuler(transform, 0.0f, 0.0f, 0.0f);

			mRegistry->AddComponent(cubeEntity, transform);

			// 3. Mesh Component (공유 메시)
			ECS::MeshComponent meshComp;
			meshComp.meshId = mSharedMeshId;
			mRegistry->AddComponent(cubeEntity, meshComp);

			// 4. Material Component (공유 머티리얼)
			ECS::MaterialComponent matComp;
			matComp.materialId = mSharedMaterialId;
			mRegistry->AddComponent(cubeEntity, matComp);

			mCubeEntities.push_back(cubeEntity);
		}
	}

	// Mesh와 Material 설정 (한 번만)
	SetupSharedMeshData();
	SetupSharedMaterial();

	LOG_INFO(
		"[Scene] Created %zu Cubes in %dx%d grid",
		mCubeEntities.size(),
		gridSize + 1,
		gridSize + 1
	);
}

void PhongLightingApp::SetupSharedMeshData()
{
	auto* mesh = mResourceManager->GetMesh(mSharedMeshId);
	if (!mesh)
	{
		LOG_ERROR("[Mesh] Shared mesh not found!");
		return;
	}

	// 1. 기본 데이터만 별도 배열로 준비
	std::vector<Math::Vector3> positions = {
		// Front face
		{ -1.0f, -1.0f, -1.0f }, { -1.0f,  1.0f, -1.0f }, {  1.0f,  1.0f, -1.0f }, {  1.0f, -1.0f, -1.0f },
		// Back face
		{  1.0f, -1.0f,  1.0f }, {  1.0f,  1.0f,  1.0f }, { -1.0f,  1.0f,  1.0f }, { -1.0f, -1.0f,  1.0f },
		// Left face
		{ -1.0f, -1.0f,  1.0f }, { -1.0f,  1.0f,  1.0f }, { -1.0f,  1.0f, -1.0f }, { -1.0f, -1.0f, -1.0f },
		// Right face
		{  1.0f, -1.0f, -1.0f }, {  1.0f,  1.0f, -1.0f }, {  1.0f,  1.0f,  1.0f }, {  1.0f, -1.0f,  1.0f },
		// Top face
		{ -1.0f,  1.0f, -1.0f }, { -1.0f,  1.0f,  1.0f }, {  1.0f,  1.0f,  1.0f }, {  1.0f,  1.0f, -1.0f },
		// Bottom face
		{ -1.0f, -1.0f,  1.0f }, { -1.0f, -1.0f, -1.0f }, {  1.0f, -1.0f, -1.0f }, {  1.0f, -1.0f,  1.0f },
	};

	std::vector<Math::Vector3> normals = {
		// Front
		{ 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f },
		// Back
		{ 0.0f, 0.0f,  1.0f }, { 0.0f, 0.0f,  1.0f }, { 0.0f, 0.0f,  1.0f }, { 0.0f, 0.0f,  1.0f },
		// Left
		{ -1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f },
		// Right
		{ 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f },
		// Top
		{ 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
		// Bottom
		{ 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f },
	};

	std::vector<Math::Vector2> texCoords = {
		// Front
		{ 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f },
		// Back
		{ 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f },
		// Left
		{ 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f },
		// Right
		{ 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f },
		// Top
		{ 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f },
		// Bottom
		{ 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f },
	};

	std::vector<Core::uint16> indices = {
		0,  1,  2,  0,  2,  3,   // Front
		4,  5,  6,  4,  6,  7,   // Back
		8,  9,  10, 8,  10, 11,  // Left
		12, 13, 14, 12, 14, 15,  // Right
		16, 17, 18, 16, 18, 19,  // Top
		20, 21, 22, 20, 22, 23   // Bottom
	};

	// 2. Tangent 자동 계산
	std::vector<Math::Vector3> tangents;
	Math::CalculateTangents(
		positions,
		normals,
		texCoords,
		indices,
		tangents
	);

	// 3. StandardVertex 배열 생성
	std::vector<Graphics::StandardVertex> vertices;
	vertices.reserve(positions.size());

	for (size_t i = 0; i < positions.size(); ++i)
	{
		Graphics::StandardVertex vertex;
		vertex.position = positions[i];
		vertex.normal = normals[i];
		vertex.texCoord = texCoords[i];
		vertex.tangent = tangents[i];
		vertices.push_back(vertex);
	}

	// 4. Mesh 초기화
	bool success = mesh->InitializeStandard(
		GetDevice()->GetDevice(),
		GetDevice()->GetCommandQueue(),
		GetDevice()->GetCommandContext(GetRenderer()->GetCurrentFrameIndex()),
		vertices.data(),
		static_cast<Core::uint32>(vertices.size()),
		indices.data(),
		static_cast<Core::uint32>(indices.size())
	);

	if (!success)
	{
		LOG_ERROR("[Mesh] Failed to initialize cube mesh");
		return;
	}

	LOG_INFO("[Mesh] Cube mesh initialized (24 vertices, 36 indices)");
}

void PhongLightingApp::SetupSharedMaterial()
{
	auto* material = mResourceManager->GetMaterial(mSharedMaterialId);
	if (!material)
	{
		LOG_ERROR("[Material] Shared material not found!");
		return;
	}

	Framework::ResourceId diffuseId = mResourceManager->LoadTexture(
		"../../Assets/Textures/BrickWall17_1K_BaseColor.png"
	);

	if (diffuseId.IsValid())
	{
		material->SetTexture(Graphics::TextureType::Albedo, diffuseId);
		LOG_DEBUG("[Material] Set Albedo texture: 0x%llX", diffuseId.id);
	}

	// Normal Map 로드 (선택적)
	Framework::ResourceId normalId = mResourceManager->LoadTexture(
		"../../Assets/Textures/BrickWall17_1K_Normal.png"
	);

	if (normalId.IsValid())
	{
		material->SetTexture(Graphics::TextureType::Normal, normalId);
		LOG_DEBUG("[Material] Set Normal texture: 0x%llX", normalId.id);
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

	LOG_INFO("[Material] Material setup complete");
}

void PhongLightingApp::CreateHierarchyTestEntities()
{
	LOG_INFO("[Scene] Creating Hierarchy Test Entities (Phase 3.5)...");

	auto* transformSystem = mSystemManager->GetSystem<ECS::TransformSystem>();

	//=========================================================================
	// 부모 큐브 생성 (그리드 옆에 배치, 제자리 회전)
	//=========================================================================
	mHierarchyParent = mRegistry->CreateEntity();
	{
		ECS::TransformComponent transform;
		transform.position = Math::Vector3(30.0f, 0.0f, 0.0f);  // 그리드 오른쪽
		transform.scale = Math::Vector3(1.5f);
		mRegistry->AddComponent<ECS::TransformComponent>(mHierarchyParent, transform);

		// 계층 구조 참여
		ECS::HierarchyComponent hierarchy;
		mRegistry->AddComponent<ECS::HierarchyComponent>(mHierarchyParent, hierarchy);

		ECS::MeshComponent mesh;
		mesh.meshId = mSharedMeshId;
		mRegistry->AddComponent<ECS::MeshComponent>(mHierarchyParent, mesh);

		ECS::MaterialComponent material;
		material.materialId = mSharedMaterialId;
		mRegistry->AddComponent<ECS::MaterialComponent>(mHierarchyParent, material);

		// Root Entity로 등록
		transformSystem->SetParent(mHierarchyParent, ECS::Entity::Invalid());
	}

	//=========================================================================
	// 자식 큐브들 생성 (부모 주위 3개, 120도 간격)
	//=========================================================================
	constexpr Core::int32 childCount = 3;
	constexpr Core::float32 childRadius = 4.0f;

	for (Core::int32 i = 0; i < childCount; ++i)
	{
		ECS::Entity child = mRegistry->CreateEntity();

		// 부모 기준 로컬 위치 (120도 간격 원형 배치)
		Core::float32 angle = (2.0f * Math::PI / childCount) * i;
		Math::Vector3 localPos(
			std::cos(angle) * childRadius,
			0.0f,
			std::sin(angle) * childRadius
		);

		ECS::TransformComponent transform;
		transform.position = localPos;
		transform.scale = Math::Vector3(0.7f);
		mRegistry->AddComponent<ECS::TransformComponent>(child, transform);

		ECS::HierarchyComponent hierarchy;
		mRegistry->AddComponent<ECS::HierarchyComponent>(child, hierarchy);

		ECS::MeshComponent mesh;
		mesh.meshId = mSharedMeshId;
		mRegistry->AddComponent<ECS::MeshComponent>(child, mesh);

		ECS::MaterialComponent material;
		material.materialId = mSharedMaterialId;
		mRegistry->AddComponent<ECS::MaterialComponent>(child, material);

		// 부모 설정
		transformSystem->SetParent(child, mHierarchyParent);

		mHierarchyChildren.push_back(child);
	}

	//=========================================================================
	// 손자 큐브 생성 (첫 번째 자식의 자식, 2단계 계층 테스트)
	//=========================================================================
	if (!mHierarchyChildren.empty())
	{
		mHierarchyGrandChild = mRegistry->CreateEntity();

		ECS::TransformComponent transform;
		transform.position = Math::Vector3(2.5f, 0.0f, 0.0f);  // 자식 기준 로컬 위치
		transform.scale = Math::Vector3(0.4f);
		mRegistry->AddComponent<ECS::TransformComponent>(mHierarchyGrandChild, transform);

		ECS::HierarchyComponent hierarchy;
		mRegistry->AddComponent<ECS::HierarchyComponent>(mHierarchyGrandChild, hierarchy);

		ECS::MeshComponent mesh;
		mesh.meshId = mSharedMeshId;
		mRegistry->AddComponent<ECS::MeshComponent>(mHierarchyGrandChild, mesh);

		ECS::MaterialComponent material;
		material.materialId = mSharedMaterialId;
		mRegistry->AddComponent<ECS::MaterialComponent>(mHierarchyGrandChild, material);

		// 첫 번째 자식의 자식으로 설정 (2단계 계층)
		transformSystem->SetParent(mHierarchyGrandChild, mHierarchyChildren[0]);
	}

	LOG_INFO("[Scene] Hierarchy structure created:");
	LOG_INFO("  Parent (id=%u) at (30, 0, 0)", mHierarchyParent.id);
	for (size_t i = 0; i < mHierarchyChildren.size(); ++i)
	{
		LOG_INFO("    +-- Child%zu (id=%u)", i, mHierarchyChildren[i].id);
	}
	if (mHierarchyGrandChild.IsValid())
	{
		LOG_INFO("    |     +-- GrandChild (id=%u)", mHierarchyGrandChild.id);
	}
}

void PhongLightingApp::OnUpdate(Core::float32 deltaTime)
{
	// System 인스턴스 가져오기 (고수준 API 사용을 위해)
	auto* cameraSystem = mSystemManager->GetSystem<ECS::CameraSystem>();
	auto* transformSystem = mSystemManager->GetSystem<ECS::TransformSystem>();

	// [테스트] 카메라를 원형으로 회전
	static Core::float32 cameraAngle = 0.0f;
	cameraAngle += deltaTime * 0.3f;

	if (cameraSystem)
	{
		Core::float32 radius = 20.0f;
		Core::float32 height = 8.0f;

		Math::Vector3 cameraPos = Math::Vector3(
			std::cos(cameraAngle) * radius,
			height,
			std::sin(cameraAngle) * radius
		);

		// 고수준 API (Entity 기반) - System 인스턴스 통해 호출
		cameraSystem->SetLookAt(
			mCameraEntity,
			cameraPos,
			Math::Vector3(0.0f, 0.0f, 0.0f),
			Math::Vector3(0.0f, 1.0f, 0.0f)
		);
	}

	// 큐브 회전 애니메이션
	mRotationAngle += deltaTime * 0.5f;

	if (transformSystem)
	{
		for (auto cubeEntity : mCubeEntities)
		{
			// 고수준 API (Entity 기반)
			transformSystem->SetRotationEuler(
				cubeEntity,
				Math::Vector3(
					mRotationAngle,
					mRotationAngle * 0.7f,
					mRotationAngle * 0.5f
				)
			);
		}

		//=====================================================================
		// Phase 3.5: 계층 구조 애니메이션
		// 부모만 회전하면 자식들이 자동으로 공전
		//=====================================================================
		if (mHierarchyParent.IsValid())
		{
			// 부모 Y축 회전 (자식들이 자동으로 부모 주위 공전)
			transformSystem->Rotate(
				mHierarchyParent,
				Math::Vector3(0.0f, mHierarchyRotationSpeed * deltaTime, 0.0f)
			);
		}

		// 자식들 자체 회전 (선택적)
		for (size_t i = 0; i < mHierarchyChildren.size(); ++i)
		{
			Core::float32 speed = 2.0f + static_cast<Core::float32>(i) * 0.5f;
			transformSystem->Rotate(
				mHierarchyChildren[i],
				Math::Vector3(0.0f, 0.0f, speed * deltaTime)
			);
		}

		// 손자 자체 회전
		if (mHierarchyGrandChild.IsValid())
		{
			transformSystem->Rotate(
				mHierarchyGrandChild,
				Math::Vector3(0.0f, 3.0f * deltaTime, 0.0f)
			);
		}
	}

	// SystemManager 업데이트 (모든 System의 Update 호출)
	// - TransformSystem::Update() → 계층 구조 World Matrix 계산 (Phase 3.5)
	// - CameraSystem::Update() → UpdateAllCameras() 자동 호출
	// - RenderSystem::Update() → FrameData 수집
	mSystemManager->UpdateSystems(deltaTime);
}

void PhongLightingApp::OnRender()
{
	auto* renderSystem = mSystemManager->GetSystem<ECS::RenderSystem>();
	if (renderSystem)
	{
		const Graphics::FrameData& frameData = renderSystem->GetFrameData();

		// RenderFrame() 대신 RenderScene()만 호출
		// BeginFrame, EndFrame, Present는 Application이 관리
		GetRenderer()->RenderScene(frameData);
	}
}

void PhongLightingApp::OnShutdown()
{
	LOG_INFO("[PhongLighting] Shutting down...");

	GetDevice()->GetCommandQueue()->WaitForIdle();

	// Phase 3.5: 계층 구조 Entity 정리
	if (mHierarchyGrandChild.IsValid() && mRegistry)
	{
		mRegistry->DestroyEntity(mHierarchyGrandChild);
	}

	for (auto childEntity : mHierarchyChildren)
	{
		if (childEntity.IsValid() && mRegistry)
		{
			mRegistry->DestroyEntity(childEntity);
		}
	}
	mHierarchyChildren.clear();

	// Entities 정리
	for (auto cubeEntity : mCubeEntities)
	{
		if (cubeEntity.IsValid() && mRegistry)
		{
			mRegistry->DestroyEntity(cubeEntity);
		}
	}
	mCubeEntities.clear();

	for (auto lightEntity : mPointLightEntities)
	{
		if (lightEntity.IsValid() && mRegistry)
		{
			mRegistry->DestroyEntity(lightEntity);
		}
	}
	mPointLightEntities.clear();

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
	mResourceManager.reset();

	LOG_INFO("[PhongLighting] Shutdown complete");
}

void PhongLightingApp::OnRenderDebugUI()
{
	Framework::ECSInspector* inspector = GetECSInspector();
	if (inspector)
	{
		inspector->Render(mRegistry.get());
	}
}
