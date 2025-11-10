#include "pch.h"
#include "Framework/Application.h"
#include "Core/Timing/ScopedTimer.h"
#include "Core/Logging/LogMacros.h"
#include "Graphics/DX12/DX12Device.h"
#include "Graphics/DX12/DX12Renderer.h"
#include "Platform/Input.h"
#include "Platform/PlatformTypes.h"
#include "Platform/Window.h"

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

		mIsInitialized = true;
		return true;
	}

	void Application::Shutdown()
	{
		if (!mIsInitialized)
		{
			return;
		}

		LOG_INFO("Shutting down application...");

		// 역순으로 정리
		mRenderer.reset();
		mDevice.reset();
		mWindow.reset();

		mIsInitialized = false;
	}

	void Application::RunMainLoop()
	{
		auto& input = mWindow->GetInput();
		mIsRunning = true;

		// 타이머 시작
		mTimer.Reset();
		mTimer.Start();

		// FPS 로깅용 타이머
		Core::float32 fpsLogTimer = 0.0f;
		const Core::float32 FPS_LOG_INTERVAL = 1.0f;  // 1초마다

		while (mIsRunning && !mWindow->ShouldClose())
		{
			// 타이머 업데이트 (VSync 활성화 시 60 FPS 제한)
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

			// ESC 키 종료 처리
			if (input.IsKeyPressed(Platform::KeyCode::Escape))
			{
				LOG_INFO("ESC pressed - Requesting exit");
				RequestExit();
			}

			// 업데이트
			{
				//PROFILE_SCOPE("Update");
				OnUpdate(deltaTime);
			}

			// 렌더링
			{
				//PROFILE_SCOPE("Render");
				OnRender();
			}

			// 입력 리셋
			input.Reset();
		}
	}

} // namespace Framework
