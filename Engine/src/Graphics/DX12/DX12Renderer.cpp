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
	//// MVP 상수 버퍼 구조체
	//struct MVPConstants
	//{
	//	Math::Matrix4x4 MVP;
	//};

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
		Core::uint32 height
	)
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

		mClearColor[0] = { 0.1f };
		mClearColor[1] = { 0.1f };
		mClearColor[2] = { 0.1f };
		mClearColor[3] = { 1.0f };

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

		// 4-1. MVP Constant Buffer (b0)
		mObjectConstantBuffer = std::make_unique<DX12ConstantBuffer>();
		if (!mObjectConstantBuffer->Initialize(
			device->GetDevice(),
			sizeof(ObjectConstants) * MAX_OBJECTS_PER_FRAME,
			FRAME_BUFFER_COUNT
		))
		{
			return false;
		}

		mMaterialConstantBuffer = std::make_unique<DX12ConstantBuffer>();
		if (!mMaterialConstantBuffer->Initialize(
			device->GetDevice(),
			sizeof(MaterialConstants),
			FRAME_BUFFER_COUNT
		))
		{
			return false;
		}

		constexpr size_t lightingBufferSize = sizeof(LightingConstants);

		//// 4-3. Lighting Constant Buffer (b2) - Phase 3.3
		//// DirectionalLight[4] + PointLight[8] + metadata
		//constexpr size_t dirLightSize = sizeof(Math::Vector4) + sizeof(Math::Vector3) + sizeof(Core::float32);  // 32 bytes

		//constexpr size_t pointLightSize =
		//	sizeof(Math::Vector4) +
		//	sizeof(Core::float32) +
		//	sizeof(Math::Vector3) +
		//	sizeof(Core::float32) +
		//	sizeof(Math::Vector3) +
		//	sizeof(Core::float32);  // 48 bytes (padding 포함)

		//constexpr size_t lightingBufferSize =
		//	(dirLightSize * 4) +      // DirectionalLight[4] = 128 bytes
		//	(pointLightSize * 8) +    // PointLight[8] = 384 bytes
		//	16;                       // numDirLights, numPointLights, viewPos, padding

		mLightingConstantBuffer = std::make_unique<DX12ConstantBuffer>();
		if (!mLightingConstantBuffer->Initialize(
			device->GetDevice(),
			lightingBufferSize,
			FRAME_BUFFER_COUNT
		))
		{
			return false;
		}

		// 5. Descriptor Heaps (텍스쳐 SRV)
		mSrvDescriptorHeap = std::make_unique<DX12DescriptorHeap>();
		if (!mSrvDescriptorHeap->Initialize(
			device->GetDevice(),
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			256,
			true
		))
		{
			return false;
		}

		// 6. Depth Buffer
		mDepthStencilBuffer = std::make_unique<DX12DepthStencilBuffer>();
		if (!mDepthStencilBuffer->Initialize(
			device->GetDevice(),
			width,
			height,
			DXGI_FORMAT_D24_UNORM_S8_UINT
		))
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
		// Phase 3.3: 3개의 CBV + SRV Table + Sampler
		CD3DX12_ROOT_PARAMETER1 rootParameters[4]{};

		// CBV (b0) - Object Constants (worldMatrix, mvpMatrix)
		rootParameters[0].InitAsConstantBufferView(
			0,
			0,
			D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
			D3D12_SHADER_VISIBILITY_VERTEX
		);

		// CBV (b1) - Material Constants
		rootParameters[1].InitAsConstantBufferView(
			1,
			0,
			D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
			D3D12_SHADER_VISIBILITY_PIXEL
		);

		// CBV (b2) - Lighting Data
		rootParameters[2].InitAsConstantBufferView(
			2,
			0,
			D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
			D3D12_SHADER_VISIBILITY_PIXEL
		);

		// SRV Table for Textures (t0~t6)
		CD3DX12_DESCRIPTOR_RANGE1 srvRange;
		srvRange.Init(
			D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
			static_cast<UINT>(Graphics::TextureType::Count),
			0
		);

		rootParameters[3].InitAsDescriptorTable(
			1, &srvRange,
			D3D12_SHADER_VISIBILITY_PIXEL
		);

		// Sampler
		CD3DX12_STATIC_SAMPLER_DESC sampler(
			0,
			D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP
		);

		mRootSignature = std::make_unique<DX12RootSignature>();
		return mRootSignature->Initialize(
			mDevice->GetDevice(),
			4,  // 4개의 Root Parameters
			rootParameters,
			1,
			&sampler,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		);
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

		// Phase 3.3: Constant Buffers 정리
		mLightingConstantBuffer.reset();
		mMaterialConstantBuffer.reset();
		mObjectConstantBuffer.reset();

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
		bool cb = !mObjectConstantBuffer || !mMaterialConstantBuffer || !mLightingConstantBuffer;

		if (!mRootSignature || !mPipelineStateCache || cb || !mSrvDescriptorHeap)
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

		// Phase 3.3: Lighting Constant Buffer 업데이트
		UpdateLightingBuffer(frameData);

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

		mCurrentObjectCBIndex = 0;

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
		if (items.empty())
		{
			return;
		}

		auto* cmdList = GetCurrentCommandList();
		if (!cmdList)
		{
			return;
		}

		LOG_DEBUG("[DX12Renderer] Drawing %zu items", items.size());

		// Root Signature 설정
		cmdList->SetGraphicsRootSignature(mRootSignature->GetRootSignature());

		// Descriptor Heap 설정
		ID3D12DescriptorHeap* heaps[] = { mSrvDescriptorHeap->GetHeap() };
		cmdList->SetDescriptorHeaps(1, heaps);

		// 각 렌더 아이템 그리기
		for (const auto& item : items)
		{
			if (!item.mesh || !item.material)
			{
				continue;
			}

			// 1. Pipeline State Object 설정
			auto pso = mPipelineStateCache->GetOrCreatePipelineState(
				*item.material,
				mRootSignature->GetRootSignature(),
				item.mesh->GetInputLayout()
			);

			if (!pso)
			{
				LOG_WARN("Failed to get PSO for material");
				continue;
			}

			cmdList->SetPipelineState(pso);

			ObjectConstants objectData;
			objectData.worldMatrix = Math::MatrixTranspose(item.worldMatrix);
			objectData.mvpMatrix = item.mvpMatrix;

			constexpr size_t ALIGNED_OBJECT_SIZE = 256;  // 256바이트 정렬

			mObjectConstantBuffer->UpdateAtOffset(
				mCurrentFrameIndex,      // 프레임 인덱스
				mCurrentObjectCBIndex,   // 오브젝트 슬롯 인덱스
				&objectData,             // 데이터
				sizeof(ObjectConstants), // 크기
				ALIGNED_OBJECT_SIZE      // 슬롯 크기
			);

			// GPU 주소 계산
			D3D12_GPU_VIRTUAL_ADDRESS baseAddress = mObjectConstantBuffer->GetGPUAddress(mCurrentFrameIndex);
			D3D12_GPU_VIRTUAL_ADDRESS objectCbvAddress = baseAddress + (ALIGNED_OBJECT_SIZE * mCurrentObjectCBIndex);

			cmdList->SetGraphicsRootConstantBufferView(0, objectCbvAddress);

			MaterialConstants materialData;
			materialData.baseColor = Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f);  // 기본 흰색
			materialData.metallic = 0.0f;
			materialData.roughness = 0.5f;
			materialData.padding = Math::Vector2(0.0f, 0.0f);

			mMaterialConstantBuffer->Update(mCurrentFrameIndex, &materialData, sizeof(MaterialConstants));

			D3D12_GPU_VIRTUAL_ADDRESS materialCbvAddress = mMaterialConstantBuffer->GetGPUAddress(mCurrentFrameIndex);
			cmdList->SetGraphicsRootConstantBufferView(1, materialCbvAddress);

			// 4. Lighting Constants 설정 (b2) - Phase 3.3
			// UpdateLightingBuffer()에서 이미 업데이트됨
			D3D12_GPU_VIRTUAL_ADDRESS lightingCbvAddress = mLightingConstantBuffer->GetGPUAddress(mCurrentFrameIndex);
			cmdList->SetGraphicsRootConstantBufferView(2, lightingCbvAddress);

			// 5. 텍스처 설정 (Root Parameter 3)
			if (item.material->HasAllocatedDescriptors())
			{
				D3D12_GPU_DESCRIPTOR_HANDLE tableHandle = item.material->GetDescriptorTableHandle(mSrvDescriptorHeap.get());
				cmdList->SetGraphicsRootDescriptorTable(3, tableHandle);
			}
			else
			{
				LOG_WARN("Material has no allocated descriptors");
			}

			// 6. 메시 그리기
			item.mesh->Draw(cmdList);

			++mCurrentObjectCBIndex;
		}
	}

	void DX12Renderer::UpdateLightingBuffer(const FrameData& frameData)
	{

		LightingConstants lightingData = {};

		// 1. Directional Lights 복사
		lightingData.numDirLights = static_cast<Core::uint32>(
			std::min(frameData.directionalLights.size(), static_cast<size_t>(4)));
		for (Core::uint32 i = 0; i < lightingData.numDirLights; ++i)
		{
			const auto& src = frameData.directionalLights[i];
			auto& dst = lightingData.dirLights[i];

			dst.direction = src.direction;
			dst.color = src.color;
			dst.intensity = src.intensity;
		}

		// 2. Point Lights 복사
		lightingData.numPointLights = static_cast<Core::uint32>(
			std::min(frameData.pointLights.size(), static_cast<size_t>(8))
			);

		for (Core::uint32 i = 0; i < lightingData.numPointLights; ++i)
		{
			const auto& src = frameData.pointLights[i];
			auto& dst = lightingData.pointLights[i];

			dst.position = src.position;
			dst.rangeAndColor = src.rangeAndColor;
			dst.intensityAndAttenuation = src.intensityAndAttenuation;
		}

		// 3. Camera Position
		lightingData.viewPos = frameData.cameraPosition;

		// 4. GPU로 업데이트
		mLightingConstantBuffer->Update(mCurrentFrameIndex, &lightingData, sizeof(LightingConstants));
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
