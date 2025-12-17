#pragma once

#include "Core/Types.h"

// Forward declarations
struct ID3D12DescriptorHeap;
struct ID3D12GraphicsCommandList;

namespace Platform
{
	class Window;
}

namespace Graphics
{
	class DX12Device;
}

namespace Framework
{
	/**
	 * @brief ImGui 초기화, 렌더링, 종료를 관리하는 래퍼 클래스
	 *
	 * DirectX 12 및 Win32 백엔드를 사용합니다.
	 */
	class ImGuiManager
	{
	public:
		ImGuiManager() = default;
		~ImGuiManager();

		// 복사/이동 방지
		ImGuiManager(const ImGuiManager&) = delete;
		ImGuiManager& operator=(const ImGuiManager&) = delete;
		ImGuiManager(ImGuiManager&&) = delete;
		ImGuiManager& operator=(ImGuiManager&&) = delete;

		/**
		 * @brief ImGui 초기화
		 * @param window Win32 윈도우 (HWND 획득용)
		 * @param device DX12 디바이스 (리소스 생성용)
		 * @return 성공 여부
		 */
		bool Initialize(Platform::Window* window, Graphics::DX12Device* device);

		/**
		 * @brief ImGui 종료 및 리소스 해제
		 */
		void Shutdown();

		/**
		 * @brief 프레임 시작 (NewFrame 호출)
		 *
		 * UI 코드 작성 전에 호출해야 합니다.
		 */
		void BeginFrame();

		/**
		 * @brief 프레임 종료 (Render 및 DrawData 제출)
		 *
		 * UI 코드 작성 후, Present 전에 호출해야 합니다.
		 * @param commandList 현재 커맨드 리스트
		 */
		void EndFrame(ID3D12GraphicsCommandList* commandList);

		/**
		 * @brief 초기화 상태 확인
		 */
		bool IsInitialized() const { return mIsInitialized; }

	private:
		bool CreateDescriptorHeap(struct ID3D12Device* device);
		bool UploadFontTexture(Graphics::DX12Device* device);

	private:
		ComPtr<ID3D12DescriptorHeap> mSrvHeap = nullptr;
		bool mIsInitialized = false;
	};

} // namespace Framework
