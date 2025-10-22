# 08_DXInit - DirectX 12 초기화 샘플

> **참고**: 이 문서의 코드 예시는 이해를 돕기 위해 단순화되었습니다.
> 실제 구현은 `Engine/Graphics/src/DX12/` 및 `Samples/08_DX12Init/main.cpp`를 참고하세요.

## 개요
DirectX 12의 기본 초기화 파이프라인을 구현한 샘플 프로젝트입니다. Device부터 SwapChain까지 생성하고 단색(Cornflower Blue)을 화면에 출력합니다.

## 학습 목표
- DirectX 12 초기화 순서 이해
- Command Queue와 Command List 개념
- SwapChain과 더블 버퍼링
- CPU-GPU 동기화 (Fence)
- 리소스 상태 전환 (Resource Barrier)

---

## 주요 생성 과정

### 1. Factory 및 Adapter 선택
```cpp
// DXGI Factory 생성
UINT flags = enableDebugLayer ? DXGI_CREATE_FACTORY_DEBUG : 0;
CreateDXGIFactory2(flags, IID_PPV_ARGS(&factory));

// Factory 버전 쿼리 (기능 감지)
factory->QueryInterface(IID_PPV_ARGS(&factory5));  // Tearing 지원
factory->QueryInterface(IID_PPV_ARGS(&factory6));  // DXGI 1.6 기능

// 고성능 GPU 어댑터 선택
factory->EnumAdapterByGpuPreference(
    0, 
    DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, 
    IID_PPV_ARGS(&adapter)
);
```

**Factory 버전별 기능**:
- **Factory4**: 기본 기능
- **Factory5**: Tearing 지원 감지
- **Factory6**: 어댑터 GPU 선호도 지원

### 2. Device 생성
```cpp
// Feature Level 순차 시도: 12.1 → 12.0 → 11.1 → 11.0
D3D_FEATURE_LEVEL featureLevels[] = {
    D3D_FEATURE_LEVEL_12_1,
    D3D_FEATURE_LEVEL_12_0,
    D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0
};

for (auto level : featureLevels)
{
    if (SUCCEEDED(D3D12CreateDevice(adapter, level, IID_PPV_ARGS(&device))))
    {
        featureLevel = level;
        break;
    }
}
```

**Debug Layer 활성화** (Debug 빌드):
```cpp
#ifdef _DEBUG
ComPtr<ID3D12Debug> debugController;
if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
{
    debugController->EnableDebugLayer();
}
#endif
```

### 3. Command Queue 생성
```cpp
// Graphics Queue 생성
D3D12_COMMAND_QUEUE_DESC queueDesc = {};
queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));

// Fence 생성 (GPU 동기화용)
device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
```

**Fence 동기화**:
- `Signal()`: GPU가 명령 완료 시 Fence 값 증가
- `WaitForFenceValue()`: 특정 Fence 값까지 CPU 대기

### 4. SwapChain 생성
```cpp
// SwapChain 생성
DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
swapChainDesc.Width = width;
swapChainDesc.Height = height;
swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
swapChainDesc.BufferCount = FRAME_BUFFER_COUNT;  // 2
swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
swapChainDesc.SampleDesc.Count = 1;

factory->CreateSwapChainForHwnd(
    commandQueue,
    hwnd,
    &swapChainDesc,
    nullptr,
    nullptr,
    &swapChain1
);
swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain));
```

**SwapChain 내부에서 RTV Descriptor Heap 생성**:
```cpp
// RTV Descriptor Heap (SwapChain 소유)
D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
rtvHeapDesc.NumDescriptors = FRAME_BUFFER_COUNT;  // 2
rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));

// 각 백 버퍼의 RTV 생성
CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
    rtvHeap->GetCPUDescriptorHandleForHeapStart()
);
for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
{
    swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
    device->CreateRenderTargetView(backBuffers[i], nullptr, rtvHandle);
    rtvHandle.Offset(1, rtvDescriptorSize);
}
```

### 5. Command Context 생성
```cpp
// 프레임 버퍼 개수만큼 생성 (2개)
for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
{
    // Command Allocator
    device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&commandAllocators[i])
    );
    
    // Command List
    device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        commandAllocators[i],
        nullptr,
        IID_PPV_ARGS(&commandLists[i])
    );
    commandLists[i]->Close();  // 초기 상태는 닫힌 상태
}
```

### 6. Frame Resources 초기화
```cpp
// 각 프레임의 Fence Value 추적용
uint64 frameResources[FRAME_BUFFER_COUNT] = {};  // {0, 0}
uint32 currentFrameIndex = 0;
```

