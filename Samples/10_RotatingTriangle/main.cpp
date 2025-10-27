// main.cpp (수정 버전)
#include "Platform/Window.h"
#include "Platform/PlatformTypes.h"
#include "Platform/Input.h"
#include "Core/Logging/Logger.h"
#include "Core/Logging/ConsoleSink.h"
#include "Core/Logging/LogMacros.h"

#include "Math/MathUtils.h"

// Graphics headers
#include "Graphics/Mesh.h"
#include "Graphics/DX12/DX12Device.h"
#include "Graphics/DX12/DX12CommandQueue.h"
#include "Graphics/DX12/DX12SwapChain.h"
#include "Graphics/DX12/DX12DescriptorHeap.h"
#include "Graphics/DX12/DX12CommandContext.h"
#include "Graphics/DX12/DX12Renderer.h"
#include "Graphics/DX12/DX12RootSignature.h"
#include "Graphics/DX12/DX12ShaderCompiler.h"
#include "Graphics/DX12/DX12PipelineStateCache.h"
#include "Graphics/DX12/DX12ConstantBuffer.h"
#include "Graphics/Material.h"

using namespace Microsoft::WRL;

struct MVPConstants
{
	Math::Matrix4x4 MVP;
};


// ============================================================================
// 함수 선언
// ============================================================================

/**
 * @brief 그래픽스 파이프라인 스테이트를 초기화합니다.
 */
bool InitializePipelineState(
	Graphics::DX12Device& device,
	Graphics::DX12RootSignature& rootSignature,
	Graphics::DX12ShaderCompiler& shaderCompiler,
	Graphics::Mesh& mesh,
	Graphics::Material& material,
	Graphics::DX12PipelineStateCache& DX12PipelineStateCache
)
{
	LOG_INFO("Initializing Pipeline State...");

	// DX12PipelineStateCache 초기화
	if (!DX12PipelineStateCache.Initialize(device.GetDevice(), &shaderCompiler))
	{
		LOG_ERROR("Failed to initialize DX12PipelineStateCache");
		return false;
	}

	// PSO 생성 (캐시에 저장됨)
	ID3D12PipelineState* pipelineState = DX12PipelineStateCache.GetOrCreatePipelineState(
		material,
		rootSignature.GetRootSignature(),
		mesh.GetInputLayout()
	);

	if (pipelineState)
	{
		LOG_INFO("Pipeline State initialized successfully");
		return true;
	}
	else
	{
		LOG_ERROR("Failed to initialize Pipeline State");
		return false;
	}
}

/**
 * @brief 삼각형 렌더링을 위한 리소스를 초기화합니다.
 */
bool InitializeForTriangle(
	Graphics::DX12Device& device,
	Graphics::DX12Renderer& renderer,
	Graphics::Mesh& mesh,
	Graphics::DX12RootSignature& rootSignature,
	Graphics::DX12ShaderCompiler& shaderCompiler,
	Graphics::Material& material,
	Graphics::DX12PipelineStateCache& pipelineStateCache,
	Graphics::DX12ConstantBuffer& constantBuffer
)
{
	LOG_INFO("Initializing Triangle Resources...");

	// 삼각형 정점 데이터: 위치와 색상
	Graphics::BasicVertex rectangleVertices[] =
	{
		{ Math::Vector3(-0.3f, 0.25f, 0.0f), Math::Vector4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ Math::Vector3(0.3f, 0.25f, 0.0f), Math::Vector4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ Math::Vector3(-0.3f,-0.25f, 0.0f), Math::Vector4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ Math::Vector3(0.3f, -0.25f, 0.0f), Math::Vector4(0.0f, 1.0f, 0.0f, 1.0f) }
	};

	Core::uint16 triangleIndices[] = { 0, 1, 2, 1, 3, 2 };

	// Mesh 초기화
	if (!mesh.Initialize(
		device.GetDevice(),
		device.GetGraphicsQueue(),
		device.GetCommandContext(renderer.GetCurrentFrameIndex()),
		rectangleVertices,
		4,
		triangleIndices,
		6
	))
	{
		LOG_ERROR("Failed to create mesh");
		return false;
	}

	LOG_INFO("Mesh created successfully");

	// Constant Buffer 초기화
	if (!constantBuffer.Initialize(
		device.GetDevice(),
		sizeof(MVPConstants),
		Graphics::FRAME_BUFFER_COUNT
	))
	{
		LOG_ERROR("Failed to create Constant Buffer");
		return false;
	}

	LOG_INFO("Constant Buffer created successfully");

	// Root Signature 생성 (CBV 포함)
	// Root Parameter 정의: CBV 1개
	CD3DX12_ROOT_PARAMETER1 rootParameters[1];
	rootParameters[0].InitAsConstantBufferView(
		0,                                  // register: b0
		0,                                  // register space: 0
		D3D12_ROOT_DESCRIPTOR_FLAG_NONE,    // flags
		D3D12_SHADER_VISIBILITY_VERTEX      // Vertex Shader에서만 접근
	);

	// Root Signature 초기화
	if (!rootSignature.Initialize(
		device.GetDevice(),
		1,                                              // numParameters
		rootParameters,                                 // parameters
		0,                                              // numStaticSamplers
		nullptr,                                        // staticSamplers
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	))
	{
		LOG_ERROR("Failed to initialize Root Signature");
		return false;
	}

	LOG_INFO("Root Signature created with CBV");

	// Pipeline State 생성
	if (!InitializePipelineState(
		device,
		rootSignature,
		shaderCompiler,
		mesh,
		material,
		pipelineStateCache
	))
	{
		return false;
	}

	LOG_INFO("Triangle Resources initialization completed successfully");
	return true;
}

