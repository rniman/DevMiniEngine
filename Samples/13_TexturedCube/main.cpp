#include "Platform/Window.h"
#include "Platform/PlatformTypes.h"
#include "Platform/Input.h"
#include "Core/Logging/Logger.h"
#include "Core/Logging/ConsoleSink.h"
#include "Core/Logging/LogMacros.h"

#include "Math/MathUtils.h"
 
// Graphics headers
#include <d3d12.h> 
#include <dxgi1_6.h>

#pragma warning(push, 0)
#include "d3dx12.h"
#pragma warning(pop)

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

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
#include "Graphics/DX12/DX12DepthStencilBuffer.h"
#include "Graphics/Camera/PerspectiveCamera.h"
#include "Graphics/Mesh.h"
#include "Graphics/Material.h"
#include "Graphics/Texture.h"

constexpr Core::uint32 FRAME_WIDTH = 16 * 80;
constexpr Core::uint32 FRAME_HEIGHT = 9 * 80;

using namespace Microsoft::WRL;

struct MVPConstants
{
	Math::Matrix4x4 MVP;
};


// ============================================================================
// 함수 선언
// ============================================================================

bool InitializeMaterialTextures(
	Graphics::DX12Device& device,
	Graphics::DX12Renderer& renderer,
	Graphics::DX12DescriptorHeap& srvDescriptorHeap,
	Graphics::Material& material
)
{
	// SRV용 Descriptor Heap 생성
	if (!srvDescriptorHeap.Initialize(
		device.GetDevice(),
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		256,
		true
	))
	{
		LOG_ERROR("Failed to initialize SRV Descriptor Heap");
		return false;
	}

	auto diffuseTex = std::make_shared<Graphics::Texture>();
	diffuseTex->LoadFromFile(
		device.GetDevice(),
		device.GetGraphicsQueue(),
		device.GetCommandContext(renderer.GetCurrentFrameIndex()),
		L"../../Assets/Textures/BrickWall17_1K_BaseColor.png"
	);

	auto normalTex = std::make_shared<Graphics::Texture>();
	normalTex->LoadFromFile(
		device.GetDevice(),
		device.GetGraphicsQueue(),
		device.GetCommandContext(renderer.GetCurrentFrameIndex()),
		L"../../Assets/Textures/BrickWall17_1K_Normal.png"
	);

	material.SetTexture(Graphics::TextureType::Diffuse, diffuseTex);
	material.SetTexture(Graphics::TextureType::Normal, normalTex);

	// Descriptor 할당
	if (!material.AllocateDescriptors(device.GetDevice(), &srvDescriptorHeap))
	{
		LOG_ERROR("Failed to allocate material descriptors");
		return false;
	}

	LOG_INFO("Texture initialized successfully");
	return true;
}


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

