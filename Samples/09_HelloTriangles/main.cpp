#include "Platform/Window.h"
#include "Platform/PlatformTypes.h"
#include "Platform/Input.h"
#include "Core/Logging/Logger.h"
#include "Core/Logging/ConsoleSink.h"
#include "Core/Logging/LogMacros.h"

// Graphics headers
#include "Graphics/DX12/DX12Device.h"
#include "Graphics/DX12/DX12CommandQueue.h"
#include "Graphics/DX12/DX12SwapChain.h"
#include "Graphics/DX12/DX12DescriptorHeap.h"
#include "Graphics/DX12/DX12CommandContext.h"
#include "Graphics/DX12/DX12Renderer.h"
#include "Graphics/DX12/DX12VertexBuffer.h"
#include "Graphics/DX12/DX12IndexBuffer.h"
#include "Graphics/DX12/DX12GraphicsRootSignature.h"
#include "Graphics/DX12/DX12PipelineState.h"
#include "Graphics/DX12/DX12ShaderCompiler.h"

using namespace Microsoft::WRL;

/**
 * @brief 그래픽스 파이프라인 스테이트를 초기화합니다.
 *
 * 래스터라이저, 블렌드, 깊이-스텐실 상태를 설정하고 셰이더를 컴파일하여
 * PSO를 생성합니다.
 */
bool InitializePipelineState(
	Graphics::DX12Device& device,
	Graphics::DX12GraphicsRootSignature& rootSignature,
	const D3D12_INPUT_LAYOUT_DESC& inputLayoutDesc,
	Graphics::DX12ShaderCompiler& shaderCompiler,
	Graphics::DX12PipelineState& pipelineState
)
{
	LOG_INFO("Initializing Pipeline State...");

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};
	pipelineStateDesc.pRootSignature = rootSignature.Get();
	pipelineStateDesc.InputLayout = inputLayoutDesc;

	// Vertex Shader 컴파일
	ComPtr<ID3DBlob> vertexShaderBlob;
	if (!shaderCompiler.CompileFromFile(
		L"BasicShader.hlsl",
		"VSMain",
		"vs_5_1",
		vertexShaderBlob.GetAddressOf()))
	{
		LOG_ERROR("Failed to compile Vertex Shader");
		return false;
	}
	pipelineStateDesc.VS.pShaderBytecode = vertexShaderBlob->GetBufferPointer();
	pipelineStateDesc.VS.BytecodeLength = vertexShaderBlob->GetBufferSize();

	// Pixel Shader 컴파일
	ComPtr<ID3DBlob> pixelShaderBlob;
	if (!shaderCompiler.CompileFromFile(
		L"BasicShader.hlsl",
		"PSMain",
		"ps_5_1",
		pixelShaderBlob.GetAddressOf()))
	{
		LOG_ERROR("Failed to compile Pixel Shader");
		return false;
	}
	pipelineStateDesc.PS.pShaderBytecode = pixelShaderBlob->GetBufferPointer();
	pipelineStateDesc.PS.BytecodeLength = pixelShaderBlob->GetBufferSize();

	// Rasterizer State: 솔리드 모드, 후면 컬링
	pipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	pipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	pipelineStateDesc.RasterizerState.FrontCounterClockwise = FALSE;
	pipelineStateDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	pipelineStateDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	pipelineStateDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	pipelineStateDesc.RasterizerState.DepthClipEnable = TRUE;
	pipelineStateDesc.RasterizerState.MultisampleEnable = FALSE;
	pipelineStateDesc.RasterizerState.AntialiasedLineEnable = FALSE;
	pipelineStateDesc.RasterizerState.ForcedSampleCount = 0;
	pipelineStateDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	// Blend State: 알파 블렌딩 비활성화
	pipelineStateDesc.BlendState.AlphaToCoverageEnable = FALSE;
	pipelineStateDesc.BlendState.IndependentBlendEnable = FALSE;

	const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
	{
		FALSE,                              // BlendEnable
		FALSE,                              // LogicOpEnable
		D3D12_BLEND_ONE,                    // SrcBlend
		D3D12_BLEND_ZERO,                   // DestBlend
		D3D12_BLEND_OP_ADD,                 // BlendOp
		D3D12_BLEND_ONE,                    // SrcBlendAlpha
		D3D12_BLEND_ZERO,                   // DestBlendAlpha
		D3D12_BLEND_OP_ADD,                 // BlendOpAlpha
		D3D12_LOGIC_OP_NOOP,                // LogicOp
		D3D12_COLOR_WRITE_ENABLE_ALL        // RenderTargetWriteMask
	};

	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
	{
		pipelineStateDesc.BlendState.RenderTarget[i] = defaultRenderTargetBlendDesc;
	}

	// Depth-Stencil State: 깊이 테스트 비활성화 (2D 렌더링)
	pipelineStateDesc.DepthStencilState.DepthEnable = FALSE;
	pipelineStateDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	pipelineStateDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	pipelineStateDesc.DepthStencilState.StencilEnable = FALSE;
	pipelineStateDesc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	pipelineStateDesc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

	const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
	{
		D3D12_STENCIL_OP_KEEP,              // StencilFailOp
		D3D12_STENCIL_OP_KEEP,              // StencilDepthFailOp
		D3D12_STENCIL_OP_KEEP,              // StencilPassOp
		D3D12_COMPARISON_FUNC_ALWAYS        // StencilFunc
	};

	pipelineStateDesc.DepthStencilState.FrontFace = defaultStencilOp;
	pipelineStateDesc.DepthStencilState.BackFace = defaultStencilOp;

	// Sample 설정
	pipelineStateDesc.SampleMask = UINT_MAX;
	pipelineStateDesc.SampleDesc.Count = 1;
	pipelineStateDesc.SampleDesc.Quality = 0;

	// Primitive Topology 및 Render Target 설정
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateDesc.NumRenderTargets = 1;
	pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pipelineStateDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;

	bool success = pipelineState.Initialize(device.GetDevice(), pipelineStateDesc);

	if (success)
	{
		LOG_INFO("Pipeline State initialized successfully");
	}
	else
	{
		LOG_ERROR("Failed to initialize Pipeline State");
	}

	return success;
}

