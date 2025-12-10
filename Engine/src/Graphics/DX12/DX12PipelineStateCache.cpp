#include "pch.h"
#include "Graphics/DX12/DX12PipelineStateCache.h"
#include "Graphics/DX12/DX12ShaderCompiler.h"
#include "Graphics/Material.h"

using namespace std;

namespace Graphics
{
	DX12PipelineStateCache::~DX12PipelineStateCache()
	{
		Shutdown();
	}

	bool DX12PipelineStateCache::Initialize(ID3D12Device* device, DX12ShaderCompiler* shaderCompiler)
	{
		if (!device)
		{
			LOG_ERROR("DX12PipelineStateCache: Invalid device pointer");
			return false;
		}

		if (!shaderCompiler)
		{
			LOG_ERROR("DX12PipelineStateCache: Invalid shader compiler pointer");
			return false;
		}

		mDevice = device;
		mShaderCompiler = shaderCompiler;

		LOG_INFO("DX12PipelineStateCache initialized");
		return true;
	}

	ID3D12PipelineState* DX12PipelineStateCache::GetOrCreatePipelineState(
		const Material& material,
		ID3D12RootSignature* rootSignature,
		const D3D12_INPUT_LAYOUT_DESC& inputLayout
	)
	{
		if (!mDevice || !mShaderCompiler)
		{
			LOG_ERROR("DX12PipelineStateCache not initialized");
			return nullptr;
		}

		if (!rootSignature)
		{
			LOG_ERROR("DX12PipelineStateCache: Root Signature is null");
			return nullptr;
		}

		// 캐시 키 생성
		PSOKey key;
		key.materialHash = material.GetHash();
		key.inputLayoutHash = HashInputLayout(inputLayout);
		key.rootSignature = rootSignature;

		// 캐시 검색
		auto it = mPSOCache.find(key);
		if (it != mPSOCache.end())
		{
			// LOG_DEBUG("DX12PipelineStateCache: PSO cache hit (Material hash: %zu)", key.materialHash);
			return it->second.Get();
		}

		// 새로 생성
		LOG_INFO("DX12PipelineStateCache: Creating new PSO (Material hash: %zu)", key.materialHash);

		ComPtr<ID3D12PipelineState> pso = CreatePSO(material, rootSignature, inputLayout);
		if (!pso)
		{
			LOG_ERROR("DX12PipelineStateCache: Failed to create PSO");
			return nullptr;
		}

		// 캐시에 저장
		mPSOCache[key] = pso;

		LOG_INFO("DX12PipelineStateCache: PSO created successfully (Total cached: %zu)", mPSOCache.size());

		return pso.Get();
	}

	ComPtr<ID3D12PipelineState> DX12PipelineStateCache::CreatePSO(
		const Material& material,
		ID3D12RootSignature* rootSignature,
		const D3D12_INPUT_LAYOUT_DESC& inputLayout
	)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		ComPtr<ID3DBlob> vsBlob;
		ComPtr<ID3DBlob> psBlob;

		// 셰이더 컴파일 및 Desc 채우기
		if (!CompileShadersAndFillDesc(material, psoDesc, vsBlob, psBlob))
		{
			LOG_ERROR("Failed to compile shaders");
			return nullptr;
		}

		// Root Signature 및 Input Layout 설정
		psoDesc.pRootSignature = rootSignature;
		psoDesc.InputLayout = inputLayout;

		// Material에서 렌더 상태 가져오기
		psoDesc.BlendState = material.GetBlendState();
		psoDesc.RasterizerState = material.GetRasterizerState();
		psoDesc.DepthStencilState = material.GetDepthStencilState();

		// Material에서 PSO 설정 가져오기
		psoDesc.PrimitiveTopologyType = material.GetPrimitiveTopology();
		psoDesc.NumRenderTargets = static_cast<UINT>(material.GetNumRenderTargets());

		const DXGI_FORMAT* rtvFormats = material.GetRTVFormats();
		for (UINT i = 0; i < material.GetNumRenderTargets(); ++i)
		{
			psoDesc.RTVFormats[i] = rtvFormats[i];
		}

