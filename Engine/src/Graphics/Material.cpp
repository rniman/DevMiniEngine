#include "pch.h"
#include "Graphics/Material.h"
#include "Core/Logging/Logger.h"

using namespace std;

namespace Graphics
{

	Material::Material(const MaterialDesc& desc)
		: mVertexShaderPath(desc.vertexShaderPath)
		, mPixelShaderPath(desc.pixelShaderPath)
		, mVSEntryPoint(desc.vsEntryPoint)
		, mPSEntryPoint(desc.psEntryPoint)
		, mPrimitiveTopology(desc.primitiveTopology)
		, mNumRenderTargets(desc.numRenderTargets)
		, mDSVFormat(desc.dsvFormat)
		, mSampleCount(desc.sampleCount)
		, mSampleQuality(desc.sampleQuality)
	{
		mBlendDesc = CreateBlendDesc(desc.blendMode);
		mDepthStencilDesc = CreateDepthStencilDesc(desc.depthTestEnabled, desc.depthWriteEnabled);

		// Rasterizer State 직접 초기화
		mRasterizerDesc = {};
		mRasterizerDesc.FillMode = desc.fillMode;
		mRasterizerDesc.CullMode = desc.cullMode;
		mRasterizerDesc.FrontCounterClockwise = FALSE;
		mRasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		mRasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		mRasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		mRasterizerDesc.DepthClipEnable = TRUE;
		mRasterizerDesc.MultisampleEnable = FALSE;
		mRasterizerDesc.AntialiasedLineEnable = FALSE;
		mRasterizerDesc.ForcedSampleCount = 0;
		mRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		// RTV 포맷 복사
		for (Core::uint32 i = 0; i < 8; ++i)
		{
			mRTVFormats[i] = desc.rtvFormats[i];
		}
	}

	Material::Material()
		: mVertexShaderPath(L"BasicShader.hlsl")
		, mPixelShaderPath(L"BasicShader.hlsl")
		, mVSEntryPoint("VSMain")
		, mPSEntryPoint("PSMain")
		, mPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
		, mNumRenderTargets(1)
		, mDSVFormat(DXGI_FORMAT_D24_UNORM_S8_UINT)
		, mSampleCount(1)
		, mSampleQuality(0)
	{
		// 기본 설정: Opaque, Back culling, Solid fill, Depth test enabled
		mBlendDesc = CreateBlendDesc(BlendMode::Opaque);
		mDepthStencilDesc = CreateDepthStencilDesc(true, true);

		// Rasterizer State 직접 초기화
		mRasterizerDesc = {};
		mRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		mRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
		mRasterizerDesc.FrontCounterClockwise = FALSE;
		mRasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		mRasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		mRasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		mRasterizerDesc.DepthClipEnable = TRUE;
		mRasterizerDesc.MultisampleEnable = FALSE;
		mRasterizerDesc.AntialiasedLineEnable = FALSE;
		mRasterizerDesc.ForcedSampleCount = 0;
		mRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		// RTV 포맷 기본값
		mRTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		for (Core::uint32 i = 1; i < 8; ++i)
		{
			mRTVFormats[i] = DXGI_FORMAT_UNKNOWN;
		}
	}

	D3D12_BLEND_DESC Material::CreateBlendDesc(BlendMode mode)
	{
		D3D12_BLEND_DESC blendDesc = {};
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable = FALSE;

		D3D12_RENDER_TARGET_BLEND_DESC& rtBlend = blendDesc.RenderTarget[0];

		switch (mode)
		{
		case BlendMode::Opaque:
			rtBlend.BlendEnable = FALSE;
			rtBlend.LogicOpEnable = FALSE;
			rtBlend.SrcBlend = D3D12_BLEND_ONE;
			rtBlend.DestBlend = D3D12_BLEND_ZERO;
			rtBlend.BlendOp = D3D12_BLEND_OP_ADD;
			rtBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
			rtBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
			rtBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
			rtBlend.LogicOp = D3D12_LOGIC_OP_NOOP;
			rtBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
			break;

		case BlendMode::AlphaBlend:
			rtBlend.BlendEnable = TRUE;
			rtBlend.LogicOpEnable = FALSE;
			rtBlend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
			rtBlend.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			rtBlend.BlendOp = D3D12_BLEND_OP_ADD;
			rtBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
			rtBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
			rtBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
			rtBlend.LogicOp = D3D12_LOGIC_OP_NOOP;
			rtBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
			break;

		case BlendMode::Additive:
			rtBlend.BlendEnable = TRUE;
			rtBlend.LogicOpEnable = FALSE;
			rtBlend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
			rtBlend.DestBlend = D3D12_BLEND_ONE;
			rtBlend.BlendOp = D3D12_BLEND_OP_ADD;
			rtBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
			rtBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
			rtBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
			rtBlend.LogicOp = D3D12_LOGIC_OP_NOOP;
			rtBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
			break;
		}

		// 나머지 렌더 타겟도 동일하게 설정
		for (Core::uint32 i = 1; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
		{
			blendDesc.RenderTarget[i] = rtBlend;
		}

		return blendDesc;
	}

	D3D12_DEPTH_STENCIL_DESC Material::CreateDepthStencilDesc(bool depthTest, bool depthWrite)
	{
		D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};

		depthStencilDesc.DepthEnable = depthTest ? TRUE : FALSE;
		depthStencilDesc.DepthWriteMask = depthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		depthStencilDesc.StencilEnable = FALSE;
		depthStencilDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
		depthStencilDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

		const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
		{
			D3D12_STENCIL_OP_KEEP,
			D3D12_STENCIL_OP_KEEP,
			D3D12_STENCIL_OP_KEEP,
			D3D12_COMPARISON_FUNC_ALWAYS
		};

		depthStencilDesc.FrontFace = defaultStencilOp;
		depthStencilDesc.BackFace = defaultStencilOp;
		
		return depthStencilDesc;
	}

	size_t Material::GetHash() const
	{
		// 캐시된 해시가 있으면 반환
		if (mCachedHash != 0)
		{
			return mCachedHash;
		}

		// 해시 계산
		size_t hash = 0;

		// 셰이더 경로 해시
		hash ^= HashString(mVertexShaderPath) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		hash ^= HashString(mPixelShaderPath) + 0x9e3779b9 + (hash << 6) + (hash >> 2);

		// Entry Point 해시
		hash ^= HashString(mVSEntryPoint) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		hash ^= HashString(mPSEntryPoint) + 0x9e3779b9 + (hash << 6) + (hash >> 2);

		// 렌더 상태 해시 (간단히 몇 가지 주요 필드만)
		hash ^= std::hash<BOOL>()(mBlendDesc.RenderTarget[0].BlendEnable) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		hash ^= std::hash<D3D12_FILL_MODE>()(mRasterizerDesc.FillMode) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		hash ^= std::hash<D3D12_CULL_MODE>()(mRasterizerDesc.CullMode) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		hash ^= std::hash<BOOL>()(mDepthStencilDesc.DepthEnable) + 0x9e3779b9 + (hash << 6) + (hash >> 2);

		mCachedHash = hash;
		return mCachedHash;
	}

	size_t Material::HashString(const wstring& str)
	{
		return std::hash<wstring>()(str);
	}

	size_t Material::HashString(const string& str)
	{
		return std::hash<string>()(str);
	}

} // namespace Graphics