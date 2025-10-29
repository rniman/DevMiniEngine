#include "pch.h"
#include "Graphics/DX12/DX12CommandQueue.h"
#include "Core/Logging/LogMacros.h"

namespace Graphics
{
	DX12CommandQueue::~DX12CommandQueue()
	{
		Shutdown();
	}

	bool DX12CommandQueue::Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
	{
		if (!device)
		{
			LOG_ERROR("[DX12CommandQueue] Device is null");
			return false;
		}

		LOG_INFO("[DX12CommandQueue] Initializing Command Queue (%s)...", GetCommandListTypeString(type));

		mType = type;
		mNextFenceValue = 1;
		mFenceEvent = nullptr;

		// 1단계: Command Queue 생성
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = type;
		queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.NodeMask = 0;

		HRESULT hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue));
		if (FAILED(hr))
		{
			LOG_ERROR("[DX12CommandQueue] Failed to create Command Queue (HRESULT: 0x%08X)", hr);
			return false;
		}

		LOG_INFO("[DX12CommandQueue] Command Queue created successfully");

		// 2단계: Fence 생성
		if (!CreateFence(device))
		{
			LOG_ERROR("[DX12CommandQueue] Failed to create Fence");
			return false;
		}

		LOG_INFO("[DX12CommandQueue] Command Queue initialized successfully");
		return true;
	}

	void DX12CommandQueue::Shutdown()
	{
		if (!IsInitialized())
		{
			return;
		}

		LOG_INFO("[DX12CommandQueue] Shutting down Command Queue...");

		// 모든 GPU 작업 완료 대기
		WaitForIdle();

		// Fence 이벤트 닫기
		if (mFenceEvent)
		{
			CloseHandle(mFenceEvent);
			mFenceEvent = nullptr;
		}

		// 리소스 해제
		mFence.Reset();
		mCommandQueue.Reset();

		LOG_INFO("[DX12CommandQueue] Command Queue shut down successfully");
	}

	Core::uint64 DX12CommandQueue::ExecuteCommandLists(
		ID3D12CommandList* const* commandLists,
		Core::uint32 numCommandLists
	)
	{
		if (!IsInitialized())
		{
			LOG_ERROR("[DX12CommandQueue] Command Queue not initialized");
			return mNextFenceValue;
		}

		if (!commandLists || numCommandLists == 0)
		{
			LOG_WARN("[DX12CommandQueue] No command lists to execute");
			return mNextFenceValue;
		}

		const Core::uint64 fenceValueToSignal = mNextFenceValue;
		
		// Command List 실행
		mCommandQueue->ExecuteCommandLists(static_cast<UINT>(numCommandLists), commandLists);

		// Fence 신호 전송
		mCommandQueue->Signal(mFence.Get(), static_cast<UINT64>(mNextFenceValue));
		mNextFenceValue++;

		return fenceValueToSignal;
	}

	bool DX12CommandQueue::WaitForIdle()
	{
		if (!IsInitialized())
		{
			LOG_ERROR("[DX12CommandQueue] Command Queue not initialized");
			return false;
		}

		// 현재 Fence 값 신호 전송
		UINT64 fenceValueToWaitFor = static_cast<UINT64>(mNextFenceValue);
		HRESULT hr = mCommandQueue->Signal(mFence.Get(), fenceValueToWaitFor);
		if (FAILED(hr))
		{
			LOG_ERROR("[DX12CommandQueue] Failed to signal fence (HRESULT: 0x%08X)", hr);
			return false;
		}

		mNextFenceValue++;

		// GPU가 이 Fence 값까지 완료하지 않았으면 대기
		if (mFence->GetCompletedValue() < fenceValueToWaitFor)
		{
			hr = mFence->SetEventOnCompletion(fenceValueToWaitFor, mFenceEvent);
			if (FAILED(hr))
			{
				LOG_ERROR("[DX12CommandQueue] Failed to set fence event (HRESULT: 0x%08X)", hr);
				return false;
			}

			WaitForSingleObject(mFenceEvent, INFINITE);
		}

		return true;
	}

	bool DX12CommandQueue::WaitForFenceValue(Core::uint64 valueToWaitFor)
	{
		// 1. 유효성 검사 (큐 초기화)
		if (!IsInitialized())
		{
			LOG_ERROR("[DX12CommandQueue] Command Queue not initialized");
			return false;
		}

		// 2. 유효성 검사 (이벤트 핸들)
		if (mFenceEvent == nullptr)
		{
			LOG_ERROR("[DX12CommandQueue] Fence Event handle is null. (Did CreateEvent fail in Initialize?)");
			return false;
		}

		// 3. 유효성 검사 (값)
		// 0은 FrameResource의 초기값일 수 있으며, mNextFenceValue는 1부터 시작하므로
		// 0을 기다리는 것은 의미가 없습니다.
		if (valueToWaitFor == 0)
		{
			return true; // 기다릴 것이 없음
		}

		// 4. [최적화] GPU가 이미 이 값을 통과했는지 확인
		// GetCompletedValue()는 스레드에 안전하며 빠릅니다.
		if (mFence->GetCompletedValue() >= static_cast<UINT64>(valueToWaitFor))
		{
			return true; // 이미 완료됨, CPU를 대기시킬 필요 없음
		}

		// 5. [대기] GPU가 아직 도달하지 못함. CPU 스레드를 재웁니다.

		// GPU가 'valueToWaitFor' 값에 도달하면 mFenceEvent를 시그널(Signal)하도록 설정
		HRESULT hr = mFence->SetEventOnCompletion(static_cast<UINT64>(valueToWaitFor), mFenceEvent);
		if (FAILED(hr))
		{
			LOG_ERROR("[DX12CommandQueue] Failed to set fence event (HRESULT: 0x%08X)", hr);
			return false;
		}

		// 이벤트가 시그널될 때까지 CPU 스레드를 (효율적으로) 대기시킴
		// (INFINITE는 무한 대기를 의미)
		WaitForSingleObject(mFenceEvent, INFINITE);

		return true;
	}

	Core::uint64 DX12CommandQueue::GetCompletedFenceValue() const
	{
		if (!mFence)
		{
			return 0;
		}

		return mFence->GetCompletedValue();
	}

	bool DX12CommandQueue::CreateFence(ID3D12Device* device)
	{
		LOG_INFO("[DX12CommandQueue] Creating Fence...");

		// Fence 생성
		HRESULT hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
		if (FAILED(hr))
		{
			LOG_ERROR("[DX12CommandQueue] Failed to create Fence (HRESULT: 0x%08X)", hr);
			return false;
		}

		// Fence 이벤트 생성
		mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (!mFenceEvent)
		{
			LOG_ERROR("[DX12CommandQueue] Failed to create Fence event");
			return false;
		}

		LOG_INFO("[DX12CommandQueue] Fence created successfully");
		return true;
	}

	const char* DX12CommandQueue::GetCommandListTypeString(D3D12_COMMAND_LIST_TYPE type) const
	{
		switch (type)
		{
		case D3D12_COMMAND_LIST_TYPE_DIRECT:
			return "Direct (Graphics)";
		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			return "Compute";
		case D3D12_COMMAND_LIST_TYPE_COPY:
			return "Copy";
		default:
			return "Unknown";
		}
	}

} // namespace Graphics