---

## 렌더링 루프

### 인덱스 개념
프로젝트는 **이중 인덱싱 시스템**을 사용합니다:

- **currentFrameIndex**: CPU가 사용하는 **순차 인덱스** (0→1→0...)
  - 용도: Command Allocator, Command List, Frame Resource 접근
  - 증가 방식: `(currentFrameIndex + 1) % FRAME_BUFFER_COUNT`
  
- **backBufferIndex**: GPU가 지정하는 **비순차 인덱스**
  - 용도: RTV, Back Buffer 리소스 접근
  - 획득 방식: `swapChain->GetCurrentBackBufferIndex()`

### 렌더링 단계
```cpp
while (running)
{
    // 1. CPU 대기 (현재 프레임 리소스 사용 가능할 때까지)
    commandQueue->WaitForFenceValue(frameResources[currentFrameIndex]);
    
    // 2. 백 버퍼 인덱스 획득
    uint32 backBufferIndex = swapChain->GetCurrentBackBufferIndex();
    
    // 3. 명령 기록 준비
    commandAllocators[currentFrameIndex]->Reset();
    commandLists[currentFrameIndex]->Reset(
        commandAllocators[currentFrameIndex], 
        nullptr
    );
    
    // 4. 명령 기록
    // 4-1. 리소스 배리어 (PRESENT → RENDER_TARGET)
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        backBuffers[backBufferIndex],
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    commandLists[currentFrameIndex]->ResourceBarrier(1, &barrier);
    
    // 4-2. 화면 클리어 (Cornflower Blue)
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        rtvHeap->GetCPUDescriptorHandleForHeapStart(),
        backBufferIndex,
        rtvDescriptorSize
    );
    float clearColor[] = { 0.392f, 0.584f, 0.929f, 1.0f };
    commandLists[currentFrameIndex]->ClearRenderTargetView(
        rtvHandle,
        clearColor,
        0,
        nullptr
    );
    
    // 4-3. 리소스 배리어 (RENDER_TARGET → PRESENT)
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        backBuffers[backBufferIndex],
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT
    );
    commandLists[currentFrameIndex]->ResourceBarrier(1, &barrier);
    
    // 5. 명령 리스트 닫기
    commandLists[currentFrameIndex]->Close();
    
    // 6. 명령 제출 및 Fence Value 저장
    ID3D12CommandList* lists[] = { commandLists[currentFrameIndex] };

    // ExecuteCommandLists와 Signal을 래핑한 헬퍼 함수를 사용하거나,
    // ExecuteCommandLists가 반환한 값을 저장한다고 가정합니다.
    uint64 newFenceValue = commandQueue->ExecuteAndSignal(1, lists); // 예시 헬퍼 함수
    frameResources[currentFrameIndex] = newFenceValue;

    // 7. 화면 표시 및 인덱스 갱신
    swapChain->Present(1, 0);  // VSync 활성화
    swapChain->MoveToNextFrame();
    currentFrameIndex = (currentFrameIndex + 1) % FRAME_BUFFER_COUNT;
}
```

---

## 주요 개념

### Command Allocator vs Command List
- **Command Allocator**: 명령을 저장할 메모리 풀 관리
  - 프레임마다 별도 할당자 필요 (재사용 충돌 방지)
  - Reset으로 메모리 재사용
  
- **Command List**: 실제 명령을 기록하는 객체
  - Reset으로 기존 메모리(Allocator)에 새 명령 기록
  - Close 후 ExecuteCommandLists로 제출

### 리소스 배리어
GPU가 리소스에 접근하는 방식을 전환합니다. DirectX 12는 명시적 상태 관리가 필요합니다.

- **PRESENT**: 화면 출력 대기 상태 (Display)
- **RENDER_TARGET**: 렌더링 대상 상태 (GPU Write)

배리어 없이 상태 전환 시 **정의되지 않은 동작** 발생 가능합니다.

### Fence와 동기화

**WaitForFenceValue 방식**:
```cpp
// 프레임별 독립적 Fence 추적
frameResources[currentFrameIndex] = commandQueue->Signal();
commandQueue->WaitForFenceValue(frameResources[currentFrameIndex]);
```

- 각 프레임의 Fence Value를 추적
- 필요한 리소스만 대기 (다른 프레임은 병렬 실행 가능)
- CPU 유휴 시간 최소화

