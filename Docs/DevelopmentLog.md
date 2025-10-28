# Development Log

> **Note**: This log was written in English until 2025-10-15. 
> From 2025-10-16 onwards, entries are written in Korean for better clarity and efficiency.
> 
> **참고**: 2025-10-15까지는 영어로 작성되었습니다. 
> 2025-10-16부터는 명확성과 효율성을 위해 한글로 작성합니다.

---

## 2025-10-28 - 카메라 시스템 구현 완성

### Tasks
- [x] Camera 베이스 클래스 (View 행렬 공통 로직)
- [x] PerspectiveCamera (FOV 기반 원근 투영)
- [x] OrthographicCamera (크기 기반 직교 투영)
- [x] 11_CameraTest 샘플 검증

### Decisions

**아키텍처**
- Camera 추상 클래스로 View 행렬 관리 통합
- 투영 방식만 PerspectiveCamera/OrthographicCamera에서 구현
- Dirty Flag 패턴으로 불필요한 재계산 방지

**주요 기능**
- SetLookAt(position, target, up) 방식
- 카메라 이동/회전 헬퍼 (MoveForward, RotateYaw 등)
- DirectXMath 기반 SIMD 최적화

### Implementation

```cpp
class Camera
{
protected:
    Vector3 mPosition, mTarget, mUpVector;
    Matrix4x4 mViewMatrix, mProjectionMatrix, mViewProjectionMatrix;
    bool mViewDirty = true, mProjectionDirty = true;
    
public:
    virtual void UpdateProjectionMatrix() = 0;
    void UpdateViewMatrix();
    void SetLookAt(const Vector3& position, const Vector3& target, const Vector3& up);
};
```

### Lessons Learned
- View/Projection 독립적 Dirty Flag로 효율적 업데이트
- **Left-Handed 좌표계** 사용 (DirectX 표준, Forward = +Z)
- 동일 위치의 Perspective/Orthographic은 동일한 View 행렬 생성

### Next Steps
- [ ] MVP 행렬 통합 (12_CameraRendering)
- [ ] 3D 큐브 회전 렌더링
- [ ] 카메라 컨트롤러 (향후)
- [ ] 텍스처 로딩 및 샘플링
- [ ] Depth Buffer & Depth Test
- [ ] 여러 객체 렌더링 (다중 Draw Call)

---

## 2025-10-27 - CBV를 이용한 회전 렌더링 완성

### Tasks
- [x] 10_RotatingTriangle 샘플 구현
- [x] Constant Buffer View (CBV) 바인딩
- [x] Root Signature에 CBV 파라미터 추가
- [x] HLSL cbuffer 사용법 학습
- [x] Z축 회전 애니메이션

### Decisions

**MVP 행렬 전략**
- 현재: Model만 사용 (회전)
- View: 항등 행렬
- Projection: 항등 행렬 (2D)
- 향후: 원근 투영 + 카메라 추가 예정

**Constant Buffer 사용**
- 프레임별 업데이트 (매 프레임 회전 각도 변경)
- CPU→GPU 데이터 전송 성공
- Upload Heap 기반 동적 업데이트

### Implementation

**Root Signature (CBV 포함)**
```cpp
CD3DX12_ROOT_PARAMETER1 rootParameters[1];
rootParameters[0].InitAsConstantBufferView(0, 0, 
    D3D12_ROOT_DESCRIPTOR_FLAG_NONE, 
    D3D12_SHADER_VISIBILITY_VERTEX);
```

**HLSL Shader (cbuffer 사용)**
```hlsl
cbuffer MVPConstants : register(b0)
{
    float4x4 MVP;
};
```

### Lessons Learned
- CBV는 Descriptor Heap 없이 직접 GPU 주소로 바인딩 가능
- Root Parameter로 CBV를 사용하면 간단하지만 Root Signature 크기 증가
- 프레임별 Constant Buffer 업데이트가 CPU-GPU 동기화 핵심
- Z축 회전 시 종횡비 문제 → 향후 Projection 행렬로 해결 필요

### Next Steps
- [ ] 카메라 시스템 (View 행렬)
- [ ] 원근 투영 (Projection 행렬)
- [ ] 텍스처 로딩 및 샘플링
- [ ] Depth Buffer & Depth Test
- [ ] 여러 객체 렌더링 (다중 Draw Call)

---

## 2025-10-27 - Constant Buffer 및 Root Signature 리팩토링

### Tasks
- [x] DX12ConstantBuffer 클래스 구현
- [x] DX12GraphicsRootSignature → DX12RootSignature로 리네이밍
- [x] Root Signature Initialize 함수 개선

### Decisions

**DX12ConstantBuffer 설계**
- Upload Heap 사용으로 CPU 직접 업데이트 가능
- 프레임별 독립 메모리 영역 (256바이트 정렬)
- 초기화 시 Map, 프로그램 종료까지 유지
- CPU-GPU 병렬 처리: CPU가 프레임 N 업데이트 중 GPU는 프레임 N-1 렌더링

**타입 안전성**
- `UpdateTyped<T>()` 템플릿 헬퍼 추가
- `static_assert`로 `is_trivially_copyable` 컴파일 타임 검증
- 기존 `Update()`는 동적 크기 버퍼용으로 유지

**Root Signature 개선**
- 클래스 이름 단순화
- Version 1.1 명시, Static Sampler 지원 추가
- `InitializeEmpty()` 헬퍼 함수 추가
- 재초기화 시 경고 후 기존 리소스 해제

### Implementation

**DX12ConstantBuffer 구조**
```cpp
class DX12ConstantBuffer
{
    bool Initialize(ID3D12Device* device, size_t bufferSize, uint32 frameCount);
    void Update(uint32 frameIndex, const void* data, size_t dataSize);
    
    template<typename T>
    void UpdateTyped(uint32 frameIndex, const T& data)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        Update(frameIndex, &data, sizeof(T));
    }
    
    D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(uint32 frameIndex) const;
};
```

**메모리 레이아웃(예시)**
```
[Frame 0: 256B][Frame 1: 256B][Frame 2: 256B]
  CPU 업데이트    GPU 읽기        대기 중
```

### Lessons Learned
- static_assert로 컴파일 타임 타입 검증 → GPU 업로드 실패 사전 방지
- Upload Heap은 Write-Combined 메모리 (memcpy 후 자동 GPU 전달)
- 템플릿 전체 클래스보다 헬퍼 함수로 타입 안전성과 유연성 양립