/**
 * @brief MVP 행렬을 업데이트합니다.
 */
void UpdateMVP(
	Graphics::DX12ConstantBuffer& constantBuffer,
	Core::uint32 frameIndex,
	float timeInSeconds
)
{
	float rotationAngle = timeInSeconds * Math::DegToRad(90.0f);
	Math::Matrix4x4 model = Math::MatrixRotationZ(rotationAngle);
	
	MVPConstants constants;
	constants.MVP = model;

	// Constant Buffer 업데이트
	constantBuffer.Update(frameIndex, &constants, sizeof(MVPConstants));
}

/**
 * @brief 프레임을 렌더링합니다.
 */
void RenderFrame(
	Graphics::DX12Device& device,
	Graphics::DX12Renderer& renderer,
	Graphics::Mesh& mesh,
	Graphics::DX12RootSignature& rootSignature,
	Graphics::Material& material,
	Graphics::DX12PipelineStateCache& DX12PipelineStateCache,
	Graphics::DX12ConstantBuffer& constantBuffer,
	float timeInSeconds
)
{
	auto* swapChain = device.GetSwapChain();
	auto* graphicsQueue = device.GetGraphicsQueue();
	auto* rtvHeap = swapChain->GetRTVHeap();
	Core::uint32 backBufferIndex = swapChain->GetCurrentBackBufferIndex();
	Core::uint32 frameIndex = renderer.GetCurrentFrameIndex();

	// GPU가 현재 백 버퍼 사용을 완료할 때까지 대기
	graphicsQueue->WaitForFenceValue(renderer.GetCurrentFrameFenceValue());

	UpdateMVP(constantBuffer, frameIndex, timeInSeconds);

	// 현재 프레임의 Command Context 가져오기
	auto* cmdContext = device.GetCommandContext(frameIndex);
	if (!cmdContext)
	{
		LOG_ERROR("Failed to get Command Context");
		return;
	}

	// Command List 리셋
	if (!cmdContext->Reset())
	{
		LOG_ERROR("Failed to reset Command Context");
		return;
	}

	auto* cmdList = cmdContext->GetCommandList();
	auto* backBuffer = swapChain->GetCurrentBackBuffer();

	// 리소스 전이: PRESENT → RENDER_TARGET
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		backBuffer,
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	cmdList->ResourceBarrier(1, &barrier);

	// Render Target View 핸들 가져오기
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUHandle(backBufferIndex);

	// 화면 클리어 (Cornflower Blue)
	const float clearColor[] = { 0.392f, 0.584f, 0.929f, 1.0f };
	cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// Render Target 설정
	cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// 뷰포트 설정
	D3D12_VIEWPORT viewport{};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(swapChain->GetWidth());
	viewport.Height = static_cast<float>(swapChain->GetHeight());
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	cmdList->RSSetViewports(1, &viewport);

	// 시저 렉트 설정
	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = static_cast<LONG>(swapChain->GetWidth());
	scissorRect.bottom = static_cast<LONG>(swapChain->GetHeight());
	cmdList->RSSetScissorRects(1, &scissorRect);

	// Pipeline State 및 Root Signature 설정
	cmdList->SetGraphicsRootSignature(rootSignature.GetRootSignature());

	ID3D12PipelineState* pipelineState = DX12PipelineStateCache.GetOrCreatePipelineState(
		material,
		rootSignature.GetRootSignature(),
		mesh.GetInputLayout()
	);

	if (!pipelineState)
	{
		LOG_ERROR("Failed to get Pipeline State");
		return;
	}

	cmdList->SetPipelineState(pipelineState);

	D3D12_GPU_VIRTUAL_ADDRESS cbvAddress = constantBuffer.GetGPUAddress(frameIndex);
	cmdList->SetGraphicsRootConstantBufferView(0, cbvAddress);  // Root Parameter Index 0

	// Primitive Topology 설정
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 삼각형 그리기
	mesh.Draw(cmdList);

	// 리소스 전이: RENDER_TARGET → PRESENT
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		backBuffer,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);
	cmdList->ResourceBarrier(1, &barrier);

	// Command List 닫기
	if (!cmdContext->Close())
	{
		LOG_ERROR("Failed to close Command List");
		return;
	}

	// Command List 실행
	ID3D12CommandList* cmdLists[] = { cmdList };
	Core::uint64 fenceValue = graphicsQueue->ExecuteCommandLists(cmdLists, 1);
	renderer.SetCurrentFrameFenceValue(fenceValue);

	// 화면에 표시
	swapChain->Present(true);

	// 다음 프레임으로 이동
	swapChain->MoveToNextFrame();
	renderer.MoveFrameIndex();
}

