// Graphics/src/DX12/DX12GraphicsRootSignature.cpp
#include "Graphics/DX12/DX12GraphicsRootSignature.h"
#include "Graphics/DX12/d3dx12.h"
#include "Core/Logging/LogMacros.h"

namespace Graphics
{
	bool DX12GraphicsRootSignature::Initialize(ID3D12Device* device)
	{
		// 빈 루트 시그니처 생성 (파라미터 없음)
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc;
		rootSigDesc.Init(
			0,
			nullptr,
			0,
			nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC versionedDesc(rootSigDesc);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;

		HRESULT hr = D3DX12SerializeVersionedRootSignature(
			&versionedDesc,
			D3D_ROOT_SIGNATURE_VERSION_1_1,
			&signature,
			&error
		);

		if (FAILED(hr))
		{
			if (error)
			{
				const char* errorMsg = static_cast<const char*>(error->GetBufferPointer());
				LOG_ERROR("Failed to serialize root signature: %s", errorMsg);
			}
			else
			{
				LOG_ERROR("Failed to serialize root signature (HRESULT: 0x%08X)", hr);
			}
			return false;
		}

		hr = device->CreateRootSignature(
			0,
			signature->GetBufferPointer(),
			signature->GetBufferSize(),
			IID_PPV_ARGS(&mRootSignature)
		);

		if (FAILED(hr))
		{
			LOG_ERROR("Failed to create root signature");
			return false;
		}

		LOG_INFO("Root signature created successfully");
		return true;
	}
}