### Next Steps
- [ ] Constant Buffer를 사용한 동적 렌더링 (회전하는 삼각형)
- [ ] Root Signature에 CBV 바인딩
- [ ] DX12DescriptorHeap 개선 (CBV/SRV/UAV 통합)
- [ ] 카메라 시스템
- [ ] 텍스처 로딩
- [ ] 기본 조명

---

## 2025-10-26 - Mesh/Material 추상화 및 PSO 캐싱 시스템

### Overview
HelloTriangle 샘플의 하드코딩된 렌더링 코드를 Mesh, Material, DX12PipelineStateCache 클래스로 리팩토링.

### Implementation

**Mesh 클래스**
- DX12VertexBuffer, DX12IndexBuffer, D3D12_INPUT_LAYOUT_DESC 소유
- 자신의 정점 레이아웃에 대한 완전한 책임

**Material 클래스**
- 렌더링 상태(Blend, Rasterizer, DepthStencil) 정의
- 셰이더 경로와 엔트리 포인트 저장
- GPU 리소스 생성하지 않음

**DX12PipelineStateCache 클래스**
- Material Hash + Input Layout Hash + Root Signature로 PSO 캐싱
- GetOrCreatePipelineState() 패턴으로 중복 생성 방지

### Benefits
- 명확한 책임 분리 (Mesh, Material, PSO 관리)
- PSO 캐싱으로 런타임 생성 오버헤드 감소
- 확장 가능한 구조 (다양한 Mesh/Material 조합 지원)

### Lessons Learned
- Mesh가 Input Layout을 직접 소유하여 정점 타입 불일치 방지
- PSO 생성 비용이 크므로 캐싱이 필수적
- 하드코딩 → 패턴 파악 → 추상화 순서의 점진적 리팩토링 효과적

### Next Steps
- [ ] Constant Buffer
- [ ] 카메라 시스템
- [ ] 텍스처 로딩
- [ ] 기본 조명

---

## 2025-10-26 - 첫 삼각형 렌더링 완성

### Tasks
- [x] Root Signature 구현
- [x] Pipeline State Object (PSO) 구현
- [x] Shader 컴파일 시스템 (HLSL)
- [x] Input Layout 정의
- [x] 09_HelloTriangle 샘플 완성

### Decisions

**Root Signature**
- Empty Root Signature (파라미터 없음)
- INPUT_ASSEMBLER 플래그로 Vertex Input 활성화

**PSO 구성**
- Rasterizer: 백페이스 컬링, Solid Fill
- Blend: 알파 블렌딩 비활성화
- Depth: 깊이 테스트 비활성화
- Shader Model 5.1 (런타임 컴파일)

**Input Layout**
- POSITION: FLOAT3, COLOR: FLOAT4

### Issues Encountered

**셰이더 경로** - 작업 디렉토리 조정으로 해결  
**Vertex Stride** - sizeof 사용, POSITION(12) + COLOR(16)

### Code Statistics
- 새 클래스: 3개 (RootSignature, PipelineState, ShaderCompiler)
- 새 샘플: 1개 (09_HelloTriangle)
- 추가 코드: ~600줄
- 빌드 경고: 0개

### Visual Confirmation
- 삼각형 렌더링 성공 (빨강-초록-파랑 색상 보간)

### Next Steps

**Phase 2: Graphics (66% 완료)**
- [x] DirectX 12 초기화
- [x] Vertex/Index Buffer
- [x] Root Signature & PSO
- [x] 첫 삼각형 렌더링
- [ ] 코드 구조 개선
- [ ] Constant Buffer
- [ ] 텍스처 시스템

**즉시 다음: 코드 구조 개선**

현재 main이 너무 많은 초기화 직접 처리 → 리소스 관리 클래스 필요

1. **DX12Mesh 클래스** - VertexBuffer + IndexBuffer 통합
2. **DX12RenderResources 클래스** - PSO, RootSig, Shader 통합

**Constant Buffer 단계:**

3. **DX12Material 클래스** - PSO, RootSig, Shader, CBV 통합

**최종 목표 구조:**
```cpp
DX12Material basicMaterial;  // PSO, RootSig, Shader, CBV
DX12Mesh triangleMesh;       // Vertex, Index Buffer

basicMaterial.SetMatrix("MVP", mvpMatrix);
basicMaterial.Bind(cmdList);
triangleMesh.Bind(cmdList);
triangleMesh.Draw(cmdList);
```

---

## 2025-10-23 - Vertex Buffer, Index Buffer 및 최소 Renderer 구현

### Tasks
- [x] DX12VertexBuffer 클래스 구현
- [x] DX12IndexBuffer 클래스 구현
- [x] DX12Renderer 클래스 구현 (최소 구조)
- [x] DX12CommandQueue에 Fence 관리 통합
- [x] 코딩 컨벤션 적용 (주석, 네이밍)

### Decisions

**버퍼 업로드 전략**
- Default Heap (GPU 전용) + Upload Heap (임시) 패턴 사용
- WaitForIdle()로 즉시 동기화 (학습 단계에 적합)
- GPU 복사 완료 후 Upload Buffer 즉시 해제
- 향후: DX12ResourceUploader 패턴으로 배치 업로드 개선 예정

**인덱스 포맷 지원**
- 16bit (R16_UINT)와 32bit (R32_UINT) 모두 지원
- 65,535개 미만 버텍스는 16bit 권장
- 초기화 시점에 포맷 검증

**리소스 상태 전이**
- VertexBuffer: COPY_DEST → VERTEX_AND_CONSTANT_BUFFER
- IndexBuffer: COPY_DEST → INDEX_BUFFER
- 복사 작업 후 명시적 Barrier 필요

**프레임 동기화 구조**
- DX12Renderer가 프레임별 Fence 값 배열로 관리
- Ring Buffer 방식으로 프레임 인덱스 순환 (0→1→2→0)
- 상수 버퍼 추가 시 FrameResource 구조체로 확장 예정

**CommandQueue Fence 통합**
- ExecuteCommandLists()가 Fence 값 반환
- WaitForIdle() - 모든 작업 완료 대기
- WaitForFenceValue() - 특정 Fence 값 대기
- 단일 Fence 이벤트 핸들 사용

### Implementation

