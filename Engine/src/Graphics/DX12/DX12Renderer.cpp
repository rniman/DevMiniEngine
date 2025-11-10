#include "pch.h"
#include "Graphics/DX12/DX12Renderer.h"
#include "Graphics/DX12/DX12CommandContext.h"
#include "Graphics/DX12/DX12CommandQueue.h"
#include "Graphics/DX12/DX12ConstantBuffer.h"
#include "Graphics/DX12/DX12DepthStencilBuffer.h"
#include "Graphics/DX12/DX12DescriptorHeap.h"
#include "Graphics/DX12/DX12Device.h"
#include "Graphics/DX12/DX12PipelineStateCache.h"
#include "Graphics/DX12/DX12RootSignature.h"
#include "Graphics/DX12/DX12ShaderCompiler.h"
#include "Graphics/DX12/DX12SwapChain.h"
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"
#include "Graphics/Texture.h"
#include "Graphics/TextureType.h"
#include "Math/MathTypes.h"

namespace Graphics
{
	// MVP 상수 버퍼 구조체
	struct MVPConstants
	{
		Math::Matrix4x4 MVP;
	};

	DX12Renderer::DX12Renderer()
	{
	}

	DX12Renderer::~DX12Renderer()
	{
		Shutdown();
	}

	bool DX12Renderer::Initialize(
		DX12Device* device,
		Core::uint32 width,
		Core::uint32 height)
	{
		if (!device)
		{
			LOG_ERROR("DX12Renderer: Invalid device");
			return false;
		}

		mDevice = device;
		DX12SwapChain* swapChain = device->GetSwapChain();
		DX12CommandQueue* commandQueue = device->GetGraphicsQueue();

		if (!swapChain || !commandQueue)
		{
			LOG_ERROR("DX12Renderer: Device not properly initialized");
			return false;
		}

		mWidth = width;
		mHeight = height;

		// 1. Shader Compiler
		mShaderCompiler = std::make_unique<DX12ShaderCompiler>();

		// 2. Root Signature 생성
		if (!CreateDefaultRootSignature())
		{
			return false;
		}

		// 3. Pipeline State Cache
		mPipelineStateCache = std::make_unique<DX12PipelineStateCache>();
		if (!mPipelineStateCache->Initialize(device->GetDevice(), mShaderCompiler.get()))
		{
			return false;
		}

		// 4. Constant Buffers (MVP)
		mConstantBuffer = std::make_unique<DX12ConstantBuffer>();
		if (!mConstantBuffer->Initialize(
			device->GetDevice(),
			sizeof(Math::Matrix4x4),
			FRAME_BUFFER_COUNT))
		{
			return false;
		}

		// 5. Descriptor Heaps (텍스쳐 SRV)
		mSrvDescriptorHeap = std::make_unique<DX12DescriptorHeap>();
		if (!mSrvDescriptorHeap->Initialize(
			device->GetDevice(),
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			256,
			true))
		{
			return false;
		}

		// 6. Depth Buffer
		mDepthStencilBuffer = std::make_unique<DX12DepthStencilBuffer>();
		if (!mDepthStencilBuffer->Initialize(
			device->GetDevice(),
			width,
			height,
			DXGI_FORMAT_D24_UNORM_S8_UINT))
		{
			return false;
		}

		// 뷰포트와 시저 설정
		UpdateViewportAndScissor();

		mIsInitialized = true;
		LOG_INFO("DX12Renderer initialized (%dx%d)", mWidth, mHeight);
		return true;
	}