/**
 * @brief 삼각형 렌더링을 위한 리소스를 초기화합니다.
 *
 * Vertex Buffer, Index Buffer, Root Signature, Pipeline State를 생성합니다.
 */
bool InitializeForTriangle(
	Graphics::DX12Device& device,
	Graphics::DX12Renderer& renderer,
	Graphics::DX12VertexBuffer& vertexBuffer,
	Graphics::DX12IndexBuffer& indexBuffer,
	Graphics::DX12GraphicsRootSignature& rootSignature,
	Graphics::DX12PipelineState& pipelineState,
	Graphics::DX12ShaderCompiler& shaderCompiler
)
{
	LOG_INFO("Initializing Triangle Resources...");

	// 삼각형 정점 데이터: 위치와 색상
	Graphics::BasicVertex triangleVertices[] =
	{
		{ Math::Vector3(0.0f,  0.5f, 0.0f), Math::Vector4(1.0f, 0.0f, 0.0f, 1.0f) },  // 상단 - 빨강
		{ Math::Vector3(0.5f, -0.5f, 0.0f), Math::Vector4(0.0f, 1.0f, 0.0f, 1.0f) },  // 우하단 - 초록
		{ Math::Vector3(-0.5f, -0.5f, 0.0f), Math::Vector4(0.0f, 0.0f, 1.0f, 1.0f) }   // 좌하단 - 파랑
	};

	Core::uint16 triangleIndices[] = { 0, 1, 2 };

	// Vertex Buffer 생성
	if (!vertexBuffer.Initialize(
		device.GetDevice(),
		device.GetGraphicsQueue(),
		device.GetCommandContext(renderer.GetCurrentFrameIndex()),
		triangleVertices,
		3,
		sizeof(Graphics::BasicVertex)))
	{
		LOG_ERROR("Failed to initialize Vertex Buffer");
		return false;
	}
	LOG_INFO("Vertex Buffer created (3 vertices)");

	// Index Buffer 생성
	if (!indexBuffer.Initialize(
		device.GetDevice(),
		device.GetGraphicsQueue(),
		device.GetCommandContext(renderer.GetCurrentFrameIndex()),
		triangleIndices,
		3,
		DXGI_FORMAT_R16_UINT))
	{
		LOG_ERROR("Failed to initialize Index Buffer");
		return false;
	}
	LOG_INFO("Index Buffer created (3 indices)");

	// Root Signature 생성
	if (!rootSignature.Initialize(device.GetDevice()))
	{
		LOG_ERROR("Failed to initialize Root Signature");
		return false;
	}
	LOG_INFO("Root Signature created");

	// Input Layout 정의
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	D3D12_INPUT_ELEMENT_DESC elements[] =
	{
		{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			0,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
		{
			"COLOR",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			12,  // sizeof(Vector3)
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		}
	};
	inputLayoutDesc.pInputElementDescs = elements;
	inputLayoutDesc.NumElements = 2;

	// Pipeline State 생성
	if (!InitializePipelineState(
		device,
		rootSignature,
		inputLayoutDesc,
		shaderCompiler,
		pipelineState))
	{
		return false;
	}

	LOG_INFO("Triangle Resources initialization completed successfully");
	return true;
}

/**
 * @brief 프레임을 렌더링합니다.
 *
 * 커맨드 리스트를 기록하고 실행하여 화면에 삼각형을 그립니다.
 */
void RenderFrame(
	Graphics::DX12Device& device,
	Graphics::DX12Renderer& renderer,
	Graphics::DX12VertexBuffer& vertexBuffer,
	Graphics::DX12IndexBuffer& indexBuffer,
	Graphics::DX12GraphicsRootSignature& rootSignature,
	Graphics::DX12PipelineState& pipelineState
)
{
	auto* swapChain = device.GetSwapChain();
	auto* graphicsQueue = device.GetGraphicsQueue();
	auto* rtvHeap = swapChain->GetRTVHeap();
	Core::uint32 backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	// GPU가 현재 백 버퍼 사용을 완료할 때까지 대기
	graphicsQueue->WaitForFenceValue(renderer.GetCurrentFrameFenceValue());

	// 현재 프레임의 Command Context 가져오기
	auto* cmdContext = device.GetCommandContext(renderer.GetCurrentFrameIndex());
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
	cmdList->SetGraphicsRootSignature(rootSignature.Get());
	cmdList->SetPipelineState(pipelineState.Get());

	// Vertex Buffer 및 Index Buffer 설정
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = vertexBuffer.GetVertexBufferView();
	cmdList->IASetVertexBuffers(0, 1, &vertexBufferView);

	D3D12_INDEX_BUFFER_VIEW indexBufferView = indexBuffer.GetIndexBufferView();
	cmdList->IASetIndexBuffer(&indexBufferView);

	// Primitive Topology 설정
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 삼각형 그리기
	cmdList->DrawIndexedInstanced(3, 1, 0, 0, 0);

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

	LOG_INFO("=== 09_HelloTriangle Sample Started ===");

	// 윈도우 생성
	Platform::WindowDesc windowDesc;
	windowDesc.title = "09_HelloTriangle - DevMiniEngine";
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
	Graphics::DX12VertexBuffer vertexBuffer;
	Graphics::DX12IndexBuffer indexBuffer;
	Graphics::DX12GraphicsRootSignature rootSignature;
	Graphics::DX12PipelineState pipelineState;
	Graphics::DX12ShaderCompiler shaderCompiler;

	// 삼각형 리소스 초기화
	if (!InitializeForTriangle(
		device,
		renderer,
		vertexBuffer,
		indexBuffer,
		rootSignature,
		pipelineState,
		shaderCompiler))
	{
		LOG_ERROR("Failed to initialize Triangle Resources");
		device.Shutdown();
		window->Destroy();
		return -1;
	}

	LOG_INFO("DirectX 12 initialization completed successfully!");
	LOG_INFO("Press ESC to exit");

	// 메인 루프
	while (!window->ShouldClose())
	{
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
			vertexBuffer,
			indexBuffer,
			rootSignature,
			pipelineState
		);

		input.Reset();
	}

	LOG_INFO("09_HelloTriangle - Terminated successfully");
	return 0;
}