**DX12VertexBuffer**
```cpp
class DX12VertexBuffer
{
    ComPtr<ID3D12Resource> mVertexBuffer;   // Default Heap
    ComPtr<ID3D12Resource> mUploadBuffer;   // Upload Heap (임시)
    D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
};
```

**DX12IndexBuffer**
```cpp
class DX12IndexBuffer
{
    ComPtr<ID3D12Resource> mIndexBuffer;
    D3D12_INDEX_BUFFER_VIEW mIndexBufferView;
    DXGI_FORMAT mIndexFormat;  // R16_UINT 또는 R32_UINT
};
```

**DX12Renderer (최소 구조)**
```cpp
class DX12Renderer
{
    std::array<Core::uint64, FRAME_BUFFER_COUNT> mFrameFenceValues = {0};
    Core::uint32 mCurrentFrameIndex = 0;
};
```

**초기화 흐름**
1. Default Heap에 버퍼 생성 (GPU 전용)
2. Upload Heap에 임시 버퍼 생성
3. Map/Unmap으로 CPU 데이터 복사
4. GPU 복사 명령 기록
5. 리소스 상태 전이 (Barrier)
6. Command List 실행 및 완료 대기
7. Upload Buffer 해제
8. Buffer View 초기화

### Issues Encountered

**Upload Buffer 생명 주기**
- 고민: Shutdown()까지 유지 vs 즉시 해제
- 결정: WaitForIdle() 후 즉시 해제
- 이유: 단순하고 안전하며 메모리 효율적

**Fence 값 추적 설계**
- 고민: FrameResource 구조체 vs 단순 배열
- 결정: 현재는 단순 배열 사용
- 이유: 현재는 Fence 값만 필요, 조기 추상화 방지

**리소스 상태 전이**
- COPY_DEST에서 사용 상태로 전이 필수
- Debug Layer가 누락된 전이 감지
- VertexBuffer와 IndexBuffer는 서로 다른 최종 상태 필요

### Lessons Learned

**DirectX 12 리소스 관리**
- 모든 것이 명시적: 할당, 상태, 생명 주기
- Default Heap + Upload Heap 패턴이 표준
- GPU 복사 완료 후 Upload 리소스 해제 가능

**CPU-GPU 동기화**
- Fence 값은 "작업 영수증" 역할
- WaitForIdle()은 단순하지만 CPU 차단 (학습 단계 적합)
- 프레임별 Fence로 CPU-GPU 병렬화 가능 (향후 최적화)

**버퍼 타입 유사성**
- VertexBuffer와 IndexBuffer는 90% 동일 구현
- 주요 차이: View 구조체, 상태, 데이터 포맷
- 향후 Template/Base 클래스로 중복 제거 가능

### Code Statistics
- 새 클래스: 3개 (DX12VertexBuffer, DX12IndexBuffer, DX12Renderer)
- 수정 클래스: 1개 (DX12CommandQueue)
- 빌드 경고: 0개

### Notes

**현재 상태**
- 지오메트리 데이터를 GPU로 업로드 가능
- 렌더링 루프를 위한 Fence 동기화 준비 완료
- 실제 렌더링은 아직 불가 (Root Signature, PSO, Shader 필요)

**성능 고려사항**
- 현재: WaitForIdle()로 CPU 완전 차단
- 삼각형 업로드: < 1ms (GPU 동기화 포함)
- 향후: DX12ResourceUploader 패턴으로 배치 업로드 최적화

### Next Steps

**Phase 2: Graphics (50% 완료)**
- [x] DirectX 12 초기화
- [x] Fence 관리가 통합된 Command Queue
- [x] Vertex Buffer 구현
- [x] Index Buffer 구현
- [x] 최소 Renderer 구조
- [ ] Root Signature (파이프라인 인터페이스)
- [ ] Pipeline State Object (PSO)
- [ ] Shader 컴파일 (HLSL)
- [ ] Input Layout 정의
- [ ] 첫 삼각형 렌더링 (09_HelloTriangle 샘플)

**즉시 다음: Root Signature**
- 셰이더 파라미터 레이아웃 정의
- Descriptor Table (향후: 텍스처, 상수 버퍼)
- Root Constants (향후: Draw별 데이터)

---

## 2025-10-22 - Vertex/Index Buffer 및 Shader 컴파일 시스템 (재시작)

### Overview
이전 Vertex/Index Buffer 구현을 롤백하고 설계부터 재구상. 동기화 구조 개선 이후 더 나은 아키텍처로 재시작.

### Tasks
- [ ] DX12VertexBuffer/IndexBuffer 클래스 구현
- [ ] DX12ShaderCompiler 클래스 구현
- [ ] BasicShader.hlsl 작성
- [ ] 09_HelloTriangle 샘플 생성

### Decisions

**버퍼 클래스 분리**
- VertexBuffer와 IndexBuffer 별도 구현
- 이유: View 구조 차이, 타입 안전성

**Upload Buffer 패턴**
- DEFAULT Heap + UPLOAD Heap 조합
- Upload Buffer를 멤버로 보관 (Command List 실행까지 유지)

**셰이더 컴파일**
- 런타임 컴파일 (빠른 반복 개발)
- Shader Model 5.1

### Next Steps
1. DX12VertexBuffer 구현
2. DX12IndexBuffer 구현
3. DX12ShaderCompiler 구현
4. BasicShader.hlsl 작성
5. 09_HelloTriangle 통합 테스트

---

## 2025-10-22 - DirectX 12 초기화 구조 개선

### Tasks
- [x] Device RTV Descriptor Heap → SwapChain 이동
- [x] Factory 버전 쿼리 (Factory5, Factory6) 추가
- [x] WaitForIdle 제거, WaitForFenceValue 동기화로 전환
- [x] Frame Resource 개념 도입 (Fence Value 추적)
- [x] 이중 인덱싱 시스템 정립

### Decisions

**RTV Descriptor Heap 소유권 이동**
- Device → SwapChain으로 이동
- 이유: 백 버퍼와 RTV의 생명주기 일치, 책임 분리

**동기화 전략 변경**
- WaitForIdle → WaitForFenceValue
- 기대 효과: CPU 유휴 시간 최소화, CPU-GPU 병렬 작업, 프레임 지연 감소

**이중 인덱싱**
- `currentFrameIndex`: CPU 순차 인덱스 (Context, Fence)
- `backBufferIndex`: GPU 비순차 인덱스 (RTV, Back Buffer)