bool InitializeForCube(
	Graphics::DX12Device& device,
	Graphics::DX12Renderer& renderer,
	Graphics::Mesh& mesh,
	Graphics::DX12RootSignature& rootSignature,
	Graphics::DX12ShaderCompiler& shaderCompiler,
	Graphics::Material& material,
	Graphics::DX12PipelineStateCache& pipelineStateCache,
	Graphics::DX12ConstantBuffer& constantBuffer,
	Graphics::DX12DepthStencilBuffer& depthStencilBuffer,
	Graphics::DX12DescriptorHeap& srvDescriptorHeap
)
{
	LOG_INFO("Initializing Cube Resources...");

	Graphics::TexturedVertex cubeVertices[] =
	{
		// 1. Front face (앞면)
		{ Math::Vector3(-2.0f,  2.0f, -2.0f), Math::Vector2(0.0f, 0.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 0
		{ Math::Vector3(2.0f,  2.0f, -2.0f), Math::Vector2(1.0f, 0.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 1
		{ Math::Vector3(-2.0f, -2.0f, -2.0f), Math::Vector2(0.0f, 1.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 2
		{ Math::Vector3(2.0f, -2.0f, -2.0f), Math::Vector2(1.0f, 1.0f), Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 3

		// 2. Back face (뒷면)
		// (UV좌표를 0,0 1,0 0,1 1,1로 동일하게 가져가되, 
		//  뒷면에서 볼 때 올바른 방향이 되도록 정점 순서를 조정)
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

	Core::uint16 cubeIndices[] =
	{
		// 1. Front face
		0, 1, 2,  // 0, 1, 2
		1, 3, 2,  // 1, 3, 2

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

	// Mesh 초기화
	if (!mesh.InitializeTextured(
		device.GetDevice(),
		device.GetGraphicsQueue(),
		device.GetCommandContext(renderer.GetCurrentFrameIndex()),
		cubeVertices,
		24,
		cubeIndices,
		36
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

	depthStencilBuffer.Initialize(
		device.GetDevice(),
		FRAME_WIDTH,
		FRAME_HEIGHT,
		DXGI_FORMAT_D24_UNORM_S8_UINT
	);

	// Root Signature 생성 (CBV 포함)
	// Root Parameter 정의: CBV 1개
	CD3DX12_ROOT_PARAMETER1 rootParameters[2];
	// CBV (b0)
	rootParameters[0].InitAsConstantBufferView(
		0,
		0,
		D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
		D3D12_SHADER_VISIBILITY_VERTEX
	);

	// SRV Descriptor Table (t0)
	CD3DX12_DESCRIPTOR_RANGE1 srvRange;
	srvRange.Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		static_cast<UINT>(Graphics::TextureType::Count),
		0
	);  // t0 ~ t6
	rootParameters[1].InitAsDescriptorTable(
		1,
		&srvRange,
		D3D12_SHADER_VISIBILITY_PIXEL
	);

	// Static Sampler 추가
	CD3DX12_STATIC_SAMPLER_DESC sampler(
		0,  // s0
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP
	);

	// Root Signature 초기화
	if (!rootSignature.Initialize(
		device.GetDevice(),
		2,                                              // numParameters
		rootParameters,                                 // parameters
		1,                                              // numStaticSamplers
		&sampler,                                       // staticSamplers
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

	InitializeMaterialTextures(device, renderer, srvDescriptorHeap, material);

	LOG_INFO("Camera Resources initialization completed successfully");
	return true;
}

/**
 * @brief MVP 행렬을 업데이트합니다.
 */
void UpdateMVP(
	Graphics::DX12ConstantBuffer& constantBuffer,
	Core::uint32 frameIndex,
	Core::float32 timeInSeconds,
	const Graphics::PerspectiveCamera& camera
)
{
	// Model 행렬 (Y축 회전)
	Core::float32 rotationAngle = timeInSeconds * Math::DegToRad(90.0);
	auto a = Math::MatrixRotationY(rotationAngle);
	Math::Matrix4x4 model = Math::MatrixMultiply(Math::MatrixIdentity(), a);
	// View 행렬
	Math::Matrix4x4 view = camera.GetViewMatrix();
	// Projection 행렬
	Math::Matrix4x4 projection = camera.GetProjectionMatrix();
	// MVP 계산 (Model * View * Projection)
	Math::Matrix4x4 mvp = Math::MatrixMultiply(model, view);
	mvp = Math::MatrixMultiply(mvp, projection);

	MVPConstants constants;
	constants.MVP = Math::MatrixTranspose(mvp);

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
	Core::float32 timeInSeconds,
	Graphics::PerspectiveCamera& camera,
	Graphics::DX12DepthStencilBuffer& depthStencilBuffer,
	Graphics::DX12DescriptorHeap& srvDescriptorHeap
)
{
	auto* swapChain = device.GetSwapChain();
	auto* graphicsQueue = device.GetGraphicsQueue();
	auto* rtvHeap = swapChain->GetRTVHeap();
	Core::uint32 backBufferIndex = static_cast<Core::uint32>(swapChain->GetCurrentBackBufferIndex());
	Core::uint32 frameIndex = renderer.GetCurrentFrameIndex();

	// GPU가 현재 백 버퍼 사용을 완료할 때까지 대기
	graphicsQueue->WaitForFenceValue(renderer.GetCurrentFrameFenceValue());

	camera.UpdateViewMatrix();
	camera.UpdateProjectionMatrix();

	UpdateMVP(constantBuffer, frameIndex, timeInSeconds, camera);

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

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = depthStencilBuffer.GetDSVHandle();

	cmdList->ClearDepthStencilView(
		dsvHandle,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f,  // Depth
		0,     // Stencil
		0,
		nullptr
	);

	// Render Target 설정
	cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

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

	ID3D12DescriptorHeap* heaps[] = { srvDescriptorHeap.GetHeap() };
	cmdList->SetDescriptorHeaps(1, heaps);

	cmdList->SetGraphicsRootDescriptorTable(
		1,
		material.GetDescriptorTableHandle(&srvDescriptorHeap)
	);


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

	LOG_INFO("=== 13_TexturedCube ===");

	// 윈도우 생성
	Platform::WindowDesc windowDesc;
	windowDesc.title = "12_CameraRendering - DevMiniEngine";
	windowDesc.width = FRAME_WIDTH;
	windowDesc.height = FRAME_HEIGHT;
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
	Graphics::DX12PipelineStateCache DX12PipelineStateCache;
	Graphics::DX12ConstantBuffer constantBuffer;
	Graphics::DX12DepthStencilBuffer depthStencilBuffer;
	Graphics::DX12DescriptorHeap srvDescriptorHeap;

	Graphics::MaterialDesc materialDesc;
	materialDesc.vertexShaderPath = L"TexturedShader.hlsl";
	materialDesc.pixelShaderPath = L"TexturedShader.hlsl";

	Graphics::Material material(materialDesc);

	if (!InitializeForCube(
		device,
		renderer,
		mesh,
		rootSignature,
		shaderCompiler,
		material,
		DX12PipelineStateCache,
		constantBuffer,
		depthStencilBuffer,
		srvDescriptorHeap
	))
	{
		LOG_ERROR("Failed to initialize Cube Resources");

		DX12PipelineStateCache.Shutdown();
		mesh.Shutdown();
		rootSignature.Shutdown();

		device.Shutdown();
		window->Destroy();
		return -1;
	}

	Graphics::PerspectiveCamera camera;
	camera.SetLookAt(
		Math::Vector3(0.0f, 10.0f, -20.0f),  // 카메라 위치 (뒤로 3 유닛)
		Math::Vector3(0.0f, 0.0f, 0.0f),   // 타겟 (원점)
		Math::Vector3(0.0f, 1.0f, 0.0f)    // Up 벡터
	);

	LOG_INFO("DirectX 12 initialization completed successfully!");
	LOG_INFO("Press ESC to exit");

	Core::uint64 startTicks = GetTickCount64();

	// 메인 루프
	while (!window->ShouldClose())
	{
		Core::uint64 currentTicks = GetTickCount64();
		Core::float32 timeInSeconds = (currentTicks - startTicks) / 1000.0f;

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
			timeInSeconds,
			camera,
			depthStencilBuffer,
			srvDescriptorHeap
		);

		input.Reset();
	}

	LOG_INFO("12_CameraRendering  - Terminated successfully");
	return 0;
}
