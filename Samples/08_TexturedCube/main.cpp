#pragma warning(push, 0)
#include "d3dx12.h"
#pragma warning(pop)

#include "Framework/Application.h"
#include "Framework/Scene/Scene.h"
#include "Framework/Scene/GameObject.h"
#include "Framework/Resources/ResourceManager.h"

#include "Graphics/DX12/DX12Device.h"
#include "Graphics/DX12/DX12Renderer.h"
#include "Graphics/Mesh.h"
#include "Graphics/Texture.h"
#include "Graphics/Material.h"

#include "Core/Logging/Logger.h"
#include "Core/Logging/ConsoleSink.h"
#include "Math/MathUtils.h"

class TexturedCubeApp : public Framework::Application
{
public:
	TexturedCubeApp()
		: Application(GetAppDesc())
	{
	}

	~TexturedCubeApp() override
	{
		OnShutdown();
	}

protected:
	bool OnInitialize() override
	{
		LOG_INFO("=== TexturedCubeApp Initializing ===");

		// 리소스 매니저 생성
		mResourceManager = std::make_unique<Framework::ResourceManager>(
			GetDevice(),
			GetRenderer());

		// 씬 생성
		mScene = std::make_unique<Framework::Scene>();

		// 렌더링 리소스 초기화
		if (!GetRenderer()->Initialize(
			GetDevice(),
			GetAppDesc().windowWidth,
			GetAppDesc().windowHeight))
		{
			LOG_ERROR("Failed to initialize rendering resources");
			return false;
		}

		// 큐브 생성
		if (!CreateCube())
		{
			LOG_ERROR("Failed to create cube");
			return false;
		}

		// 카메라 설정
		mScene->GetMainCamera()->SetLookAt(
			Math::Vector3(0.0f, 10.0f, -20.0f),
			Math::Vector3(0.0f, 0.0f, 0.0f),
			Math::Vector3(0.0f, 1.0f, 0.0f));

		LOG_INFO("TexturedCubeApp initialized successfully");
		return true;
	}

	void OnUpdate(Core::float32 deltaTime) override
	{
		// 큐브 회전
		mTotalTime += deltaTime;

		if (auto* cube = mScene->FindGameObject("Cube"))
		{
			Core::float32 rotationAngle = mTotalTime * Math::DegToRad(90.0f);
			auto rotation = Math::QuaternionFromAxisAngle(
				Math::Vector3(0, 1, 0),  // Y축
				rotationAngle);
			cube->SetRotation(rotation);
		}

		// 씬 업데이트
		mScene->Update(deltaTime);
	}

	void OnRender() override
	{
		// 1. Scene에서 렌더링 데이터 수집
		Graphics::FrameData frameData;
		mScene->CollectRenderData(frameData);

		// 2. Renderer가 실제 렌더링 수행
		GetRenderer()->RenderFrame(frameData);
	}

	void OnShutdown() override
	{
		LOG_INFO("TexturedCubeApp shutting down...");

		// DX12Renderer가 리소스를 참조하므로 먼저 정리
		if (auto* renderer = GetRenderer())
		{
			renderer->Shutdown();
		}

		mScene.reset();
		mResourceManager.reset();


		LOG_INFO("TexturedCubeApp shutdown complete");
	}

private:
	bool CreateCube()
	{
		// 큐브 메시 생성
		auto cubeMesh = mResourceManager->CreateMesh("CubeMesh");
		if (!CreateCubeMesh(cubeMesh))
		{
			return false;
		}

		// 머티리얼 생성
		auto material = mResourceManager->CreateMaterial(
			"BrickMaterial",
			L"TexturedShader.hlsl",
			L"TexturedShader.hlsl");

		// 텍스처 로드
		auto diffuseTex = mResourceManager->LoadTexture(
			L"../../Assets/Textures/BrickWall17_1K_BaseColor.png");
		auto normalTex = mResourceManager->LoadTexture(
			L"../../Assets/Textures/BrickWall17_1K_Normal.png");

		if (!diffuseTex || !normalTex)
		{
			LOG_ERROR("Failed to load textures");
			return false;
		}

		material->SetTexture(Graphics::TextureType::Diffuse, diffuseTex);
		material->SetTexture(Graphics::TextureType::Normal, normalTex);

		// TODO: 아키텍처 리팩토링
		// 이 호출은 Application(main) 레이어가 Renderer의 내부 구현(SRV 힙)을
		// 직접 참조하게 만들어 캡슐화를 위반.
		if (!material->AllocateDescriptors(
			GetDevice()->GetDevice(),
			GetRenderer()->GetSrvDescriptorHeap()))
		{
			LOG_ERROR("Failed to allocate material descriptors");
			return false;
		}

		// 게임 오브젝트 생성
		auto* cube = mScene->CreateGameObject("Cube");
		cube->SetMesh(cubeMesh);
		cube->SetMaterial(material);
		cube->SetPosition(Math::Vector3(0, 0, 0));

		LOG_INFO("Cube created successfully");
		return true;
	}

	bool CreateCubeMesh(std::shared_ptr<Graphics::Mesh> mesh)
	{
		// 큐브 정점 데이터 (6면 * 4정점 = 24정점)
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

		// 실제로는 24개 정점과 36개 인덱스를 모두 정의해야 합니다
		return mesh->InitializeTextured(
			GetDevice()->GetDevice(),
			GetDevice()->GetGraphicsQueue(),
			GetDevice()->GetCommandContext(GetRenderer()->GetCurrentFrameIndex()),
			vertices,
			24,
			indices,
			36);
	}

	static Framework::ApplicationDesc GetAppDesc()
	{
		Framework::ApplicationDesc desc;
		desc.applicationName = "TexturedCubeApp";
		desc.windowTitle = "08_TexturedCube - DevMiniEngine";
		desc.windowWidth = 1280;
		desc.windowHeight = 720;
		desc.enableVSync = true;
		desc.enableDebugLayer = true;
		return desc;
	}

	// 씬 및 리소스
	std::unique_ptr<Framework::Scene> mScene;
	std::unique_ptr<Framework::ResourceManager> mResourceManager;

	// 애니메이션
	Core::float32 mTotalTime = 0.0f;
};

// 진입점
int main()
{
	// 로깅 시스템 초기화
	auto& logger = Core::Logging::Logger::GetInstance();
	logger.AddSink(std::make_unique<Core::Logging::ConsoleSink>(true));

	// 애플리케이션 실행
	TexturedCubeApp app;
	return app.Run();
}