### Implementation

**렌더링 루프 구조**
```cpp
// main 함수에서 관리 (향후 FrameResource 클래스화 예정)
uint64 frameResource[FRAME_BUFFER_COUNT] = {};
uint32 currentFrameIndex = 0;

while (running)
{
    // 1. CPU 대기
    mQueue->WaitForFenceValue(frameResource[currentFrameIndex]);
    
    // 2. 백 버퍼 인덱스 획득
    uint32 backBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
    
    // 3-5. 명령 기록 및 제출
    // ...
    
    // 6. Fence Value 저장
    frameResource[currentFrameIndex] = mQueue->Signal();
    
    // 7. 인덱스 갱신
    mSwapChain->MoveToNextFrame();
    currentFrameIndex = (currentFrameIndex + 1) % FRAME_BUFFER_COUNT;
}
```

### Code Statistics
- 주요 수정 파일: DX12Device, DX12SwapChain, DX12CommandQueue, 08_DXInit

### Issues Encountered
- 인덱스 혼동: currentFrameIndex와 backBufferIndex 용도 명확화로 해결
- Fence Value 초기화: frameResource 배열 0 초기화

### Next Steps
- [ ] Vertex/Index Buffer 구현
- [ ] Shader 컴파일 시스템
- [ ] Root Signature, PSO 생성

---

## 2025-10-15 - DirectX 12 Initialization Complete

### Overview
Implemented complete DirectX 12 initialization pipeline from Device creation to first rendering. The sample successfully displays a solid color (Cornflower Blue) on screen.

### Tasks Completed
- [x] **Step 2**: Device and Factory initialization
- [x] **Step 3**: Command Queue creation
- [x] **Step 4**: SwapChain creation
- [x] **Step 5**: Descriptor Heap creation (RTV)
- [x] **Step 6**: First rendering (Clear screen)

### Implementation Summary

**Step 2: Device & Factory (DX12Device)**
- Created DXGI Factory4 with debug flags
- Selected GPU adapter based on max dedicated VRAM
- Created D3D12 Device with Feature Level detection (12.1 → 11.0)
- Enabled Debug Layer in debug builds with break on errors

**Step 3: Command Queue (DX12CommandQueue)**
- Created Direct Command Queue for graphics commands
- Implemented Fence for GPU synchronization
- Added WaitForIdle() for simple frame synchronization
- Implemented ExecuteCommandLists() for command submission

**Step 4: SwapChain (DX12SwapChain)**
- Created IDXGISwapChain3 with double buffering
- Configured FLIP_DISCARD swap effect with tearing support
- Implemented back buffer resource management
- Added Present() and Resize() functionality
- Disabled Alt+Enter fullscreen toggle

**Step 5: Descriptor Heap (DX12DescriptorHeap)**
- Created RTV Descriptor Heap for render targets
- Implemented CPU/GPU descriptor handle management
- Created Render Target Views for each back buffer
- Added descriptor size and offset calculations

**Step 6: First Rendering (DX12CommandContext)**
- Created Command Allocator and Command List per frame
- Implemented command recording with Reset/Close
- Added Resource Barriers for state transitions (PRESENT ↔ RENDER_TARGET)
- Implemented ClearRenderTargetView with Cornflower Blue
- Completed frame execution with Present and synchronization

### Project Structure

```
Engine/
├── include/Graphics/
│   ├── DX12/
│   │   ├── d3dx12.h                    # Microsoft helper library
│   │   ├── DX12Device.h                # Device, factory, adapter
│   │   ├── DX12CommandQueue.h          # Command queue, fence
│   │   ├── DX12SwapChain.h             # SwapChain, back buffers
│   │   ├── DX12DescriptorHeap.h        # Descriptor management
│   │   └── DX12CommandContext.h        # Command allocator, list
│   └── GraphicsTypes.h                 # Common types, macros
└── src/
    └── DX12/
        ├── DX12Device.cpp
        ├── DX12CommandQueue.cpp
        ├── DX12SwapChain.cpp
        ├── DX12DescriptorHeap.cpp
        └── DX12CommandContext.cpp

Samples/08_DX12Init/
└── main.cpp                            # Render loop
```

### Key Design Decisions

**Platform Abstraction**
- Decided to use HWND directly instead of WindowHandle abstraction
- Reason: DirectX 12 is Windows-only, no cross-platform plans
- Conversion responsibility: Application layer (main.cpp)

**Double Buffering**
- Used FRAME_BUFFER_COUNT = 2 for double buffering
- Created 2 back buffers, 2 RTVs, 2 command contexts
- Triple buffering deemed unnecessary for learning phase

**Synchronization**
- Implemented simple WaitForIdle() per frame
- Per-frame fence management for future optimization
- Command context per back buffer for parallel recording

**Error Handling**
- GRAPHICS_THROW_IF_FAILED macro for HRESULT checks
- Comprehensive logging at all initialization steps
- Debug Layer enabled in debug builds for validation

**d3dx12.h Integration**
- Used `#pragma warning(push, 0)` to suppress external warnings
- Simplified resource barrier creation with CD3DX12 helpers
- Reduced boilerplate code by ~80%

### Code Statistics

**New Classes Added:**
- DX12Device (Device, factory, adapter management)
- DX12CommandQueue (Command submission, synchronization)
- DX12SwapChain (Presentation, back buffer management)
- DX12DescriptorHeap (Descriptor allocation, handle management)
- DX12CommandContext (Command recording)

### Rendering Pipeline Flow

```
Frame Start
    ↓
Reset Command Context
    ↓
Record Commands:
  - Barrier (PRESENT → RENDER_TARGET)
  - ClearRenderTargetView (Cornflower Blue)
  - Barrier (RENDER_TARGET → PRESENT)
    ↓
Close Command List
    ↓
Execute on Command Queue
    ↓
Present (SwapChain)
    ↓
Move to Next Frame
    ↓
Wait for GPU (Synchronization)
```

### Test Results

**Initialization Log:**
```
[INFO] DirectX 12 Device initialized successfully
[INFO] Feature Level: 12.1
[INFO] GPU: NVIDIA GeForce RTX 3080 (10.00 GB VRAM)
[INFO] SwapChain ready (1280 x 720, 2 buffers)
[INFO] RTV Descriptor Heap ready (2 descriptors)
[INFO] Command Contexts created (2 contexts)
[INFO] DirectX 12 initialization completed successfully!
[INFO] Rendering Cornflower Blue screen...
```

