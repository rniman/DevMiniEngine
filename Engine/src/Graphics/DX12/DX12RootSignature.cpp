#include "Graphics/DX12/DX12RootSignature.h"
#include "Core/Logging/LogMacros.h"

namespace Graphics
{
	DX12RootSignature::~DX12RootSignature()
	{
		Shutdown();
	}

	bool DX12RootSignature::Initialize(
		ID3D12Device* device,
		Core::uint32 numParameters,
		const D3D12_ROOT_PARAMETER1* parameters,
		Core::uint32 numStaticSamplers,
		const D3D12_STATIC_SAMPLER_DESC* staticSamplers,
		D3D12_ROOT_SIGNATURE_FLAGS flags
	)
	{
		CORE_ASSERT(device != nullptr, "Device is null");

		// 이미 초기화된 경우 종료
		if (mRootSignature != nullptr)
		{
			LOG_WARN("Root Signature already initialized. Shutting down existing one.");
			Shutdown();
		}

		// Root Signature Desc 1.1 생성
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
		rootSigDesc.Init_1_1(
			numParameters,
			parameters,
			numStaticSamplers,
			staticSamplers,
			flags
		);

		// Root Signature 시리얼라이즈
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
			if (errorBlob != nullptr)
			{
				const char* errorMsg = static_cast<const char*>(errorBlob->GetBufferPointer());
				LOG_ERROR("Failed to serialize Root Signature: %s", errorMsg);
			}
			else
			{
				LOG_ERROR("Failed to serialize Root Signature (HRESULT: 0x%08X)", hr);
			}
			return false;
		}

		// Root Signature 생성
		hr = device->CreateRootSignature(
			0,
			signatureBlob->GetBufferPointer(),
			signatureBlob->GetBufferSize(),
			IID_PPV_ARGS(&mRootSignature)
		);

		if (FAILED(hr))
		{
			LOG_ERROR("Failed to create Root Signature (HRESULT: 0x%08X)", hr);
			return false;
		}

		LOG_INFO(
			"Root Signature created successfully (%u parameters, %u samplers)",
			numParameters, numStaticSamplers
		);

		return true;
	}

	bool DX12RootSignature::InitializeEmpty(ID3D12Device* device)
	{
		CORE_ASSERT(device != nullptr, "Device is null");

		LOG_INFO("Creating Empty Root Signature (no parameters)");

		// Empty Root Signature는 파라미터 없이 생성
		return Initialize(
			device,
			0,
			nullptr,
			0,
			nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		);
	}

	void DX12RootSignature::Shutdown()
	{
		if (mRootSignature != nullptr)
		{
			LOG_TRACE("Shutting down Root Signature");
			mRootSignature.Reset();
		}
	}

} // namespace Graphics