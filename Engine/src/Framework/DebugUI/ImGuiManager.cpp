#include "pch.h"

#include "Framework/DebugUI/ImGuiManager.h"

// Core
#include "Core/Logging/LogMacros.h"

// Graphics
#include "Graphics/GraphicsTypes.h"
#include "Graphics/DX12/DX12Device.h"
#include "Graphics/DX12/DX12SwapChain.h"
#include "Graphics/DX12/DX12CommandContext.h"
#include "Graphics/DX12/DX12CommandQueue.h"

// Platform
#include "Platform/Window.h"

// ThirdParty - ImGui
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>

// DirectX
#include <d3d12.h>

namespace Framework
{
	// ImGui Win32 메시지 핸들러 (extern 선언)
	extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	ImGuiManager::~ImGuiManager()
	{
		if (mIsInitialized)
		{
			Shutdown();
		}
	}

	bool ImGuiManager::Initialize(Platform::Window* window, Graphics::DX12Device* device)
	{
		if (mIsInitialized)
		{
			LOG_WARN("[ImGuiManager] Already initialized");
			return true;
		}

		if (!window || !device)
		{
			LOG_ERROR("[ImGuiManager] Invalid window or device");
			return false;
		}

		// ImGui 컨텍스트 생성
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		// 스타일 설정
		ImGui::StyleColorsDark();

		// Win32 백엔드 초기화
		HWND hwnd = static_cast<HWND>(window->GetNativeHandle());
		if (!ImGui_ImplWin32_Init(hwnd))
		{
			LOG_ERROR("[ImGuiManager] Failed to initialize Win32 backend");
			ImGui::DestroyContext();
			return false;
		}

		// DX12 리소스 획득
		ID3D12Device* d3dDevice = device->GetDevice();
		if (!d3dDevice)
		{
			LOG_ERROR("[ImGuiManager] Failed to get D3D12 device");
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
			return false;
		}

		// ImGui 전용 SRV Descriptor Heap 생성
		if (!CreateDescriptorHeap(d3dDevice))
		{
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
			return false;
		}
		
		// DX12 백엔드 초기화
		Graphics::DX12SwapChain* swapChain = device->GetSwapChain();
		if (!swapChain)
		{
			LOG_ERROR("[ImGuiManager] Failed to get D3D12 swapchain");
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
			return false;
		}
		const Core::uint32 NUM_FRAMES_IN_FLIGHT = Graphics::FRAME_BUFFER_COUNT;
		const DXGI_FORMAT BACK_BUFFER_FORMAT = swapChain->GetFormat();
		
		if (!ImGui_ImplDX12_Init(
			d3dDevice,
			static_cast<int>(NUM_FRAMES_IN_FLIGHT),
			BACK_BUFFER_FORMAT,
			mSrvHeap.Get(),
			mSrvHeap->GetCPUDescriptorHandleForHeapStart(),
			mSrvHeap->GetGPUDescriptorHandleForHeapStart()
		))
		{
			LOG_ERROR("[ImGuiManager] Failed to initialize DX12 backend");
			mSrvHeap.Reset();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
			return false;
		}

		// 폰트 텍스처 업로드
		if (!UploadFontTexture(device))
		{
			LOG_ERROR("[ImGuiManager] Failed to upload font texture");
			ImGui_ImplDX12_Shutdown();
			mSrvHeap.Reset();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
			return false;
		}

		mIsInitialized = true;
		LOG_INFO("[ImGuiManager] Initialized successfully");
		return true;
	}

	void ImGuiManager::Shutdown()
	{
		if (!mIsInitialized)
		{
			return;
		}

		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		mSrvHeap.Reset();

		mIsInitialized = false;
		LOG_INFO("[ImGuiManager] Shutdown complete");
	}

	void ImGuiManager::BeginFrame()
	{
		if (!mIsInitialized)
		{
			return;
		}

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiManager::EndFrame(ID3D12GraphicsCommandList* commandList)
	{
		if (!mIsInitialized || !commandList)
		{
			return;
		}

		ImGui::Render();

		// ImGui 전용 Descriptor Heap 바인딩
		ID3D12DescriptorHeap* heaps[] = { mSrvHeap.Get() };
		commandList->SetDescriptorHeaps(1, heaps);

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
	}

	bool ImGuiManager::CreateDescriptorHeap(ID3D12Device* device)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 64;  // ImGui 폰트 텍스처용
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		desc.NodeMask = 0;

		HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mSrvHeap));
		if (FAILED(hr))
		{
			LOG_ERROR("[ImGuiManager] Failed to create descriptor heap: 0x%08X", hr);
			return false;
		}

		return true;
	}

	bool ImGuiManager::UploadFontTexture(Graphics::DX12Device* device)
	{
		if (!ImGui::GetIO().Fonts->Build())
		{
			LOG_ERROR("Build Fail!");
			return false;
		}

		auto* commandQueue = device->GetCommandQueue();
		auto* commandContext = device->GetCommandContext(0);

		if (!commandContext || !commandQueue)
		{
			LOG_ERROR("[ImGuiManager] Invalid command context or queue");
			return false;
		}

		// CommandList 리셋
		if (!commandContext->Reset())
		{
			LOG_ERROR("[ImGuiManager] Failed to reset command context");
			return false;
		}

		// 폰트 텍스처 생성 명령 기록
		if (!ImGui_ImplDX12_CreateDeviceObjects())
		{
			LOG_ERROR("[ImGuiManager] ImGui_ImplDX12_CreateDeviceObjects failed");
			commandContext->Close(); // 실패해도 닫아주는 것이 안전
			return false;
		}

		// CommandList 닫기
		if (!commandContext->Close())
		{
			LOG_ERROR("[ImGuiManager] Failed to close command context");
			return false;
		}

		// 실행 및 대기
		ID3D12CommandList* cmdLists[] = { commandContext->GetCommandList() };
		commandQueue->ExecuteCommandLists(1, cmdLists);
		commandQueue->WaitForIdle();

		LOG_INFO("[ImGuiManager] Font texture uploaded");
		return true;
	}

} // namespace Framework
