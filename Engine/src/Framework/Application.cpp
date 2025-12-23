#include "pch.h"
#include "Framework/Application.h"

// Framework/DebugUI
#include "Framework/DebugUI/ImGuiManager.h"
#include "Framework/DebugUI/PerformancePanel.h"
#include "Framework/DebugUI/ECSInspector.h"
#include "Framework/DebugUI/DebugVisualizationPanel.h"

// Core
#include "Core/Logging/LogMacros.h"
#include "Core/Timing/ScopedTimer.h"

// Graphics
#include "Graphics/DX12/DX12CommandContext.h"
#include "Graphics/DX12/DX12CommandQueue.h"
#include "Graphics/DX12/DX12Device.h"
#include "Graphics/DX12/DX12Renderer.h"
#include "Graphics/DebugDraw/DebugRenderer.h"

// Platform
#include "Platform/Input.h"
#include "Platform/PlatformTypes.h"
#include "Platform/Window.h"

// imgui
#include <imgui.h>

namespace Framework
{
	Application::Application(const ApplicationDesc& desc)
		: mDesc(desc)
	{
		LOG_INFO("=== %s Initializing ===", mDesc.applicationName);
	}

	Application::~Application()
	{
		if (mIsInitialized)
		{
			Shutdown();
		}
		LOG_INFO("=== %s Terminated ===", mDesc.applicationName);
	}

	Core::int32 Application::Run()
	{
		// 초기화
		if (!Initialize())
		{
			LOG_ERROR("Application initialization failed");
			return -1;
		}

		// 사용자 초기화 콜백
		if (!OnInitialize())
		{
			LOG_ERROR("User initialization failed");
			Shutdown();
			return -1;
		}

		LOG_INFO("Application started successfully");

		// 메인 루프 실행
		RunMainLoop();

		// 사용자 종료 콜백
		OnShutdown();

		// 엔진 종료
		Shutdown();

		return 0;
	}

	bool Application::Initialize()
	{
		PROFILE_FUNCTION();  // 초기화 시간 측정

		// 타이머 유효성 확인
		if (!mTimer.IsValid())
		{
			LOG_ERROR("Failed to initialize high-precision timer");
			return false;
		}

		LOG_INFO("High-precision timer initialized");

		// 1. 윈도우 생성
		Platform::WindowDesc windowDesc;
		windowDesc.title = mDesc.windowTitle;
		windowDesc.width = mDesc.windowWidth;
		windowDesc.height = mDesc.windowHeight;
		windowDesc.resizable = mDesc.windowResizable;

		mWindow = Platform::CreatePlatformWindow();
		if (!mWindow->Create(windowDesc))
		{
			LOG_ERROR("Failed to create window");
			return false;
		}
		LOG_INFO("Window created (%dx%d)", windowDesc.width, windowDesc.height);

		// 2. DirectX 12 디바이스 초기화
		mDevice = std::make_unique<Graphics::DX12Device>();
		if (!mDevice->Initialize(mDesc.enableDebugLayer))
		{
			LOG_ERROR("Failed to initialize DX12 Device");
			return false;
		}
		LOG_INFO("DirectX 12 Device initialized");

		// 3. SwapChain 생성
		HWND hwnd = static_cast<HWND>(mWindow->GetNativeHandle());
		if (!mDevice->CreateSwapChain(hwnd, windowDesc.width, windowDesc.height))
		{
			LOG_ERROR("Failed to create SwapChain");
			return false;
		}
		LOG_INFO("SwapChain created");

		// 4. 렌더러 초기화
		mRenderer = std::make_unique<Graphics::DX12Renderer>();
		if (!mRenderer->Initialize(mDevice.get(), windowDesc.width, windowDesc.height))
		{
			LOG_ERROR("Failed to initialize Renderer");
			return false;
		}
		LOG_INFO("Renderer initialized");

		// 5. Debug UI
		if (!InitializeDebugUI())
		{
			LOG_ERROR("Failed to initialize Debug UI");
			return false;
		}

		mIsInitialized = true;

		return true;
	}

	bool Application::InitializeDebugUI()
	{
		mImGuiManager = std::make_unique<ImGuiManager>();
		if (!mImGuiManager->Initialize(mWindow.get(), mDevice.get()))
		{
			LOG_ERROR("Failed to initialize ImGuiManager");
			return false;
		}
		LOG_INFO("ImGuiManager initialized");

		mPerformancePanel = std::make_unique<PerformancePanel>();
		LOG_INFO("PerformancePanel created");

		mECSInspector = std::make_unique<ECSInspector>();
		LOG_INFO("ECSInspector created");

		mDebugVisualizationPanel = std::make_unique<DebugVisualizationPanel>();
		LOG_INFO("DebugVisualizationPanel created");

		if (mECSInspector && mRenderer && mRenderer->GetDebugRenderer())
		{
			mECSInspector->SetSelectionChangedCallback(
				[this](ECS::Entity entity)
				{
					if (auto* debugRenderer = mRenderer->GetDebugRenderer())
					{
						debugRenderer->SetSelectedEntity(entity);
					}
				}
			);
		}

		return true;
	}

