#include "pch.h"
#include "Graphics/DX12/DX12CommandContext.h"
#include "Core/Logging/LogMacros.h"

namespace Graphics
{
	DX12CommandContext::~DX12CommandContext()
	{
		Shutdown();
	}

	bool DX12CommandContext::Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
	{
		if (!device)
		{
			LOG_ERROR("[DX12CommandContext] Device is null");
			return false;
		}

		LOG_INFO("[DX12CommandContext] Initializing Command Context...");

		mType = type;

		// Command Allocator 생성
		HRESULT hr = device->CreateCommandAllocator(type, IID_PPV_ARGS(&mCommandAllocator));
		if (FAILED(hr))
		{
			LOG_ERROR("[DX12CommandContext] Failed to create Command Allocator (HRESULT: 0x%08X)", hr);
			return false;
		}

		// Command List 생성
		hr = device->CreateCommandList(
			0,
			type,
			mCommandAllocator.Get(),
			nullptr,
			IID_PPV_ARGS(&mCommandList)
		);
		if (FAILED(hr))
		{
			LOG_ERROR("[DX12CommandContext] Failed to create Command List (HRESULT: 0x%08X)", hr);
			return false;
		}

		// Command List는 생성 시 Open 상태이므로 닫아야 함
		mCommandList->Close();

		LOG_INFO("[DX12CommandContext] Command Context initialized successfully");
		return true;
	}

	void DX12CommandContext::Shutdown()
	{
		if (!IsInitialized())
		{
			return;
		}

		LOG_INFO("[DX12CommandContext] Shutting down Command Context...");

		mCommandList.Reset();
		mCommandAllocator.Reset();

		LOG_INFO("[DX12CommandContext] Command Context shut down successfully");
	}

	bool DX12CommandContext::Reset()
	{
		if (!IsInitialized())
		{
			LOG_ERROR("[DX12CommandContext] Command Context not initialized");
			return false;
		}

		// Reset allocator
		HRESULT hr = mCommandAllocator->Reset();
		if (FAILED(hr))
		{
			LOG_ERROR("[DX12CommandContext] Failed to reset Command Allocator (HRESULT: 0x%08X)", hr);
			return false;
		}

		// Reset command list
		hr = mCommandList->Reset(mCommandAllocator.Get(), nullptr);
		if (FAILED(hr))
		{
			LOG_ERROR("[DX12CommandContext] Failed to reset Command List (HRESULT: 0x%08X)", hr);
			return false;
		}

		return true;
	}

	bool DX12CommandContext::Close()
	{
		if (!IsInitialized())
		{
			LOG_ERROR("[DX12CommandContext] Command Context not initialized");
			return false;
		}

		HRESULT hr = mCommandList->Close();
		if (FAILED(hr))
		{
			LOG_ERROR("[DX12CommandContext] Failed to close Command List (HRESULT: 0x%08X)", hr);
			return false;
		}

		return true;
	}

} // namespace Graphics