	bool DX12Renderer::CreateDefaultRootSignature()
	{
		// 기본 Root Signature 생성
		CD3DX12_ROOT_PARAMETER1 rootParameters[2]{};

		// CBV (b0) - MVP 행렬
		rootParameters[0].InitAsConstantBufferView(
			0,
			0,
			D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
			D3D12_SHADER_VISIBILITY_VERTEX);

		// SRV Table for Textures
		CD3DX12_DESCRIPTOR_RANGE1 srvRange;
		srvRange.Init(
			D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
			static_cast<UINT>(Graphics::TextureType::Count),
			0);

		rootParameters[1].InitAsDescriptorTable(
			1, &srvRange,
			D3D12_SHADER_VISIBILITY_PIXEL);

		// Sampler
		CD3DX12_STATIC_SAMPLER_DESC sampler(
			0,
			D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP);

		mRootSignature = std::make_unique<DX12RootSignature>();
		return mRootSignature->Initialize(
			mDevice->GetDevice(),
			2,
			rootParameters,
			1,
			&sampler,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	}

	void DX12Renderer::Shutdown()
	{
		if (!mIsInitialized)
		{
			return;
		}

		// GPU 작업 완료 대기
		if (mDevice->GetGraphicsQueue())
		{
			mDevice->GetGraphicsQueue()->WaitForIdle();
		}

		// 리소스 정리
		mSrvDescriptorHeap.reset();
		mDepthStencilBuffer.reset();
		mConstantBuffer.reset();
		mPipelineStateCache.reset();
		mShaderCompiler.reset();
		mRootSignature.reset();

		// 참조 초기화
		mDevice = nullptr;

		mIsInitialized = false;
		LOG_INFO("DX12Renderer shutdown");
	}

	void DX12Renderer::OnResize(Core::uint32 width, Core::uint32 height)
	{
		if (!mIsInitialized)
		{
			return;
		}

		mWidth = width;
		mHeight = height;

		// GPU 작업 완료 대기
		mDevice->GetGraphicsQueue()->WaitForIdle();

		// Depth Buffer 재생성
		mDepthStencilBuffer.reset();
		mDepthStencilBuffer = std::make_unique<DX12DepthStencilBuffer>();
		mDepthStencilBuffer->Initialize(
			mDevice->GetDevice(),
			mWidth,
			mHeight,
			DXGI_FORMAT_D24_UNORM_S8_UINT);

		// 뷰포트와 시저 업데이트
		UpdateViewportAndScissor();

		LOG_INFO("DX12Renderer resized to %dx%d", mWidth, mHeight);
	}

	void DX12Renderer::RenderFrame(const FrameData& frameData)
	{
		if (!mIsInitialized)
		{
			LOG_WARN("DX12Renderer not initialized");
			return;
		}

		// 필수 리소스 확인
		if (!mRootSignature || !mPipelineStateCache || !mConstantBuffer || !mSrvDescriptorHeap)
		{
			LOG_ERROR("DX12Renderer: Required resources not set");
			return;
		}

		// 프레임 시작
		if (!BeginFrame())
		{
			return;
		}

		// 화면 클리어
		Clear(mClearColor);

		// 파이프라인 설정
		SetupPipeline();

		// 렌더 아이템 그리기
		DrawRenderItems(frameData.opaqueItems);

		// 향후: 투명 오브젝트, UI, 디버그 렌더링 등

		// 프레임 종료
		EndFrame();

		// 화면 출력
		Present(true);  // VSync 활성화
	}

	bool DX12Renderer::BeginFrame()
	{
		// GPU가 현재 백 버퍼 사용을 완료할 때까지 대기
		mDevice->GetGraphicsQueue()->WaitForFenceValue(GetCurrentFrameFenceValue());

		// Command Context 리셋
		auto* cmdContext = GetCurrentCommandContext();
		if (!cmdContext)
		{
			LOG_ERROR("Failed to get Command Context");
			return false;
		}

		if (!cmdContext->Reset())
		{
			LOG_ERROR("Failed to reset Command Context");
			return false;
		}

		auto* cmdList = cmdContext->GetCommandList();
		auto* backBuffer = mDevice->GetSwapChain()->GetCurrentBackBuffer();

		// 리소스 전이: PRESENT - RENDER_TARGET
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer,
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		);
		cmdList->ResourceBarrier(1, &barrier);

		return true;
	}

	void DX12Renderer::Clear(const float* clearColor)
	{
		auto* cmdList = GetCurrentCommandList();
		auto* rtvHeap = mDevice->GetSwapChain()->GetRTVHeap();
		Core::uint32 backBufferIndex = static_cast<Core::uint32>(mDevice->GetSwapChain()->GetCurrentBackBufferIndex());

		// RTV와 DSV 핸들 가져오기
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUHandle(backBufferIndex);
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = mDepthStencilBuffer->GetDSVHandle();

		// 화면 클리어
		cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
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
	}

