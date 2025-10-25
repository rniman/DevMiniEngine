// Graphics/src/DX12/DX12PipelineState.cpp
#include "Graphics/DX12/DX12PipelineState.h"
#include "Core/Logging/LogMacros.h"

namespace Graphics
{
	bool DX12PipelineState::Initialize(
		ID3D12Device* device,
		const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc
	)
	{
		CORE_ASSERT(device != nullptr, "Device는 null일 수 없습니다");

		HRESULT hr = device->CreateGraphicsPipelineState(
			&desc,
			IID_PPV_ARGS(&mPipelineState)
		);

		if (FAILED(hr))
		{
			LOG_ERROR("Failed to create Graphics Pipeline State: 0x%08X", hr);
			return false;
		}

		LOG_INFO("Graphics Pipeline State created successfully");
		return true;
	}

	void DX12PipelineState::Shutdown()
	{
		mPipelineState.Reset();
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC DX12PipelineState::CreateDefaultDesc(
		ID3D12RootSignature* rootSignature,
		ID3DBlob* vs,
		ID3DBlob* ps,
		const D3D12_INPUT_LAYOUT_DESC& inputLayout
	)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

		// Shader
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vs);
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(ps);

		// Root Signature & Input Layout
		psoDesc.pRootSignature = rootSignature;
		psoDesc.InputLayout = inputLayout;

		// Rasterizer: Solid fill, back-face culling
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

		// Blend: No blending (opaque)
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

		// Depth Stencil: Disabled (2D rendering)
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = FALSE;

		// Render Target
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;

		// Primitive Topology
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		// MSAA
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		psoDesc.SampleMask = UINT_MAX;

		return psoDesc;
	}
} // namespace Graphics