**Visual Confirmation:**
- Window displays solid Cornflower Blue color (#6495ED)
- Smooth 60 FPS rendering (VSync enabled)
- No artifacts or flickering
- ESC key exits cleanly

### Issues Encountered & Solutions

**Issue 1: d3dx12.h Warnings (C4324)**
- Problem: Structure padding warnings from external library
- Solution: Wrapped include with `#pragma warning(push, 0)`

**Issue 2: Incomplete Type Error (DX12CommandContext)**
- Problem: Forward declaration insufficient for `std::make_unique`
- Solution: Added `#include "Graphics/DX12/DX12CommandContext.h"` in cpp file

**Issue 3: WindowHandle Abstraction**
- Problem: Debate between Platform abstraction vs direct HWND
- Solution: Used HWND directly, conversion in application layer

### Lessons Learned

**DirectX 12 Complexity**
- Much more verbose than DirectX 11
- Explicit resource management essential
- State transitions critical for correctness

**d3dx12.h Value**
- Dramatically reduces boilerplate
- Industry-standard helper library
- Essential for learning phase

**Synchronization Importance**
- Simple WaitForIdle() works but inefficient
- Proper fence management needed for performance
- Per-frame resources eliminate stalls

**Debug Layer Benefits**
- Caught multiple state transition errors
- Validation messages invaluable
- Essential for learning DirectX 12

### Notes

- Project successfully renders first DirectX 12 output
- Foundation complete for advanced rendering features
- All initialization steps working reliably
- Ready to proceed with geometry rendering

### Next Steps

**Phase 2 Complete - Ready for Phase 3 (Basic Rendering)**

**09_HelloTriangle Sample:**
- [ ] Vertex Buffer creation
- [ ] Index Buffer creation
- [ ] Root Signature design
- [ ] Pipeline State Object (PSO)
- [ ] Shader compilation (HLSL)
- [ ] Input Layout definition
- [ ] First triangle rendering

--- 

## 2025-10-15 - Step 2: Device and Factory Initialization

### Tasks
- [x] Designed and implemented DX12Device class
- [x] Created DXGI Factory
- [x] Selected GPU Adapter (based on max VRAM)
- [x] Created D3D12 Device (Feature Level check)
- [x] Enabled Debug Layer (Debug builds)

### Decisions

**Class Design**
- Did not use Singleton pattern (kept as regular class)
- Explicit lifecycle management (Initialize/Shutdown)
- Non-copyable

**Adapter Selection Criteria**
- Select GPU with largest dedicated video memory (VRAM)
- Exclude Software Adapters
- Verify D3D12 Device creation capability

**Feature Level**
- Try in order: 12.1 → 12.0 → 11.1 → 11.0
- Use highest available level

**Debug Layer**
- Enable only in Debug builds
- Set Break on CORRUPTION and ERROR
- Pass Debug flags to Factory as well

### Implementation

**DX12Device Class**
```cpp
class DX12Device
{
public:
    bool Initialize(bool enableDebugLayer = true);
    void Shutdown();
    
    IDXGIFactory4* GetFactory() const;
    ID3D12Device* GetDevice() const;
    D3D_FEATURE_LEVEL GetFeatureLevel() const;
    
private:
    bool EnableDebugLayer();
    bool CreateFactory();
    bool SelectAdapter();
    bool CreateDevice();
    
    ComPtr<IDXGIFactory4> mFactory;
    ComPtr<IDXGIAdapter1> mAdapter;
    ComPtr<ID3D12Device> mDevice;
    D3D_FEATURE_LEVEL mFeatureLevel;
};
```

**Initialization Order**
1. Enable Debug Layer (Debug builds)
2. Create DXGI Factory
3. Select GPU Adapter
4. Create D3D12 Device

**Log Output**
- Selected GPU information (name, VRAM, system memory)
- Supported Feature Level
- Debug Layer activation status

### Test Results

**Execution Result**
```
[INFO] === 08_DX12Init Sample Started ===
[INFO] Window created successfully
[INFO] [DX12Device] Initializing DirectX 12 Device...
[INFO] [DX12Device] Debug Layer enabled successfully
[INFO] [DX12Device] DXGI Factory created successfully
[INFO] [DX12Device] Found compatible adapter: NVIDIA GeForce RTX 3080
[INFO] [DX12Device]   Dedicated Video Memory: 10.00 GB
[INFO] [DX12Device] D3D12 Device created with Feature Level: 12.1
[INFO] [DX12Device] DirectX 12 Device initialized successfully
[INFO] Press ESC to exit
```

- [x] Debug Layer enabled successfully
- [x] GPU information displayed correctly
- [x] Device created successfully (Feature Level 12.1)
- [x] Normal shutdown and resource cleanup verified

### Issues Encountered

**d3dx12.h Warnings**
- Issue: C4324 (structure padding) warnings
- Solution: Used `#pragma warning(push, 0)` to suppress warnings only for that header
- Did not modify file as it's Microsoft official library

### Notes
- Phase 2 (Graphics) started
- Next step: Command Queue creation
- Device initialization complete, ready for GPU communication

### Next Steps

**Step 3: Command Queue Creation**
- [ ] Implement DX12CommandQueue class
- [ ] Create Direct Command Queue
- [ ] Establish command execution foundation

**Step 4: SwapChain Creation**
- [ ] Implement DX12SwapChain class
- [ ] Manage Back Buffer resources
- [ ] Prepare Present functionality

**Step 5: Descriptor Heap Creation**
- [ ] Implement RTV Descriptor Heap
- [ ] Create RTVs for Back Buffers

**Step 6: First Rendering**
- [ ] Record Command List
- [ ] Clear screen (solid color background)
- [ ] Implement Present


---

## 2025-10-15 - Step 1: Project Structure Design and Setup

### Tasks
- [x] Created Graphics module project (Static Library)
- [x] Created 08_DX12Init sample project (Console Application)
- [x] Integrated d3dx12.h helper library
- [x] Completed basic build configuration and testing

### Decisions

**Graphics Module Structure**
- Implemented as Static Library to maintain same hierarchy as Core, Math, Platform
- RHI abstraction layer to be added later if needed

**d3dx12.h Usage**
- Included Microsoft official DirectX 12 helper library
- Header-only library with no external dependencies
- Simplifies struct initialization and improves code readability

**Warning Handling**
- Suppressed warnings for d3dx12.h as external library
- Used `#pragma warning(push, 0)` to disable warnings only for that header

### Implementation

**Project Structure**
```
Engine/Graphics/
├── include/Graphics/
│   ├── DX12/
│   │   └── d3dx12.h           # Microsoft official helper
│   └── GraphicsTypes.h         # Common types, constants, macros
└── src/
    └── Graphics.cpp

Samples/08_DX12Init/
└── main.cpp                    # Window creation and input handling
```

**GraphicsTypes.h**
- Includes DirectX 12 headers and library linking
- d3dx12.h warning suppression (`#pragma warning(push, 0)`)
- ComPtr type alias definition
- FRAME_BUFFER_COUNT constant definition (Double buffering)
- GRAPHICS_THROW_IF_FAILED macro definition

**main.cpp**
- Window creation (1280x720)
- Input handling (ESC key exit)
- Frame loop structure (Update → ProcessEvents → Reset)

### Test Results
- [x] Graphics.lib built successfully
- [x] 08_DX12Init.exe built successfully (0 warnings)
- [x] Window created normally
- [x] ESC key exit working correctly

---

## 2025-10-13 - Input System Implementation

### Tasks
- [x] Design Input class architecture
- [x] Implement keyboard/mouse state tracking
  - Double buffering system (current + previous frame)
  - IsKeyPressed/Down/Released distinction
- [x] Integrate Input with Window (window-owned instance)
- [x] Create 07_InputTest sample
- [x] Update documentation

### Decisions
- **Window-owned instance**: Each window has independent input state (not singleton)
- **Double buffering**: Track current + previous frame state to distinguish "pressed" vs "held"
- **Math::Vector2 integration**: Mouse position/delta use engine's vector type
- **Frame cycle pattern**: `Update()` → `ProcessEvents()` → Game Logic → `Reset()`
- **Mouse wheel normalization**: Win32 delta (±120) normalized to ±1.0

### Lessons Learned
- **Win32 Coordinate Sign Extension**: `LOWORD(lParam)` doesn't sign-extend for negative coordinates. Must cast to `short` first for multi-monitor support.
- **System Key Handling**: Alt key combinations require handling both `WM_KEYDOWN` and `WM_SYSKEYDOWN` messages separately.
- **State vs Event Distinction**: Keys/buttons are persistent state, but mouse wheel is a per-frame event requiring explicit `Reset()`.

### Future Improvements
- **Input Mapping System**: Rebindable keys with command pattern for user customization.
- **Text Input Support**: WM_CHAR handling for text fields and UI input.
- **Gamepad Support**: XInput integration for Xbox controllers.
- **Input Recording**: Record/playback functionality for replays and automated testing.

### Notes
- ~400 lines of code, 0 warnings (Level 4)
- Window-owned design supports future multi-window scenarios
- Double buffering pattern applicable to future input devices (gamepad, touch)
- Clean integration: Input state ↔ Window events ↔ Math types

### Next Steps (Phase 4: DirectX 12)
- [ ] **Graphics module structure** - Create DX12 wrapper project
- [ ] **Device initialization** - DX12Device, factory, adapter
- [ ] **SwapChain creation** - Double/triple buffering setup
- [ ] **Command Queue** - Graphics command submission
- [ ] **08_DX12Init sample** - First clear screen render

---

## 2025-10-12 - Platform Layer Implementation

### Tasks
- [x] Design Platform Layer architecture
- [x] Implement Window abstraction (interface + Win32 implementation)
- [x] Create 06_WindowTest sample
- [x] Update documentation

### Decisions
- **Header organization**: All headers in `include/` for consistency with Core/Math
  - Chose consistency over encapsulation for learning project
- **Window architecture**: Abstract interface + Factory pattern
  - `Window` interface → `Win32Window` implementation → `CreatePlatformWindow()` factory
- **Win32 message handling**: Static WindowProc + instance HandleMessage pattern
- **Event system**: Simple callback pattern (full event queue deferred)
- **Message loop**: Non-blocking `PeekMessage` for real-time rendering

### Issues Encountered
- **Windows.h pollution**: Forward-declared `HWND__`/`HINSTANCE__` in headers, include only in .cpp
- **Unicode conversion**: `MultiByteToWideChar` for UTF-8 → UTF-16 conversion
- **Window sizing**: Used `AdjustWindowRectEx` to account for borders

### Performance
- Window creation: ~5 ms
- ProcessEvents: ~0.01-0.05 ms per frame
- Memory overhead: ~200 bytes per window
- Platform.lib: ~45 KB (Debug)

### Code Statistics
- **New files**: 6 (3 headers, 3 implementations)
- **Lines of code**: ~650
- **Build time**: < 2 seconds

### Notes
- Clean separation between interface and implementation
- Logging integration from day 1 simplifies debugging
- Ready for DirectX 12 integration

### Next Steps (Recommended Priority)
- [ ] **Input System** - Keyboard/mouse state tracking (Phase 3)
- [ ] **DirectX 12 initialization** - Device, SwapChain, first clear screen (Phase 4)
- [ ] High-DPI awareness (low priority)
- [ ] Multi-monitor support (low priority)

---

## 2025-10-11 - Logging System Implementation

### Tasks
- [x] Design and implement logging system
  - Logger singleton (thread-safe)
  - Sink pattern (Console + File)
  - Macro-based API (LOG_INFO, LOG_ERROR, etc.)
- [x] Create 05_LoggingTest sample
- [x] Apply logging to Memory allocators
- [x] Update documentation

### Decisions
- **Singleton pattern**: Global access with thread safety
- **Sink pattern**: Extensible output system
- **Printf-style formatting**: Simple, no dependencies
- **Macro API**: Compile-time optimization (Trace/Debug removed in Release)
- **Console colors**: Visual log level distinction (Windows Console API)

### Issues Encountered
- **localtime() warning**: Replaced with localtime_s()
- **Macro variadic args**: Used ##__VA_ARGS__ for optional arguments
- **File path in macros**: Strip path with GetFileName() helper

### Performance
- Debug overhead: ~50 microseconds/log
- Release overhead: 0 (Trace/Debug compiled out)
- Memory allocators now have comprehensive logging

### Notes
- ~300 lines of code, 0 warnings (Level 4)
- Async logging deferred until rendering performance profiling
- Log rotation deferred until long-running applications needed
- JSON sink deferred until profiling tools required

### Next Steps (Recommended Priority)
- [ ] **Platform layer** - Window creation (Win32 API)
- [ ] **DirectX 12 initialization** - Device, SwapChain, basic render loop
- [ ] ECS TransformComponent (using Math library)

---

## 2025-10-11 - Math Library with DirectXMath

### Tasks
- [x] Create Math project (Static Library)
- [x] Wrap DirectXMath for convenient usage
- [x] Implement MathTypes.h
  - Type aliases (Vector2/3/4, Matrix3x3/4x4, Quaternion)
  - SIMD types (VectorSIMD, MatrixSIMD)
  - Common constants (PI, DEG_TO_RAD, etc.)
  - Load/Store helpers
- [x] Implement MathUtils.h
  - Vector operations (Add, Dot, Cross, Normalize, etc.)
  - Matrix operations (Multiply, Inverse, Transpose, etc.)
  - Transform matrices (Translation, Rotation, Scaling)
  - Camera matrices (LookAt, Perspective, Orthographic)
  - Quaternion operations (FromEuler, Slerp, RotateVector, etc.)
  - Utility functions (Clamp, DegToRad, Lerp)
- [x] Create 04_MathTest sample
  - Vector operations test
  - Matrix operations test
  - Transform composition test
  - Camera matrices test
  - Quaternion operations test

### Decisions
- **DirectXMath over custom**: Save time, production-quality SIMD optimization
  - Trade-off: Less learning opportunity, but more practical
- **Storage vs SIMD types**: Separate types for memory and calculation
  - Storage (Vector3): 12 bytes, array-friendly
  - SIMD (VectorSIMD): 16 bytes, fast calculation
- **Wrapper pattern**: Convenience functions around DirectXMath
  - Hide complex Load/Store operations
  - Cleaner API for common operations
- **Transform.h deferred**: Will implement as ECS TransformComponent later
  - Better fit with entity-component architecture

### Issues Encountered
- **Type confusion**: Initially unclear when to use storage vs SIMD types
  - Solution: Storage for data structures, SIMD for calculations
  - Load/Store helpers make conversion easy
- **Quaternion representation**: DirectXMath uses XMVECTOR for quaternions
  - Solution: XMFLOAT4 for storage, XMVECTOR for calculation

### Notes
- DirectXMath already included with Windows SDK (no external dependencies)
- All functions are inline (header-only, no .cpp needed)
- Math.lib is interface library (no actual binary output)
- Test demonstrates all common operations successfully
- ~400 lines total (headers + test), 0 warnings

### Next Steps
- [ ] Create Logging system (console logger)
- [ ] Begin Platform layer (Window creation)
- [ ] Implement ECS TransformComponent (using Math library)
- [ ] Create unified memory manager facade
- [ ] Add memory statistics/profiling

---

## 2025-10-11 - Pool & Stack Allocator Implementation

### Tasks
- [x] Implement PoolAllocator
  - Fixed-size chunk allocation with free-list
  - O(1) allocation and deallocation
- [x] Implement StackAllocator  
  - LIFO allocation with marker system
  - Scoped allocation support
- [x] Create test samples (02_PoolAllocatorTest, 03_StackAllocatorTest)
- [x] Apply comment conventions to all Memory headers

### Decisions
- **PoolAllocator alignment**: Limited to 8 bytes for now (free-list requires pointer storage)
  - 16-byte SIMD alignment deferred until optimization phase
- **StackAllocator overhead**: 8 bytes per allocation for padding metadata
- **Comment policy**: Self-documenting code prioritized, minimal comments in headers

### Issues Encountered
- **Alignment assertion failure**: `DEFAULT_ALIGNMENT` (16) vs `sizeof(void*)` (8) mismatch
  - Solution: Explicitly specify 8-byte alignment in test code
  - Deferred: Upgrade to 16-byte support with `_aligned_malloc` (premature optimization)

### Notes
- Three allocator patterns complete: Linear (fastest), Pool (individual free), Stack (scoped)
- All avoid OS calls after init (10-100x faster than new/delete)
- ~600 lines total (allocators + tests), 0 warnings
- Performance: PoolAllocator ~2ms, LinearAllocator ~0.5ms, StackAllocator ~0.8ms (10,000 allocs)

### Next Steps
- [ ] Create unified memory manager facade
- [ ] Add memory statistics/profiling
- [ ] Document allocator usage guidelines
- [ ] Begin Math library (Vector, Matrix, Quaternion)
- [ ] Create Logging system

---

## 2025-10-10 - Error Handling & Memory Safety

### Tasks
- [x] Implement Assert.h for error handling
  - CORE_ASSERT (debug only, uses assert)
  - CORE_VERIFY (always checks, throws exception)
- [x] Prevent memory leaks in LinearAllocator
  - Delete copy constructor and copy assignment
  - Replace assert with new macros

### Decisions
- **CORE_ASSERT vs CORE_VERIFY**: ASSERT for debug checks (compiled out in release), VERIFY for critical checks (always executed)
- **No copy semantics**: LinearAllocator owns raw memory - copying would cause double-free
- **Fail-fast on OOM**: Out-of-memory throws exception rather than returning nullptr

### Issues Encountered
- **ASSERT vs VERIFY usage**: Initially unclear when to use which
  - Solution: ASSERT for "should never happen in correct code", VERIFY for "might fail due to external factors"
- **OOM handling**: Debated between return nullptr vs throw exception
  - Decision: Fail fast - OOM is unrecoverable in our use case

### Notes
- Assert.h pattern will be reused across all Core modules
- Copy prevention pattern applies to all future allocators
- All existing MemoryTest cases still pass
- No performance impact in release builds

### Next Steps
- [ ] Implement PoolAllocator for fixed-size object pools
- [ ] Implement StackAllocator for scoped allocations
- [ ] Document assertion guidelines in CodingConvention.md
- [ ] Add memory profiling/statistics (peak usage, leak detection)
- [ ] Create Logging system (console and file logger)
- [ ] Begin Math project (Vector, Matrix, Quaternion)

---

## 2025-10-09 - Memory System Implementation & Testing

### Tasks
- [x] Design and implement Memory subsystem
  - MemoryConfig.h (alignment constants, helper functions)
  - Allocator.h (base interface with tracking)
  - LinearAllocator (header + implementation)
- [x] Create MemoryTest sample project
  - Configure project settings (executable, links Core.lib)
  - Write test suite for LinearAllocator
  - Build and run successful

### Decisions
- **LinearAllocator first**: Simpler than PoolAllocator, most commonly needed for frame allocations
- **16-byte default alignment**: For SIMD optimization and cache performance
- **No individual deallocation**: LinearAllocator design - bulk reset only for simplicity and speed
- **Sample project pattern**: Created Samples/MemoryTest for testing static library code

### Issues Encountered
- **Static library testing**: Initially unclear how to test Core.lib
  - Solution: Created separate executable sample project
- **Linker dependencies**: Required careful configuration of library paths and dependencies
- **Build order**: Set project dependencies so Core builds before MemoryTest

### Notes
- LinearAllocator: O(1) allocation, 10-100x faster than new/delete
- Test results: All alignment and allocation tests passed
- Memory tracking works correctly (size and count)
- Sample project pattern will be reused for future features
- ~300 lines of code (Memory system + tests), 0 warnings

### Next Steps
- [ ] Implement PoolAllocator for fixed-size object pools
- [ ] Implement StackAllocator for scoped allocations
- [ ] Add memory profiling/statistics (peak usage, leak detection)
- [ ] Create Logging system (console and file logger)
- [ ] Begin Math project (Vector, Matrix, Quaternion)

---

## 2025-10-05 - Core Project Setup & Basic Types

### Tasks
- [x] Create Core project (Static Library)
- [x] Configure project settings
  - Output: `bin\$(Configuration)\`, Intermediate: `intermediate\$(Configuration)\$(ProjectName)\`
  - C++20, Additional includes: `$(ProjectDir)include\`
  - Warning Level 4, Runtime Library: /MDd (Debug), /MD (Release)
- [x] Implement Types.h
  - Basic type aliases (uint8, uint16, uint32, int8, int16, int32, float32, float64)
  - Smart pointer aliases (UniquePtr, SharedPtr, WeakPtr)
  - Container aliases (Vector, Array, String)
  - Helper functions (MakeUnique, MakeShared)
- [x] Add Core.cpp for module initialization
- [x] Build verification successful

### Decisions
- **Simplified output paths**: No platform suffix (x64 only target)
- **Core namespace**: All type aliases in `Core::` to avoid std conflicts
- **Smart pointers included**: WeakPtr added for circular reference handling

### Issues Encountered
- C/C++ property tab not visible until .cpp file added
- Initially added redundant `_DEBUG`/`NDEBUG` preprocessor definitions (auto-defined by VS)

### Notes
- First successful build: `bin/Debug/Core.lib`
- ~50 lines of code, 0 warnings, < 1s build time
- Types.h provides foundation for all future modules

### Next Steps
- [ ] Implement basic Logging system (Logger.h/cpp)
- [ ] Add Macros.h (Assert, platform detection)
- [ ] Create Math project structure
- [ ] Design Memory allocator interfaces

---

## 2025-10-05 - Project Initialization

### Tasks
- [x] Set up project directory structure
- [x] Write initial documentation
  - README.md (English)
  - README.ko.md (Korean)
  - Architecture.md (English)
  - Architecture.ko.md (Korean)
  - CodingConvention.md (Korean)
  - DevelopmentLog.md (English)
- [x] Add MIT License
- [x] Configure Git repository
- [x] Create `.gitignore` for Visual Studio and DirectX projects
- [x] Set up Visual Studio 2022 solution file (empty projects)

### Decisions
- **Architecture**: Chose ECS (Entity Component System) as core architecture
- **Graphics API**: DirectX 12 as primary rendering API
- **Language**: C++20 for modern features and performance
- **Platform**: Windows-first, potential cross-platform later
- **Documentation**: Bilingual (English/Korean) for accessibility

### Notes
- Project structure designed with modularity in mind
- Planned 6 development phases from foundation to tools
- Solution created with empty project structure ready for implementation
- All core documentation completed before first commit

### Next Steps
- [ ] Create Core module project (Types, Memory, Logging)
- [ ] Implement basic Math library (Vector, Matrix)
- [ ] Set up unit testing framework
- [ ] Begin ECS framework implementation

---

## Template for Future Entries

```markdown
## YYYY-MM-DD - Brief Description

### Tasks
- [ ] Task 1
- [ ] Task 2
- [x] Completed task

### Decisions
- Decision point and rationale

### Issues Encountered
- Problem and solution

### Notes
- Additional observations or learnings

### Next Steps
- [ ] Upcoming task 1
- [ ] Upcoming task 2
```

---

## Development Phases

### Phase 0: Project Setup ✅
- **Status**: Complete
- **Duration**: 2025-10-05
- **Key Deliverables**: Documentation, project structure

### Phase 1: Foundation Systems 🔄
- **Status**: Not Started
- **Target**: TBD
- **Key Goals**: Core systems, Math library, Basic ECS

### Phase 2: Graphics
- **Status**: Not Started
- **Key Goals**: DirectX 12 initialization, Basic rendering pipeline

### Phase 3: Advanced ECS
- **Status**: Not Started
- **Key Goals**: Component queries, System dependencies

### Phase 4: Gameplay Systems
- **Status**: Not Started
- **Key Goals**: Physics, AI pathfinding, Terrain generation

### Phase 5: Advanced Rendering
- **Status**: Not Started
- **Key Goals**: PBR materials, Shadow mapping, Post-processing

### Phase 6: Tools
- **Status**: Not Started
- **Key Goals**: Asset pipeline, Editor prototype

---

## Statistics

- **Project Start Date**: 2025-10-05
- **Total Commits**: 0 (Initial commit pending)
- **Lines of Code**: 0
- **Modules Completed**: 0/8

---

## Resources & Learning

### Books
- Game Engine Architecture by Jason Gregory
- 3D Game Programming with DirectX 12 by Frank Luna

### Documentation
- [DirectX 12 Official Docs](https://docs.microsoft.com/en-us/windows/win32/direct3d12/)
- [Learn OpenGL](https://learnopengl.com/)

### References
- [EnTT - ECS Library](https://github.com/skypjack/entt)
- [Game Programming Patterns](https://gameprogrammingpatterns.com/)

---

**최종 업데이트**: 2025-10-26