**~~WaitForIdle 방식~~ (사용하지 않음)**:
```cpp
// 매 프레임 모든 GPU 작업 대기 (비효율적)
commandQueue->WaitForIdle();
```

---

## 핵심 차이점: 동기화 전략

### ~~WaitForIdle~~ (이전 방식)
- CPU가 GPU의 **모든 작업 완료**까지 대기
- 구현이 단순하지만 CPU 유휴 시간 발생
- 더블 버퍼링 효과 없음 (순차 실행)

### WaitForFenceValue (현재 방식)
- **현재 사용할 리소스만** 완료 확인
- CPU-GPU 병렬 작업 가능
- Frame N 렌더링 중 Frame N-1이 GPU에서 실행 가능

**동작 예시** (FRAME_BUFFER_COUNT = 2):
```
Frame 0: CPU 기록 → GPU 실행
         ↓ (병렬)
Frame 1: CPU 기록 대기 → GPU Frame 0 완료 대기 → 기록 시작
```

---

## 구조 설계 결정 사항

### RTV Descriptor Heap 소유권
- **이전**: Device가 소유
- **현재**: SwapChain이 소유
- **이유**: 백 버퍼와 RTV의 생명주기 일치, 책임 분리

### Factory 버전 쿼리
```cpp
ComPtr<IDXGIFactory5> factory5;
ComPtr<IDXGIFactory6> factory6;
factory->QueryInterface(IID_PPV_ARGS(&factory5));
factory->QueryInterface(IID_PPV_ARGS(&factory6));
```
- Tearing 지원, GPU 선호도 등 최신 기능 감지
- 기능 없을 경우 기본 Factory4 사용

### 더블 버퍼링
- `FRAME_BUFFER_COUNT = 2` 사용
- 트리플 버퍼링은 학습 단계에서 불필요하다고 판단

---

## 구현 파일

### Graphics 모듈
```
Engine/Graphics/
├── include/Graphics/
│   ├── DX12/
│   │   ├── DX12Device.h              # Device, Factory, Adapter
│   │   ├── DX12CommandQueue.h         # Command Queue, Fence 동기화
│   │   ├── DX12SwapChain.h            # SwapChain, Back Buffer, RTV
│   │   ├── DX12CommandContext.h       # Command Allocator, List
│   │   └── d3dx12.h                   # Microsoft Helper
│   └── GraphicsTypes.h                # 공통 타입, 매크로
└── src/DX12/
    ├── DX12Device.cpp
    ├── DX12CommandQueue.cpp
    ├── DX12SwapChain.cpp
    └── DX12CommandContext.cpp
```

### 샘플 프로젝트
```
Samples/08_DX12Init/
└── main.cpp                           # 렌더링 루프
```

---

## 주요 개선 사항 (2025-10-22)

### 1. 동기화 전략 변경
- **변경 전**: `WaitForIdle()` - 모든 GPU 작업 대기
- **변경 후**: `WaitForFenceValue(frameResources[i])` - 필요한 리소스만 대기
- **기대 효과**: CPU 유휴 시간 감소, 프레임 병렬 처리

### 2. RTV Heap 소유권 이동
- **변경 전**: DX12Device 소유
- **변경 후**: DX12SwapChain 소유
- **이유**: 백 버퍼와 RTV의 생명주기를 일치시키고 책임 분리

### 3. 이중 인덱싱 명확화
- **currentFrameIndex**: CPU 리소스 접근용
- **backBufferIndex**: GPU 렌더링 타겟용
- 용도 혼동 방지를 위한 명확한 구분

---

## 참고 자료
- [DirectX 12 Programming Guide - Microsoft Learn](https://learn.microsoft.com/en-us/windows/win32/direct3d12/directx-12-programming-guide)
- [3D Game Programming with DirectX 12 by Frank Luna](http://www.d3dcoder.net/)
- [Learning DirectX 12 by Jeremiah van Oosten](https://www.3dgep.com/learning-directx-12-1/)

---

## 주요 개선 이력

| 날짜 | 변경 사항 | 이유 |
|------|-----------|------|
| 2025-10-15 | 초기 구현 완료 | DirectX 12 초기화 파이프라인 완성 |
| 2025-10-22 | 동기화 전략 개선 | CPU-GPU 병렬 작업 활성화 |
| 2025-10-22 | RTV Heap 소유권 이동 | 책임 분리 및 생명주기 일치 |
| 2025-10-22 | Factory 버전 쿼리 추가 | 최신 DXGI 기능 활용 |

---

**최종 업데이트**: 2025-10-22