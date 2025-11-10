#include "pch.h"
#include "Graphics/Material.h"
#include "Graphics/DX12/DX12DescriptorHeap.h"
#include "Graphics/Texture.h"
#include "Framework/Resources/ResourceManager.h"

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
		, mSampleMask(desc.sampleMask)
	{
		mBlendDesc = CreateBlendDesc(desc.blendMode);
		mDepthStencilDesc = CreateDepthStencilDesc(
			desc.depthTestEnabled,
			desc.depthWriteEnabled,
			desc.depthComparisonFunc
		);

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

		for (auto& id : mTextureIds)
		{
			id = Framework::ResourceId::Invalid();
		}

		mDescriptorStartIndex = INVALID_DESCRIPTOR_INDEX;
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
		, mSampleMask(0xFFFFFFFF)
	{
		// 기본 설정: Opaque, Back culling, Solid fill, Depth test enabled
		mBlendDesc = CreateBlendDesc(BlendMode::Opaque);
		mDepthStencilDesc = CreateDepthStencilDesc(true, true, D3D12_COMPARISON_FUNC_LESS);

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

		for (auto& id : mTextureIds)
		{
			id = Framework::ResourceId::Invalid();
		}
		
		mDescriptorStartIndex = INVALID_DESCRIPTOR_INDEX;
	}

	bool Material::AllocateDescriptors(
		ID3D12Device* device,
		DX12DescriptorHeap* heap,
		Framework::ResourceManager* resourceMgr
	)
	{
		if (!device || !heap)
		{
			LOG_ERROR("[Material] Device or Heap is null");
			return false;
		}

		if (mDescriptorStartIndex != INVALID_DESCRIPTOR_INDEX)
		{
			LOG_WARN("[Material] Descriptors already allocated. Freeing first.");
			FreeDescriptors(heap);
		}

		// 7개 연속 슬롯 할당
		constexpr uint32 slotCount = static_cast<uint32>(TextureType::Count);
		mDescriptorStartIndex = heap->AllocateBlock(slotCount);

		if (mDescriptorStartIndex == INVALID_DESCRIPTOR_INDEX)
		{
			LOG_ERROR("[Material] Failed to allocate descriptor block");
			return false;
		}

		LOG_INFO(
			"[Material] Allocated descriptor block [%u ~ %u]",
			mDescriptorStartIndex,
			mDescriptorStartIndex + slotCount - 1
		);

		// 각 텍스처 타입별로 SRV 생성
		for (size_t i = 0; i < static_cast<size_t>(TextureType::Count); ++i)
		{
			TextureType type = static_cast<TextureType>(i);
			uint32 descriptorIndex = mDescriptorStartIndex + static_cast<uint32>(i);

			// ResourceId가 유효한지 확인
			if (mTextureIds[i].IsValid())
			{
				// ResourceManager에서 실제 Texture 포인터 조회
				Texture* texture = resourceMgr->GetTexture(mTextureIds[i]);

				if (texture && texture->IsInitialized())
				{
					// 실제 텍스처의 SRV 생성
					if (!texture->CreateSRV(device, heap, descriptorIndex))
					{
						LOG_ERROR(
							"[Material] Failed to create SRV for texture type: %s (ID: 0x%llX)",
							TextureTypeToString(type),
							mTextureIds[i].id
						);
						FreeDescriptors(heap);
						return false;
					}

					LOG_TRACE(
						"[Material] Created SRV for %s at descriptor %u (ID: 0x%llX)",
						TextureTypeToString(type),
						descriptorIndex,
						mTextureIds[i].id
					);
				}
				else
				{
					LOG_WARN(
						"[Material] Texture not found for type %s (ID: 0x%llX), creating Dummy SRV",
						TextureTypeToString(type),
						mTextureIds[i].id
					);
					CreateDummySRV(device, heap, descriptorIndex);
				}
			}
			else
			{
				// ResourceId가 Invalid → Dummy SRV 생성
				CreateDummySRV(device, heap, descriptorIndex);

				LOG_TRACE(
					"[Material] Created Dummy SRV for %s at descriptor %u (no texture)",
					TextureTypeToString(type),
					descriptorIndex
				);
			}
		}

		LOG_INFO("[Material] All descriptors allocated successfully");
		return true;
	}

	void Material::FreeDescriptors(DX12DescriptorHeap* heap)
	{
		if (!heap)
		{
			LOG_ERROR("[Material] Heap is null");
			return;
		}

		if (mDescriptorStartIndex == INVALID_DESCRIPTOR_INDEX)
		{
			LOG_WARN("[Material] No descriptors to free");
			return;
		}

		constexpr uint32 slotCount = static_cast<uint32>(TextureType::Count);
		heap->FreeBlock(mDescriptorStartIndex, slotCount);

		LOG_INFO(
			"[Material] Freed descriptor block [%u ~ %u]",
			mDescriptorStartIndex,
			mDescriptorStartIndex + slotCount - 1
		);

		mDescriptorStartIndex = INVALID_DESCRIPTOR_INDEX;
	}

	void Material::SetTexture(TextureType type, Framework::ResourceId textureId)
	{
		size_t index = static_cast<size_t>(type);
		mTextureIds[index] = textureId;

		LOG_DEBUG(
			"[Material] Set texture %s to ID: 0x%llX",
			TextureTypeToString(type),
			textureId.id
		);
	}

	Framework::ResourceId Material::GetTextureId(TextureType type) const
	{
		size_t index = static_cast<size_t>(type);
		return mTextureIds[index];
	}

	bool Material::HasTexture(TextureType type) const
	{
		size_t index = static_cast<size_t>(type);
		return mTextureIds[index].IsValid();
	}

	uint32 Material::GetTextureCount() const
	{
		uint32 count = 0;
		for (const auto& id : mTextureIds)
		{
			if (id.IsValid())
			{
				++count;
			}
		}
		return count;
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

	D3D12_DEPTH_STENCIL_DESC Material::CreateDepthStencilDesc(
		bool depthTest,
		bool depthWrite,
		D3D12_COMPARISON_FUNC depthComparisonFunc
	)
	{
		D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};

		depthStencilDesc.DepthEnable = depthTest ? TRUE : FALSE;
		depthStencilDesc.DepthWriteMask = depthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
		depthStencilDesc.DepthFunc = depthComparisonFunc;
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

	//std::shared_ptr<Texture> Material::GetTexture(TextureType type) const
	//{
	//	size_t index = static_cast<size_t>(type);
	//	return mTextures[index];
	//}

	//bool Material::HasTexture(TextureType type) const
	//{
	//	size_t index = static_cast<size_t>(type);
	//	return mTextures[index] != nullptr;
	//}

	//uint32 Material::GetTextureCount() const
	//{
	//	uint32 count = 0;
	//	for (const auto& texture : mTextures)
	//	{
	//		if (texture != nullptr)
	//		{
	//			++count;
	//		}
	//	}
	//	return count;
	//}

	D3D12_GPU_DESCRIPTOR_HANDLE Material::GetDescriptorTableHandle(const DX12DescriptorHeap* heap) const
	{
		if (!heap)
		{
			LOG_ERROR("[Material] Heap is null");
			return {};
		}

		if (mDescriptorStartIndex == INVALID_DESCRIPTOR_INDEX)
		{
			LOG_ERROR("[Material] Descriptors not allocated");
			return {};
		}

		return heap->GetGPUHandle(mDescriptorStartIndex);
	}

	bool Material::HasAllocatedDescriptors() const { return mDescriptorStartIndex != INVALID_DESCRIPTOR_INDEX; }

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

	void Material::CreateDummySRV(ID3D12Device* device, DX12DescriptorHeap* heap, uint32 index)
	{
		// 1x1 검은색 텍스처 대신, null descriptor 생성
		// D3D12에서는 null descriptor를 명시적으로 생성할 수 없으므로
		// 기본 텍스처 리소스를 생성하거나 전역 Dummy 텍스처를 사용해야 합니다.
		
		// 방법 1: Null Descriptor (간단하지만 샘플링 시 문제 가능)
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.PlaneSlice = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = heap->GetCPUHandle(index);
		
		// nullptr 리소스로 SRV 생성 (Null Descriptor)
		device->CreateShaderResourceView(
			nullptr,  // null resource
			&srvDesc,
			cpuHandle
		);

		// 방법 2: 실제 1x1 Dummy 텍스처 생성 (더 안전하지만 복잡)
		// TODO: 향후 TextureManager에서 전역 Dummy 텍스처 관리
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