	void Application::Shutdown()
	{
		if (!mIsInitialized)
		{
			return;
		}

		LOG_INFO("Application shutting down...");

		// GPU 작업 완료 대기
		if (mDevice && mDevice->GetCommandQueue())
		{
			mDevice->GetCommandQueue()->WaitForIdle();
		}

		// Debug UI 정리
		ShutdownDebugUI();

		// 역순으로 정리
		mRenderer.reset();
		mDevice.reset();
		mWindow.reset();

		mIsInitialized = false;
	}

	void Application::ShutdownDebugUI()
	{
		mDebugVisualizationPanel.reset();
		mECSInspector.reset();
		mPerformancePanel.reset();
		mImGuiManager.reset();
	}

	void Application::RunMainLoop()
	{
		auto& input = mWindow->GetInput();
		mIsRunning = true;

		mTimer.Reset();
		mTimer.Start();

		Core::float32 fpsLogTimer = 0.0f;
		const Core::float32 FPS_LOG_INTERVAL = 1.0f;

		while (mIsRunning && !mWindow->ShouldClose())
		{
			mTimer.Tick(mDesc.enableVSync ? 60.0f : 0.0f);
			Core::float32 deltaTime = mTimer.GetDeltaTime();

			// FPS 로깅
			fpsLogTimer += deltaTime;
			if (fpsLogTimer >= FPS_LOG_INTERVAL)
			{
				LOG_DEBUG(
					"%s | Frame Time: %.3fms | Total: %.2fs",
					mTimer.GetFrameRateString().c_str(),
					deltaTime * 1000.0f,
					mTimer.GetTotalTime()
				);
				fpsLogTimer -= FPS_LOG_INTERVAL;
			}

			// 입력 업데이트
			input.Update();

			// 윈도우 이벤트 처리
			mWindow->ProcessEvents();

			// 입력 처리
			ProcessInput(input);

			// 업데이트
			OnUpdate(deltaTime);
			UpdateDebugUI(deltaTime);

			// 렌더링
			if (mRenderer->BeginFrame())
			{
				// 사용자 렌더링 (씬)
				OnRender();

				// Debug UI 렌더링
				RenderDebugUI();

				// 프레임 종료 및 출력
				mRenderer->EndFrame();
				mRenderer->Present(mDesc.enableVSync);
			}

			// 입력 리셋
			input.Reset();
		}
	}

	void Application::ProcessInput(Platform::Input& input)
	{
		// ESC: 종료
		if (input.IsKeyPressed(Platform::KeyCode::Escape))
		{
			LOG_INFO("ESC pressed - Requesting exit");
			RequestExit();
		}

		// F1: Performance Panel 토글
		if (input.IsKeyPressed(Platform::KeyCode::F1))
		{
			if (mPerformancePanel)
			{
				mPerformancePanel->SetVisible(!mPerformancePanel->IsVisible());
			}
		}

		// F2: ECS Inspector 토글
		if (input.IsKeyPressed(Platform::KeyCode::F2))
		{
			if (mECSInspector)
			{
				mECSInspector->SetVisible(!mECSInspector->IsVisible());
			}
		}

		// F3: 전체 Debug UI 토글
		if (input.IsKeyPressed(Platform::KeyCode::F3))
		{
			mShowDebugUI = !mShowDebugUI;
		}
	}

	void Application::UpdateDebugUI(Core::float32 deltaTime)
	{
		if (mPerformancePanel)
		{
			mPerformancePanel->Update(deltaTime);
		}
	}

	void Application::RenderDebugUI()
	{

		if (!mShowDebugUI)  // 추가
		{
			return;
		}

		if (!mImGuiManager || !mImGuiManager->IsInitialized())
		{
			return;
		}

		// ImGui 프레임 시작
		mImGuiManager->BeginFrame();

		// 테스트: Demo 창 표시
		ImGui::ShowDemoWindow();

		// PerformancePanel 렌더링
		if (mPerformancePanel)
		{
			mPerformancePanel->Render(nullptr);  // Registry는 파생 클래스에서 전달
		}

		// DebugVisualizationPanel 렌더링
		if (mDebugVisualizationPanel && mRenderer)
		{
			mDebugVisualizationPanel->Render(mRenderer->GetDebugRenderer());
		}

		// 파생 클래스의 UI 코드 호출
		OnRenderDebugUI();

		// ImGui 렌더링 제출
		mImGuiManager->EndFrame(mRenderer->GetCurrentCommandList());
	}

} // namespace Framework