	void DX12Renderer::SetupPipeline()
	{
		auto* cmdList = GetCurrentCommandList();

		// 뷰포트 및 시저 설정
		cmdList->RSSetViewports(1, &mViewport);
		cmdList->RSSetScissorRects(1, &mScissorRect);

		// Root Signature 설정
		cmdList->SetGraphicsRootSignature(mRootSignature->GetRootSignature());

		// Descriptor Heap 설정
		ID3D12DescriptorHeap* heaps[] = { mSrvDescriptorHeap->GetHeap() };
		cmdList->SetDescriptorHeaps(1, heaps);

		// Primitive Topology 설정
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void DX12Renderer::DrawRenderItems(const std::vector<RenderItem>& items)
	{
		auto* cmdList = GetCurrentCommandList();

		for (const auto& item : items)
		{
			if (!item.material || !item.mesh)
			{
				LOG_WARN("Invalid RenderItem (material or mesh is null)");
				continue;
			}

			// PSO 가져오기/생성
			ID3D12PipelineState* pso = mPipelineStateCache->GetOrCreatePipelineState(
				*item.material,
				mRootSignature->GetRootSignature(),
				item.mesh->GetInputLayout()
			);

			if (!pso)
			{
				LOG_ERROR("Failed to get Pipeline State");
				continue;
			}

			cmdList->SetPipelineState(pso);

			// MVP 상수 버퍼 업데이트
			MVPConstants constants;
			constants.MVP = item.mvpMatrix;
			mConstantBuffer->Update(mCurrentFrameIndex, &constants, sizeof(MVPConstants));

			// CBV 설정 (Root Parameter 0)
			D3D12_GPU_VIRTUAL_ADDRESS cbvAddress = mConstantBuffer->GetGPUAddress(mCurrentFrameIndex);
			cmdList->SetGraphicsRootConstantBufferView(0, cbvAddress);

			// 텍스처 설정 (Material의 Descriptor Table 사용)
			// Material.AllocateDescriptors()에서 이미 SRV 생성 완료
			if (item.material->HasAllocatedDescriptors())
			{
				D3D12_GPU_DESCRIPTOR_HANDLE tableHandle = item.material->GetDescriptorTableHandle(mSrvDescriptorHeap.get());

				cmdList->SetGraphicsRootDescriptorTable(1, tableHandle);
			}
			else
			{
				LOG_WARN("Material has no allocated descriptors");
			}

			// 메시 그리기
			item.mesh->Draw(cmdList);
		}
	}

	void DX12Renderer::EndFrame()
	{
		auto* cmdContext = GetCurrentCommandContext();
		auto* cmdList = cmdContext->GetCommandList();
		auto* backBuffer = mDevice->GetSwapChain()->GetCurrentBackBuffer();

		// 리소스 전이: RENDER_TARGET - PRESENT
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
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
		Core::uint64 fenceValue = mDevice->GetGraphicsQueue()->ExecuteCommandLists(cmdLists, 1);
		SetCurrentFrameFenceValue(fenceValue);
	}

	void DX12Renderer::Present(bool vsync)
	{
		auto* swapChain = mDevice->GetSwapChain();

		// 화면에 표시
		swapChain->Present(vsync);

		// 다음 프레임으로 이동
		swapChain->MoveToNextFrame();
		MoveFrameIndex();
	}

	ID3D12GraphicsCommandList* DX12Renderer::GetCurrentCommandList()
	{
		auto* cmdContext = GetCurrentCommandContext();
		return cmdContext ? cmdContext->GetCommandList() : nullptr;
	}

	DX12CommandContext* DX12Renderer::GetCurrentCommandContext()
	{
		return mDevice->GetCommandContext(mCurrentFrameIndex);
	}

	void DX12Renderer::UpdateViewportAndScissor()
	{
		mViewport.TopLeftX = 0.0f;
		mViewport.TopLeftY = 0.0f;
		mViewport.Width = static_cast<float>(mWidth);
		mViewport.Height = static_cast<float>(mHeight);
		mViewport.MinDepth = 0.0f;
		mViewport.MaxDepth = 1.0f;

		mScissorRect.left = 0;
		mScissorRect.top = 0;
		mScissorRect.right = static_cast<LONG>(mWidth);
		mScissorRect.bottom = static_cast<LONG>(mHeight);
	}

} // namespace Graphics
