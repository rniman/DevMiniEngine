#pragma once
#include "Core/Timing/Timer.h" 
#include "Core/Types.h"
#include <memory>

namespace Platform
{
	class Window;
	class Input;
}

namespace Graphics
{
	class DX12Device;
	class DX12Renderer;
}

namespace Framework
{
	class ImGuiManager;  // 전방 선언
	class PerformancePanel;
	class ECSInspector;

	struct ApplicationDesc
	{
		const char* applicationName = "DevMiniEngine Application";
		const char* windowTitle = "DevMiniEngine";
		Core::uint32 windowWidth = 1280;
		Core::uint32 windowHeight = 720;
		bool windowResizable = true;
		bool enableVSync = true;
		bool enableDebugLayer = true;  // DirectX 12 디버그 레이어
	};

	/**
	 * @brief 애플리케이션 베이스 클래스
	 *
	 * 엔진의 라이프사이클을 관리하고 하위 시스템들을 초기화합니다.
	 */
	class Application
	{
	public:
		Application(const ApplicationDesc& desc);
		virtual ~Application();

		// 복사/이동 금지
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

		/**
		 * @brief 애플리케이션 실행 진입점
		 * @return 종료 코드
		 */
		Core::int32 Run();

		/**
		 * @brief 애플리케이션 종료 요청
		 */
		void RequestExit() { mIsRunning = false; }

	protected:
		// 파생 클래스에서 오버라이드할 라이프사이클 메서드

		/**
		 * @brief 초기화 콜백 (Run 시작 시 한 번 호출)
		 * @return 초기화 성공 여부
		 */
		virtual bool OnInitialize() { return true; }

		/**
		 * @brief 업데이트 콜백 (매 프레임 호출)
		 * @param deltaTime 이전 프레임과의 시간 차이 (초 단위)
		 */
		virtual void OnUpdate(Core::float32 deltaTime) { (void)deltaTime; }

		/**
		 * @brief 렌더링 콜백 (매 프레임 호출)
		 */
		virtual void OnRender() {}

		/**
		 * @brief 종료 콜백 (Run 종료 시 한 번 호출)
		 */
		virtual void OnShutdown() {}

		/**
		 * @brief Debug UI 렌더링 콜백 (Debug 빌드에서만 호출)
		 *
		 * ImGui::Begin/End 등의 UI 코드를 작성합니다.
		 * BeginFrame/EndFrame은 Application이 자동 호출합니다.
		 */
		virtual void OnRenderDebugUI() {}

		// 하위 시스템 접근자
		Platform::Window* GetWindow() const { return mWindow.get(); }
		Graphics::DX12Device* GetDevice() const { return mDevice.get(); }
		Graphics::DX12Renderer* GetRenderer() const { return mRenderer.get(); }

		// 타이머 접근자
		Core::Timing::Timer* GetTimer() { return &mTimer; }
		const Core::Timing::Timer* GetTimer() const { return &mTimer; }

	protected:
		// 파생 클래스에서 접근 가능한 Debug UI 접근자
		PerformancePanel* GetPerformancePanel() const { return mPerformancePanel.get(); }
		ECSInspector* GetECSInspector() const { return mECSInspector.get(); }

	private:
		bool Initialize();
		bool InitializeDebugUI();
		void Shutdown();
		void ShutdownDebugUI();
		void RunMainLoop();
		void ProcessInput(Platform::Input& input);
		void UpdateDebugUI(Core::float32 deltaTime);

		// ImGui 렌더링 (내부 호출)
		void RenderDebugUI();

		ApplicationDesc mDesc;
		bool mIsRunning = false;
		bool mIsInitialized = false;

		// 엔진 하위 시스템
		std::unique_ptr<Platform::Window> mWindow;
		std::unique_ptr<Graphics::DX12Device> mDevice;
		std::unique_ptr<Graphics::DX12Renderer> mRenderer;

		Core::Timing::Timer mTimer;

		std::unique_ptr<ImGuiManager> mImGuiManager;
		std::unique_ptr<PerformancePanel> mPerformancePanel;
		std::unique_ptr<ECSInspector> mECSInspector;

		bool mShowDebugUI = true;
	};

} // namespace Framework
