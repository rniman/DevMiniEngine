#include "Platform/Window.h"
#include "Platform/PlatformTypes.h"
#include "Platform/Input.h"
#include "Core/Logging/Logger.h"
#include "Core/Logging/ConsoleSink.h"
#include "Core/Logging/LogMacros.h"

// Graphics headers
#include "Graphics/DX12/DX12Device.h"
#include "Graphics/DX12/DX12CommandQueue.h"
#include "Graphics/DX12/DX12SwapChain.h"
#include "Graphics/DX12/DX12DescriptorHeap.h"
#include "Graphics/DX12/DX12CommandContext.h"


// float r = 0.0f;
// float g = 0.0f;
// float b = 0.0f;

Core::uint64 frameResource[Graphics::FRAME_BUFFER_COUNT] = {};
Core::uint32 currentFrameIndex = 0;

// Helper function: Render one frame
void RenderFrame(Graphics::DX12Device& device)
{
    auto* swapChain = device.GetSwapChain();
    auto* graphicsQueue = device.GetGraphicsQueue();
    auto* rtvHeap = swapChain->GetRTVHeap();

    // Get current back buffer index
    Core::uint32 backBufferIndex = swapChain->GetCurrentBackBufferIndex();
    
    // 현재 Back Buffer에 대한 GPU의 사용이 끝났는지 확인합니다.
    graphicsQueue->WaitForFenceValue(frameResource[currentFrameIndex]);

    // Get command context for this frame
    auto* cmdContext = device.GetCommandContext(currentFrameIndex);
    if (!cmdContext)
    {
        LOG_ERROR("Failed to get Command Context");
        return;
    }

    // Reset command list
    if (!cmdContext->Reset())
    {
        LOG_ERROR("Failed to reset Command Context");
        return;
    }

    auto* cmdList = cmdContext->GetCommandList();
    auto* backBuffer = swapChain->GetCurrentBackBuffer();

    // Transition: PRESENT → RENDER_TARGET
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        backBuffer,
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    cmdList->ResourceBarrier(1, &barrier);

    // Get RTV handle
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUHandle(backBufferIndex);

    // Clear render target
    // const float clearColor[] = { r, g, b, 1.0f };  // #6495ED
    // r += 0.0001f;
    // g += 0.0002f;
    // b += 0.0003f;
       
    // if (r > 1.0f) r = 0.0f;
    // if (g > 1.0f) g = 0.0f;
    // if (b > 1.0f) b = 0.0f;

    // Cornflower Blue (기본)
    const float clearColor[] = { 0.392f, 0.584f, 0.929f, 1.0f };
    //// 빨강
    //const float clearColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    //// 초록
    //const float clearColor[] = { 0.0f, 1.0f, 0.0f, 1.0f };
    //// 검정
    //const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    //// 흰색
    //const float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // Transition: RENDER_TARGET → PRESENT
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        backBuffer,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT
    );
    cmdList->ResourceBarrier(1, &barrier);

    // Close command list
    if (!cmdContext->Close())
    {
        LOG_ERROR("Failed to close Command List");
        return;
    }

    // Execute command list
    ID3D12CommandList* cmdLists[] = { cmdList };
    frameResource[currentFrameIndex] = graphicsQueue->ExecuteCommandLists(cmdLists, 1);

    // Present
    swapChain->Present(true);  // VSync enabled

    // Move to next frame
    swapChain->MoveToNextFrame();

    currentFrameIndex = (currentFrameIndex + 1) % Graphics::FRAME_BUFFER_COUNT;
    // Wait for GPU (간단한 동기화 - 실제로는 Fence 사용) -> 사용 최소화로 변경
    // graphicsQueue->WaitForIdle();
}

int main()
{
    // Initialize logging
    auto& logger = Core::Logging::Logger::GetInstance();
    logger.AddSink(std::make_unique<Core::Logging::ConsoleSink>(true));

    LOG_INFO("=== 08_DX12Init Sample Started ===");

    // Create window
    Platform::WindowDesc windowDesc;
    windowDesc.title = "08_DX12Init - DevMiniEngine";
    windowDesc.width = 1280;
    windowDesc.height = 720;
    windowDesc.resizable = true;

    auto window = Platform::CreatePlatformWindow();
    if (!window->Create(windowDesc))
    {
        LOG_ERROR("Failed to create window");
        return -1;
    }

    LOG_INFO("Window created successfully");

    // Get input manager
    auto& input = window->GetInput();

    // Initialize DirectX 12 Device
    Graphics::DX12Device device;
    if (!device.Initialize(true))  // Enable debug layer in debug builds
    {
        LOG_ERROR("Failed to initialize DirectX 12 Device");
        window->Destroy();
        return -1;
    }
    LOG_INFO("DirectX 12 Device initialized successfully");

    // Get Command Queue
    auto* graphicsQueue = device.GetGraphicsQueue();
    if (!graphicsQueue || !graphicsQueue->IsInitialized())
    {
        LOG_ERROR("Failed to get Graphics Command Queue");
        device.Shutdown();
        window->Destroy();
        return -1;
    }
    LOG_INFO("Graphics Command Queue ready");

    // Create SwapChain & Descriptor Heap
    HWND hwnd = reinterpret_cast<HWND>(window->GetNativeHandle());
    if (!device.CreateSwapChain(hwnd, windowDesc.width, windowDesc.height))
    {
        LOG_ERROR("Failed to create SwapChain");
        device.Shutdown();
        window->Destroy();
        return -1;
    }

    auto* swapChain = device.GetSwapChain();
    auto* rtvHeap = swapChain->GetRTVHeap();

    LOG_INFO(
        "SwapChain ready (%u x %u, %u buffers)",
        swapChain->GetWidth(),
        swapChain->GetHeight(),
        swapChain->GetBufferCount()
    );

    LOG_INFO("RTV Descriptor Heap ready (%u descriptors)", rtvHeap->GetNumDescriptors());

    LOG_INFO("DirectX 12 initialization completed successfully!");
    LOG_INFO("Rendering Cornflower Blue screen...");
    LOG_INFO("Press ESC to exit");

    // Main loop
    while (!window->ShouldClose())
    {
        input.Update();
        window->ProcessEvents();

        if (input.IsKeyPressed(Platform::KeyCode::Escape))
        {
            LOG_INFO("ESC pressed - Exiting");
            break;
        }

        // Render frame
        RenderFrame(device);

        input.Reset();
    }

    // Shutdown
    device.Shutdown();
    window->Destroy();

    LOG_INFO("=== 08_DX12Init Sample Completed ===");

    return 0;
}