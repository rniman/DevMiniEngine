#include "pch.h"
#include "Graphics/DebugDraw/DebugRenderer.h"
#include "Graphics/DebugDraw/DebugShapes.h"
#include "Graphics/DX12/DX12ConstantBuffer.h"
#include "Graphics/DX12/DX12Device.h"
#include "Graphics/DX12/DX12ShaderCompiler.h"
#include "Graphics/DX12/DX12SwapChain.h"
#include "Graphics/DX12/DX12VertexBuffer.h"
#include "Graphics/DX12/DX12CommandQueue.h"
#include "Graphics/DX12/DX12CommandContext.h"
#include "Graphics/RenderTypes.h"
#include "Graphics/VertexTypes.h"
#include "Math/MathUtils.h"

namespace Graphics
{
	DebugRenderer::~DebugRenderer()
	{
		Shutdown();
	}

	//=========================================================================
	// 생명주기
	//=========================================================================

	bool DebugRenderer::Initialize(DX12Device* device, DX12ShaderCompiler* shaderCompiler)
	{
		if (mIsInitialized)
		{
			LOG_WARN("[DebugRenderer] Already initialized.");
			return true;
		}

		if (!device || !shaderCompiler)
		{
			LOG_ERROR("[DebugRenderer] Invalid device or shader compiler.");
			return false;
		}

		mDevice = device;
		mShaderCompiler = shaderCompiler;

		LOG_INFO("[DebugRenderer] Initializing...");

		if (!CreateRootSignature()) return false;
		if (!CreatePipelineStates()) return false;
		if (!CreateConstantBuffer()) return false;
		if (!CreateUnitShapes()) return false;

		mIsInitialized = true;
		LOG_INFO("[DebugRenderer] Initialized successfully.");
		return true;
	}

	void DebugRenderer::Shutdown()
	{
		if (!mIsInitialized)
		{
			return;
		}

		LOG_INFO("[DebugRenderer] Shutting down...");

		mArrowBuffer.reset();
		mSphereBuffer.reset();
		mConstantBuffer.reset();

		mPSODepthOn.Reset();
		mPSODepthOff.Reset();
		mRootSignature.Reset();

		mHiddenEntities.clear();
		mSelectedEntity = ECS::Entity{};

		mDevice = nullptr;
		mShaderCompiler = nullptr;
		mIsInitialized = false;

		LOG_INFO("[DebugRenderer] Shutdown complete.");
	}

	//=========================================================================
	// 초기화
	//=========================================================================