		psoDesc.DSVFormat = material.GetDSVFormat();

		// Sample 설정
		psoDesc.SampleMask = static_cast<UINT>(material.GetSampleMask());
		psoDesc.SampleDesc.Count = static_cast<UINT>(material.GetSampleCount());
		psoDesc.SampleDesc.Quality = static_cast<UINT>(material.GetSampleQuality());

		// PSO 생성
		ComPtr<ID3D12PipelineState> pso;
		HRESULT hr = mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));

		if (FAILED(hr))
		{
			LOG_ERROR("Failed to create Pipeline State Object (HRESULT: 0x%08X)", hr);
			return nullptr;
		}

		// 디버그 이름 설정
#if defined(_DEBUG)
		wstring debugName = L"PSO_";
		debugName += material.GetVertexShaderPath();
		pso->SetName(debugName.c_str());
#endif

		return pso;
	}

	bool DX12PipelineStateCache::CompileShadersAndFillDesc(
		const Material& material,
		D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc,
		ComPtr<ID3DBlob>& vsBlob,
		ComPtr<ID3DBlob>& psBlob
	)
	{
		// Vertex Shader 컴파일
		if (!mShaderCompiler->CompileFromFile(
			material.GetVertexShaderPath(),
			material.GetVSEntryPoint(),
			"vs_5_1",
			vsBlob.GetAddressOf()
		))
		{
			LOG_ERROR("Failed to compile Vertex Shader: %ls", material.GetVertexShaderPath());
			return false;
		}

		desc.VS.pShaderBytecode = vsBlob->GetBufferPointer();
		desc.VS.BytecodeLength = vsBlob->GetBufferSize();

		// Pixel Shader 컴파일
		if (!mShaderCompiler->CompileFromFile(
			material.GetPixelShaderPath(),
			material.GetPSEntryPoint(),
			"ps_5_1",
			psBlob.GetAddressOf()
		))
		{
			LOG_ERROR("Failed to compile Pixel Shader: %ls", material.GetPixelShaderPath());
			return false;
		}

		desc.PS.pShaderBytecode = psBlob->GetBufferPointer();
		desc.PS.BytecodeLength = psBlob->GetBufferSize();

		return true;
	}

	size_t DX12PipelineStateCache::HashInputLayout(const D3D12_INPUT_LAYOUT_DESC& inputLayout)
	{
		size_t hash = 0;

		// Input Element 개수 해시
		hash ^= std::hash<UINT>()(inputLayout.NumElements) + 0x9e3779b9 + (hash << 6) + (hash >> 2);

		// 각 Input Element 해시
		for (UINT i = 0; i < inputLayout.NumElements; ++i)
		{
			const D3D12_INPUT_ELEMENT_DESC& element = inputLayout.pInputElementDescs[i];

			// SemanticName 해시
			if (element.SemanticName)
			{
				string semanticName(element.SemanticName);
				hash ^= std::hash<string>()(semanticName) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			}

			// 주요 필드 해시
			hash ^= std::hash<UINT>()(element.SemanticIndex) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			hash ^= std::hash<DXGI_FORMAT>()(element.Format) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			hash ^= std::hash<UINT>()(element.InputSlot) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			hash ^= std::hash<UINT>()(element.AlignedByteOffset) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		}

		return hash;
	}

	void DX12PipelineStateCache::Clear()
	{
		size_t count = mPSOCache.size();
		if (count > 0)
		{
			LOG_INFO("DX12PipelineStateCache: Clearing PSO cache (%zu PSOs)", count);
			mPSOCache.clear();
		}
	}

	void DX12PipelineStateCache::Shutdown()
	{
		LOG_GRAPHICS_INFO("[DX12PipelineStateCache] Shutting down Pipeline State Cache...");

		Clear();
		mDevice = nullptr;
		mShaderCompiler = nullptr;

		LOG_GRAPHICS_INFO("[DX12PipelineStateCache] Pipeline State Cache shut down successfully");
	}

} // namespace Graphics