int main()
{
	// 로깅 시스템 초기화
	auto& logger = Core::Logging::Logger::GetInstance();
	logger.AddSink(std::make_unique<Core::Logging::ConsoleSink>(true));

	LOG_INFO("=== 10_RotatingTriangle Sample Started ===");

	// 윈도우 생성
	Platform::WindowDesc windowDesc;
	windowDesc.title = "10_RotatingTriangle - DevMiniEngine";
	windowDesc.width = 1280;
	windowDesc.height = 720;
	windowDesc.resizable = true;

	auto window = Platform::CreatePlatformWindow();
	if (!window->Create(windowDesc))
	{
		LOG_ERROR("Failed to create window");
		return -1;
	}

	LOG_INFO("Window created successfully (%dx%d)", windowDesc.width, windowDesc.height);

	HWND hwnd = static_cast<HWND>(window->GetNativeHandle());
	auto& input = window->GetInput();

	// DirectX 12 디바이스 초기화
	Graphics::DX12Renderer renderer{};
	Graphics::DX12Device device;

	if (!device.Initialize(true))
	{
		LOG_ERROR("Failed to initialize DirectX 12 Device");
		window->Destroy();
		return -1;
	}
	LOG_INFO("DirectX 12 Device initialized");

	// SwapChain 생성
	if (!device.CreateSwapChain(hwnd, windowDesc.width, windowDesc.height))
	{
		LOG_ERROR("Failed to create SwapChain");
		device.Shutdown();
		window->Destroy();
		return -1;
	}
	LOG_INFO("SwapChain created");

	// 렌더링 리소스 선언
	Graphics::Mesh mesh;
	Graphics::DX12RootSignature rootSignature;
	Graphics::DX12ShaderCompiler shaderCompiler;
	Graphics::Material material;
	Graphics::DX12PipelineStateCache DX12PipelineStateCache;
	Graphics::DX12ConstantBuffer constantBuffer;

	// 삼각형 리소스 초기화
	if (!InitializeForTriangle(
		device,
		renderer,
		mesh,
		rootSignature,
		shaderCompiler,
		material,
		DX12PipelineStateCache,
		constantBuffer
	))
	{
		LOG_ERROR("Failed to initialize Triangle Resources");

		DX12PipelineStateCache.Shutdown();
		mesh.Shutdown();
		rootSignature.Shutdown();

		device.Shutdown();
		window->Destroy();
		return -1;
	}

	LOG_INFO("DirectX 12 initialization completed successfully!");
	LOG_INFO("Press ESC to exit");

	Core::uint64 startTicks = GetTickCount64();

	// 메인 루프
	while (!window->ShouldClose())
	{
		Core::uint64 currentTicks = GetTickCount64();
		float timeInSeconds = (currentTicks - startTicks) / 1000.0f;

		input.Update();
		window->ProcessEvents();

		if (input.IsKeyPressed(Platform::KeyCode::Escape))
		{
			LOG_INFO("ESC pressed - Exiting");
			break;
		}

		RenderFrame(
			device,
			renderer,
			mesh,
			rootSignature,
			material,
			DX12PipelineStateCache,
			constantBuffer,
			timeInSeconds
		);

		LOG_INFO("Time In Seconds %f", timeInSeconds);

		input.Reset();
	}

	LOG_INFO("10_RotatingTriangle  - Terminated successfully");
	return 0;
}