	bool DebugRenderer::CreateRootSignature()
	{
		// CBV 1개: MVP + Color (b0, Vertex Shader)
		CD3DX12_ROOT_PARAMETER1 rootParameters[1]{};
		rootParameters[0].InitAsConstantBufferView(
			0, 0,
			D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
			D3D12_SHADER_VISIBILITY_VERTEX
		);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
		rootSigDesc.Init_1_1(
			1, rootParameters,
			0, nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		);

		ComPtr<ID3DBlob> signatureBlob;
		ComPtr<ID3DBlob> errorBlob;

		HRESULT hr = D3DX12SerializeVersionedRootSignature(
			&rootSigDesc,
			D3D_ROOT_SIGNATURE_VERSION_1_1,
			&signatureBlob,
			&errorBlob
		);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				LOG_ERROR("[DebugRenderer] Root Signature error: %s",
					static_cast<const char*>(errorBlob->GetBufferPointer()));
			}
			return false;
		}

		hr = mDevice->GetDevice()->CreateRootSignature(
			0,
			signatureBlob->GetBufferPointer(),
			signatureBlob->GetBufferSize(),
			IID_PPV_ARGS(&mRootSignature)
		);

		if (FAILED(hr))
		{
			LOG_ERROR("[DebugRenderer] CreateRootSignature failed. HR: 0x%08X", hr);
			return false;
		}

		return true;
	}

	bool DebugRenderer::CreatePipelineStates()
	{
		// 셰이더 컴파일
		ComPtr<ID3DBlob> vsBlob;
		ComPtr<ID3DBlob> psBlob;

		if (!mShaderCompiler->CompileFromFile(
			L"../../Assets/Shaders/DebugVS.hlsl", "VSMain", "vs_5_1", vsBlob.GetAddressOf()))
		{
			LOG_ERROR("[DebugRenderer] Failed to compile DebugVS.hlsl");
			return false;
		}

		if (!mShaderCompiler->CompileFromFile(
			L"../../Assets/Shaders/DebugPS.hlsl", "PSMain", "ps_5_1", psBlob.GetAddressOf()))
		{
			LOG_ERROR("[DebugRenderer] Failed to compile DebugPS.hlsl");
			return false;
		}

		// PSO 공통 설정
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.pRootSignature = mRootSignature.Get();
		psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
		psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };
		psoDesc.InputLayout = DebugVertex::GetInputLayout();
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

		// Depth On: 물체에 가려짐
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = TRUE;
		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

		HRESULT hr = mDevice->GetDevice()->CreateGraphicsPipelineState(
			&psoDesc, IID_PPV_ARGS(&mPSODepthOn));
		if (FAILED(hr))
		{
			LOG_ERROR("[DebugRenderer] CreatePSO (DepthOn) failed. HR: 0x%08X", hr);
			return false;
		}

		// Depth Off: 항상 위에 표시 (X-Ray)
		psoDesc.DepthStencilState.DepthEnable = FALSE;

		hr = mDevice->GetDevice()->CreateGraphicsPipelineState(
			&psoDesc, IID_PPV_ARGS(&mPSODepthOff));
		if (FAILED(hr))
		{
			LOG_ERROR("[DebugRenderer] CreatePSO (DepthOff) failed. HR: 0x%08X", hr);
			return false;
		}

		return true;
	}

	bool DebugRenderer::CreateConstantBuffer()
	{
		mConstantBuffer = std::make_unique<DX12ConstantBuffer>();

		if (!mConstantBuffer->Initialize(
			mDevice->GetDevice(),
			sizeof(DebugConstants) * MAX_DEBUG_DRAWS_PER_FRAME,
			FRAME_BUFFER_COUNT))
		{
			LOG_ERROR("[DebugRenderer] CreateConstantBuffer failed.");
			return false;
		}

		return true;
	}

	bool DebugRenderer::CreateUnitShapes()
	{
		auto* commandQueue = mDevice->GetCommandQueue();
		auto* commandContext = mDevice->GetCommandContext(0);

		// Arrow (Directional Light용)
		std::vector<DebugVertex> arrowVertices;
		DebugShapes::GenerateArrow(arrowVertices);

		mArrowBuffer = std::make_unique<DX12VertexBuffer>();
		if (!mArrowBuffer->Initialize(
			mDevice->GetDevice(), commandQueue, commandContext,
			arrowVertices.data(), arrowVertices.size(), sizeof(DebugVertex)))
		{
			LOG_ERROR("[DebugRenderer] Failed to create Arrow buffer.");
			return false;
		}

		// Sphere (Point Light용)
		std::vector<DebugVertex> sphereVertices;
		DebugShapes::GenerateSphereWireframe(sphereVertices, DebugShapes::DEFAULT_CIRCLE_SEGMENTS);

		mSphereBuffer = std::make_unique<DX12VertexBuffer>();
		if (!mSphereBuffer->Initialize(
			mDevice->GetDevice(), commandQueue, commandContext,
			sphereVertices.data(), sphereVertices.size(), sizeof(DebugVertex)))
		{
			LOG_ERROR("[DebugRenderer] Failed to create Sphere buffer.");
			return false;
		}

		LOG_INFO("[DebugRenderer] Unit shapes created. Arrow: %zu verts, Sphere: %zu verts",
			mArrowBuffer->GetVertexCount(), mSphereBuffer->GetVertexCount());

		return true;
	}

	//=========================================================================
	// 렌더링
	//=========================================================================

	void DebugRenderer::Render(
		ID3D12GraphicsCommandList* cmdList,
		const FrameData& frameData,
		const Math::Matrix4x4& viewMatrix,
		const Math::Matrix4x4& projMatrix)
	{
		if (!mIsInitialized || mSettings.displayMode == DebugDisplayMode::None)
		{
			return;
		}

		// 프레임 상태 초기화
		mCurrentFrameIndex = mDevice->GetSwapChain()->GetCurrentBackBufferIndex();
		mCurrentDrawIndex = 0;

		// PSO 및 공통 상태 설정
		ID3D12PipelineState* pso = mSettings.depthTestEnabled ? mPSODepthOn.Get() : mPSODepthOff.Get();
		cmdList->SetPipelineState(pso);
		cmdList->SetGraphicsRootSignature(mRootSignature.Get());
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

		Math::Matrix4x4 viewProj = viewMatrix * projMatrix;

		// 렌더링 조건 결정
		bool isAllMode = (mSettings.displayMode == DebugDisplayMode::All);
		bool renderDirLights = isAllMode ? mSettings.showDirectionalLights : true;
		bool renderPointLights = isAllMode ? mSettings.showPointLights : true;

		if (renderDirLights)
		{
			RenderDirectionalLights(cmdList, frameData, viewProj);
		}

		if (renderPointLights)
		{
			RenderPointLights(cmdList, frameData, viewProj);
		}
	}

	void DebugRenderer::RenderDirectionalLights(
		ID3D12GraphicsCommandList* cmdList,
		const FrameData& frameData,
		const Math::Matrix4x4& viewProj)
	{
		static const Math::Vector3 ARROW_FORWARD = Math::Vector3::Forward();

		for (size_t i = 0; i < frameData.directionalLights.size(); ++i)
		{
			// Entity 확인
			bool hasEntity = (i < frameData.debug.directionalLightEntities.size());
			ECS::Entity entity = hasEntity ? frameData.debug.directionalLightEntities[i] : ECS::Entity{};

			// 필터링
			if (mSettings.displayMode == DebugDisplayMode::SelectedOnly)
			{
				if (!hasEntity || entity != mSelectedEntity) continue;
			}
			if (hasEntity && mHiddenEntities.count(entity) > 0) continue;

			// Transform 계산
			const auto& light = frameData.directionalLights[i];
			Math::Vector3 dir = Math::Vector3(light.direction.x, light.direction.y, light.direction.z).Normalized();
			Math::Quaternion rotation = Math::QuaternionFromToRotation(ARROW_FORWARD, dir);
			Math::Matrix4x4 worldMatrix = Math::MatrixSRT(Math::Vector3(2.0f), rotation, Math::Vector3::Zero());

			// 색상 결정
			Math::Vector3 color = (hasEntity && entity == mSelectedEntity) ?
				mSettings.selectedColor : mSettings.directionalLightColor;

			DrawArrow(cmdList, worldMatrix, viewProj, color);
		}
	}

	void DebugRenderer::RenderPointLights(
		ID3D12GraphicsCommandList* cmdList,
		const FrameData& frameData,
		const Math::Matrix4x4& viewProj)
	{
		for (size_t i = 0; i < frameData.pointLights.size(); ++i)
		{
			// Entity 확인
			bool hasEntity = (i < frameData.debug.pointLightEntities.size());
			ECS::Entity entity = hasEntity ? frameData.debug.pointLightEntities[i] : ECS::Entity{};

			// 필터링
			if (mSettings.displayMode == DebugDisplayMode::SelectedOnly)
			{
				if (!hasEntity || entity != mSelectedEntity) continue;
			}
			if (hasEntity && mHiddenEntities.count(entity) > 0) continue;

			// Transform 계산 (GPU 데이터에서 추출)
			const auto& light = frameData.pointLights[i];
			Math::Vector3 position(light.position.x, light.position.y, light.position.z);
			Core::float32 range = light.rangeAndColor.x;
			Math::Matrix4x4 worldMatrix = Math::MatrixSRT(Math::Vector3(range), Math::Quaternion::Identity(), position);

			// 색상 결정
			Math::Vector3 color = (hasEntity && entity == mSelectedEntity) ?
				mSettings.selectedColor : mSettings.pointLightColor;

			DrawSphere(cmdList, worldMatrix, viewProj, color);
		}
	}

	void DebugRenderer::DrawArrow(
		ID3D12GraphicsCommandList* cmdList,
		const Math::Matrix4x4& worldMatrix,
		const Math::Matrix4x4& viewProj,
		const Math::Vector3& color)
	{
		if (mCurrentDrawIndex >= MAX_DEBUG_DRAWS_PER_FRAME)
		{
			return;
		}

		// Constant Buffer 업데이트
		DebugConstants constants;
		constants.mvpMatrix = Math::MatrixTranspose(worldMatrix * viewProj);
		constants.color = color;

		mConstantBuffer->UpdateAtOffset(
			mCurrentFrameIndex, mCurrentDrawIndex,
			&constants, sizeof(DebugConstants), ALIGNED_CB_SIZE);

		// 바인딩 및 Draw
		D3D12_GPU_VIRTUAL_ADDRESS cbvAddress =
			mConstantBuffer->GetGPUAddress(mCurrentFrameIndex) + (ALIGNED_CB_SIZE * mCurrentDrawIndex);
		cmdList->SetGraphicsRootConstantBufferView(0, cbvAddress);

		D3D12_VERTEX_BUFFER_VIEW vbv = mArrowBuffer->GetVertexBufferView();
		cmdList->IASetVertexBuffers(0, 1, &vbv);
		cmdList->DrawInstanced(static_cast<UINT>(mArrowBuffer->GetVertexCount()), 1, 0, 0);

		++mCurrentDrawIndex;
	}

	void DebugRenderer::DrawSphere(
		ID3D12GraphicsCommandList* cmdList,
		const Math::Matrix4x4& worldMatrix,
		const Math::Matrix4x4& viewProj,
		const Math::Vector3& color)
	{
		if (mCurrentDrawIndex >= MAX_DEBUG_DRAWS_PER_FRAME)
		{
			return;
		}

		// Constant Buffer 업데이트
		DebugConstants constants;
		constants.mvpMatrix = Math::MatrixTranspose(worldMatrix * viewProj);
		constants.color = color;

		mConstantBuffer->UpdateAtOffset(
			mCurrentFrameIndex, mCurrentDrawIndex,
			&constants, sizeof(DebugConstants), ALIGNED_CB_SIZE);

		// 바인딩 및 Draw
		D3D12_GPU_VIRTUAL_ADDRESS cbvAddress =
			mConstantBuffer->GetGPUAddress(mCurrentFrameIndex) + (ALIGNED_CB_SIZE * mCurrentDrawIndex);
		cmdList->SetGraphicsRootConstantBufferView(0, cbvAddress);

		D3D12_VERTEX_BUFFER_VIEW vbv = mSphereBuffer->GetVertexBufferView();
		cmdList->IASetVertexBuffers(0, 1, &vbv);
		cmdList->DrawInstanced(static_cast<UINT>(mSphereBuffer->GetVertexCount()), 1, 0, 0);

		++mCurrentDrawIndex;
	}

	//=========================================================================
	// Entity 가시성
	//=========================================================================

	void DebugRenderer::SetEntityVisible(ECS::Entity entity, bool visible)
	{
		if (visible)
		{
			mHiddenEntities.erase(entity);
		}
		else
		{
			mHiddenEntities.insert(entity);
		}
	}

	bool DebugRenderer::IsEntityVisible(ECS::Entity entity) const
	{
		return mHiddenEntities.find(entity) == mHiddenEntities.end();
	}

} // namespace Graphics
