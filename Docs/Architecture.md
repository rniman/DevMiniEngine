# DevMiniEngine 아키텍처 문서

## 목차
1. [개요](#개요)
2. [설계 원칙](#설계-원칙)
3. [모듈 아키텍처](#모듈-아키텍처)
4. [ECS 아키텍처](#ecs-아키텍처)
5. [프레임 플로우](#프레임-플로우)
6. [메모리 관리](#메모리-관리)
7. [렌더링 아키텍처](#렌더링-아키텍처)
8. [로드맵](#로드맵)
9. [구현 진행 상황](#구현-진행-상황)
10. [참고 자료](#참고-자료)

---

## 개요

DevMiniEngine은 DirectX 12 기반의 학습용 게임 엔진 프로젝트입니다. ECS(Entity Component System) 아키텍처와 다양한 그래픽스 기술을 학습하고 실험하는 것을 목표로 합니다.

### 핵심 기술 스택
- **Graphics API**: DirectX 12
- **언어**: C++20
- **빌드 시스템**: Visual Studio 2022
- **아키텍처**: 모듈식 설계, ECS (구현 완료)

### 프로젝트 목표
1. DirectX 12 저수준 API 완전 이해
2. ECS 아키텍처 패턴 학습 및 구현
3. 최신 그래픽스 기술 실험 (PBR, IBL, Shadow Mapping 등)
4. 포트폴리오용 데모 제작

---

## 설계 원칙

### 1. 모듈화 (Modularity)
각 시스템은 명확한 책임과 인터페이스를 가진 독립적인 모듈로 구성됩니다.

**모듈 간 의존성 규칙:**
```
Framework → ECS → Graphics → Platform → Core
              ↘ Math   ↗
```

### 2. 추상화 레벨 분리
- **High-Level**: Framework (Application, ResourceManager)
- **Mid-Level**: ECS (Entity, Component, System), Graphics (Mesh, Material, Texture)
- **Low-Level**: DX12 (Device, CommandQueue, DescriptorHeap)

### 3. What/How 분리
- **What**: ECS System이 무엇을 처리할지 결정 (논리)
- **How**: Renderer가 어떻게 렌더링할지 구현 (DirectX 12)

### 4. 메모리 효율성
- 커스텀 할당자 (Linear, Pool, Stack)
- CPU-GPU 메모리 동기화 최적화
- 리소스 캐싱 및 재사용

---

## 모듈 아키텍처

### 디렉토리 구조
```
DevMiniEngine/
├── Engine/
│   ├── include/
│   │   ├── Core/                        # Core 레이어
│   │   │   ├── Memory/                  # 메모리 관리
│   │   │   ├── Logging/                 # 로깅 시스템
│   │   │   └── Timing/                  # 타이머 시스템
│   │   ├── Math/                        # Math 레이어
│   │   ├── Platform/                    # Platform 레이어
│   │   │   ├── Window/                  # 윈도우 관리
│   │   │   └── Input/                   # 입력 처리
│   │   ├── Graphics/                    # Graphics 레이어
│   │   │   ├── DX12/                    # DirectX 12 구현
│   │   │   └── Camera/                  # 카메라 시스템
│   │   ├── ECS/                         # ECS 레이어
│   │   │   ├── Components/              # Component 정의
│   │   │   └── Systems/                 # System 구현
│   │   └── Framework/                   # Framework 레이어
│   │       ├── Application.h            # 애플리케이션 베이스
│   │       ├── Resources/               # 리소스 관리
│   │       └── DebugUI/                 # ImGui 통합
│   │
│   ├── src/                             # 모든 모듈의 구현
│   │
│   ├── Core/                            # Core 모듈 프로젝트
│   ├── Math/                            # Math 모듈 프로젝트
│   ├── Platform/                        # Platform 모듈 프로젝트
│   ├── Graphics/                        # Graphics 모듈 프로젝트
│   ├── ECS/                             # ECS 모듈 프로젝트
│   └── Framework/                       # Framework 모듈 프로젝트
│
├── Samples/                             # 샘플 프로젝트
│   ├── 01_MemoryTest/
│   ├── ...
│   ├── 09_ECSRotatingCube/              # ECS 기반 회전 큐브
│   └── 10_PhongLighting/                # Phong + 계층 구조 데모
│
├── Assets/                              # 에셋
│   ├── Textures/                        # 텍스처 파일
│
└── Docs/                                # 문서
    ├── Architecture.md
    ├── CodingConvention.md
    └── DevelopmentLog.md
```

### 모듈 설명

#### 1. Platform 레이어 (구현 완료)
**책임**: OS 종속적인 기능 추상화

**핵심 클래스:**

```cpp
// Platform/include/Platform/Window/Window.h
class Window
{
public:
    bool Initialize(const WindowDesc& desc);
    void ProcessMessages();
    void Show();
    void Hide();
    
    HWND GetHandle() const { return mHandle; }
    uint32 GetWidth() const { return mWidth; }
    uint32 GetHeight() const { return mHeight; }
    
private:
    HWND mHandle = nullptr;
    uint32 mWidth = 0;
    uint32 mHeight = 0;
};
```

**테스트 커버리지**: 06_WindowTest, 07_InputTest

**기능:**
- Win32 Window 관리 (생성, 메시지 처리, 이벤트)
- 입력 시스템 (키보드, 마우스)
- 프레임 시작/종료 시 입력 상태 업데이트
- 델타 타임 측정

#### 2. Math 레이어 (구현 완료)
**책임**: 수학적 기본 요소 및 연산

**상태**: SIMD 최적화와 함께 완전히 구현됨

**기능:**
- SIMD 최적화 벡터/행렬 연산 (DirectXMath 래퍼)
- Vector2, Vector3, Vector4 (클래스, 연산자 오버로딩)
- Matrix4x4 (행 우선)
- 회전을 위한 Quaternion
- Transform 유틸리티 (Translation, Rotation, Scaling)
- 카메라 행렬 (LookAt, Perspective, Orthographic)
- 일반적인 수학 유틸리티 (Lerp, Clamp, DegToRad 등)

**테스트 커버리지**: 04_MathTest

```cpp
// Math/include/Math/MathTypes.h
struct Vector3
{
    float32 x, y, z;
    
    Vector3 operator+(const Vector3& other) const;
    Vector3 operator-(const Vector3& other) const;
    Vector3 operator*(float32 scalar) const;
    float32 Dot(const Vector3& other) const;
    Vector3 Cross(const Vector3& other) const;
    Vector3 Normalized() const;
    
    static Vector3 Zero();
    static Vector3 One();
    static Vector3 Up();
    static Vector3 Forward();
    static Vector3 Right();
};

struct Matrix4x4
{
    float32 m[4][4];
    
    Matrix4x4 operator*(const Matrix4x4& other) const;
    static Matrix4x4 Identity();
    static Matrix4x4 Translation(const Vector3& t);
    static Matrix4x4 RotationQuaternion(const Quaternion& q);
    static Matrix4x4 Scaling(const Vector3& s);
};
```

**설계 결정사항:**
- DirectXMath를 기반으로 사용 (프로덕션 품질 SIMD)
- 저장 타입(클래스)과 SIMD 타입(XMVECTOR) 분리
- 연산자 오버로딩으로 직관적인 수학 표현
- 모든 함수 인라인 (헤더 온리, 오버헤드 없음)

#### 3. Core 레이어 (구현 완료)
**책임**: 엔진 기반 시스템

**서브시스템:**

##### 메모리 관리 (구현 완료)
**상태**: 완전히 구현되고 테스트됨

**할당자들:**
- **LinearAllocator**: 범프 포인터 할당, O(1) 할당
  - 사용 사례: 프레임 임시 데이터, 레벨 로딩
  - 개별 할당 해제 불가 (대량 리셋만 가능)
  - 테스트: 01_MemoryTest
  
- **PoolAllocator**: 고정 크기 객체 풀, O(1) 할당/해제
  - 사용 사례: 엔티티, 컴포넌트, 파티클
  - 재사용을 위한 프리 리스트 구현
  - 테스트: 02_PoolAllocatorTest
  
- **StackAllocator**: 마커를 사용한 LIFO 할당 패턴
  - 사용 사례: 중첩된 스코프 할당
  - 마커를 통한 부분 해제 지원
  - 테스트: 03_StackAllocatorTest

```cpp
// Core/include/Core/Memory/LinearAllocator.h
class LinearAllocator : public Allocator
{
public:
    explicit LinearAllocator(size_t size);
    void* Allocate(size_t size, size_t alignment = DEFAULT_ALIGNMENT) override;
    void Reset() override;  // 한 번에 모두 해제
    
private:
    void* mMemory;
    size_t mSize;
    size_t mOffset;
};
```

**기능:**
- 커스텀 메모리 정렬 지원 (SIMD를 위한 16바이트 기본값)
- 포괄적인 로깅 통합
- 스레드 안전 어설션 매크로 (CORE_ASSERT, CORE_VERIFY)
- 메모리 추적 (할당 크기/횟수)

##### 로깅 시스템 (구현 완료)
**상태**: 다중 출력 지원과 함께 완전히 구현됨

**구성 요소:**
- **Logger**: 모든 로그 작업을 관리하는 스레드 안전 싱글톤
- **LogSink**: 출력 대상을 위한 추상 인터페이스
  - ConsoleSink: 컬러 코딩된 콘솔 출력 (Windows Console API)
  - FileSink: 타임스탬프가 있는 파일 기반 로깅
- **LogLevel**: Trace, Debug, Info, Warning, Error, Fatal
- **LogCategory**: Core, Graphics, Physics, AI, Audio, Input, Memory

**테스트 커버리지**: 05_LoggingTest

```cpp
// Core/include/Core/Logging/Logger.h
class Logger
{
public:
    static Logger& GetInstance();
    
    void Log(LogLevel level, LogCategory category, 
             const std::string& message, const char* file, int line);
    
    void AddSink(std::unique_ptr<LogSink> sink);
    void SetMinLevel(LogLevel level);
    void Flush();
    
private:
    std::vector<std::unique_ptr<LogSink>> mSinks;
    LogLevel mMinLevel = LogLevel::Trace;
    std::mutex mMutex;
};

// Core/include/Core/Logging/LogMacros.h
#define LOG_INFO(format, ...) \
    Logger::GetInstance().Log(LogLevel::Info, LogCategory::Core, \
        FormatLog(format, ##__VA_ARGS__), __FILE__, __LINE__)

#define LOG_GRAPHICS_INFO(format, ...) \
    Logger::GetInstance().Log(LogLevel::Info, LogCategory::Graphics, \
        FormatLog(format, ##__VA_ARGS__), __FILE__, __LINE__)
```

**설계 결정사항:**
- 전역 접근성을 위한 싱글톤 패턴
- 확장 가능한 출력 시스템을 위한 Sink 패턴
- 편의성과 컴파일 타임 최적화를 위한 매크로 기반 API
- Printf 스타일 포매팅
- Release 빌드에서 Trace/Debug 컴파일 타임 제거

##### 타이밍 시스템 (구현 완료)
**상태**: 고정밀 타이머 완전히 구현됨

**핵심 클래스:**
```cpp
// Core/include/Core/Timing/Timer.h
class Timer
{
public:
    void Start();
    void Tick();
    float32 GetDeltaTime() const;
    float32 GetTotalTime() const;
    
private:
    int64 mFrequency;
    int64 mStartTime;
    int64 mCurrentTime;
    int64 mPreviousTime;
    
    static constexpr uint32 SAMPLE_COUNT = 50;
    std::array<float32, SAMPLE_COUNT> mDeltaTimeSamples;
    uint32 mSampleIndex;
};
```

**기능:**
- QueryPerformanceCounter 기반 고정밀 타이밍
- 50 샘플 이동 평균으로 부드러운 델타 타임
- 프레임 레이트 독립적 업데이트 지원


#### 4. Graphics 레이어 (구현 완료)
**책임**: 렌더링 시스템 및 DirectX 12 추상화

**핵심 구성 요소:**

##### DirectX 12 초기화 (구현 완료)
**상태**: 완전히 작동하는 렌더링 파이프라인

**클래스:**
- **DX12Device**: DXGI Factory, Adapter, Device 관리
- **DX12CommandQueue**: Graphics/Compute/Copy 큐 관리
- **DX12SwapChain**: 프레젠테이션 및 백 버퍼 관리
- **DX12CommandContext**: Command List 및 Allocator 풀링
- **DX12DescriptorHeap**: RTV, DSV, CBV/SRV/UAV 관리
- **DX12Fence**: CPU-GPU 동기화

```cpp
// Graphics/include/Graphics/DX12/DX12Device.h
class DX12Device
{
public:
    bool Initialize(bool useWarp = false, bool enableDebugLayer = false);
    
    ID3D12Device* GetDevice() const { return mDevice.Get(); }
    DX12CommandQueue* GetGraphicsQueue() { return &mGraphicsQueue; }
    DX12CommandContext* GetCommandContext(uint32 frameIndex);
    
private:
    Microsoft::WRL::ComPtr<ID3D12Device> mDevice;
    DX12CommandQueue mGraphicsQueue;
    std::vector<std::unique_ptr<DX12CommandContext>> mContexts;
};
```

**프레임 버퍼링 전략:**
- 트리플 버퍼링 (3개의 백 버퍼)
- 프레임별 리소스 (CommandAllocator, ConstantBuffer)
- Fence 기반 CPU-GPU 동기화

**이중 인덱싱 시스템:**
```cpp
// currentFrameIndex: CPU 순차 인덱스 (0 → 1 → 2 → 0...)
// backBufferIndex: GPU 비순차 인덱스 (Present 순서)
uint32 currentFrameIndex = 0;
uint32 backBufferIndex = swapChain->GetCurrentBackBufferIndex();
```

##### 메시 시스템 (구현 완료)
**클래스:**
- **Mesh**: 정점/인덱스 버퍼 관리
- **DX12VertexBuffer**: 정점 데이터 GPU 업로드
- **DX12IndexBuffer**: 인덱스 데이터 GPU 업로드

```cpp
// Graphics/include/Graphics/Mesh.h
class Mesh
{
public:
    bool Initialize(
        ID3D12Device* device,
        DX12CommandQueue* queue,
        DX12CommandContext* context,
        const void* vertexData,
        uint32 vertexCount,
        uint32 vertexStride,
        const uint16* indexData,
        uint32 indexCount
    );
    
    D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const;
    D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const;
    uint32 GetIndexCount() const { return mIndexCount; }
    
private:
    std::unique_ptr<DX12VertexBuffer> mVertexBuffer;
    std::unique_ptr<DX12IndexBuffer> mIndexBuffer;
    uint32 mVertexCount;
    uint32 mIndexCount;
};
```

##### 텍스처 시스템 (구현 완료)
**상태**: PBR 워크플로우 지원 완료

**PBR 텍스처 타입 (7종):**
```cpp
// Graphics/include/Graphics/TextureType.h
enum class TextureType : uint32
{
    Diffuse = 0,     // t0 - 기본 색상
    Normal,          // t1 - 노말 맵
    Specular,        // t2 - 스페큘러 맵
    Roughness,       // t3 - 거칠기 맵
    Metallic,        // t4 - 금속성 맵
    AO,              // t5 - Ambient Occlusion
    Emissive,        // t6 - 발광 맵
    Count
};
```

**Texture 클래스:**
```cpp
// Graphics/include/Graphics/Texture.h
class Texture
{
public:
    bool LoadFromFile(
        ID3D12Device* device,
        DX12CommandQueue* queue,
        DX12CommandContext* context,
        const wchar_t* filename
    );
    
    bool CreateSRV(
        ID3D12Device* device,
        DX12DescriptorHeap* heap,
        uint32 descriptorIndex
    );
    
    ID3D12Resource* GetResource() const { return mTextureResource.Get(); }
    
private:
    Microsoft::WRL::ComPtr<ID3D12Resource> mTextureResource;
    Microsoft::WRL::ComPtr<ID3D12Resource> mUploadBuffer;
};
```

**로더 지원:**
- WIC Loader: PNG, JPG (일반 이미지)
- DDS Loader: DDS (압축 텍스처, 밉맵)

##### Material 시스템 (구현 완료)
**책임**: 셰이더, 텍스처, 렌더 상태 관리

```cpp
// Graphics/include/Graphics/Material.h
class Material
{
public:
    explicit Material(const MaterialDesc& desc);
    
    // PBR 텍스처 설정
    void SetTexture(TextureType type, const std::shared_ptr<Texture>& texture);
    
    // Descriptor Table 할당 (7개 연속 슬롯)
    bool AllocateDescriptors(ID3D12Device* device, DX12DescriptorHeap* heap);
    void FreeDescriptors(DX12DescriptorHeap* heap);
    
    // 렌더링 시 바인딩
    D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorTableHandle(const DX12DescriptorHeap* heap) const;
    
    // PSO 생성 정보
    D3D12_BLEND_DESC GetBlendState() const;
    D3D12_RASTERIZER_DESC GetRasterizerState() const;
    D3D12_DEPTH_STENCIL_DESC GetDepthStencilState() const;
    
private:
    uint32 mDescriptorStartIndex = INVALID_DESCRIPTOR_INDEX;
    std::array<std::shared_ptr<Texture>, static_cast<size_t>(TextureType::Count)> mTextures;
};
```

**Descriptor Table 구조:**
- 연속된 7개 SRV 슬롯 (t0~t6)
- 빈 슬롯은 Null Descriptor 자동 생성
- 한 번의 `SetGraphicsRootDescriptorTable` 호출로 바인딩


##### 렌더링 파이프라인 (구현 완료)
**DX12Renderer 클래스:**
- Root Signature 관리
- Pipeline State Object (PSO) 캐싱
- Shader 컴파일
- ConstantBuffer 관리
- DepthStencilBuffer 관리

**렌더링 플로우:**
```cpp
// Scene에서 렌더링 데이터 수집
FrameData frameData;
scene->CollectRenderData(frameData);

// Renderer가 실제 렌더링 수행
renderer->BeginFrame();
renderer->RenderFrame(frameData);
renderer->EndFrame();
renderer->Present();
```

**RenderItem 구조:**
```cpp
// Graphics/include/Graphics/RenderTypes.h
struct RenderItem
{
    Mesh* mesh;
    Material* material;
    Matrix4x4 worldMatrix;
    Matrix4x4 mvpMatrix;  // HLSL용 전치 완료
};

struct FrameData
{
    std::vector<RenderItem> opaqueItems;
    std::vector<RenderItem> transparentItems;  // 향후 지원
    Matrix4x4 viewMatrix;
    Matrix4x4 projectionMatrix;
    Vector3 cameraPosition;
};
```

#### 5. Framework 레이어 (구현 완료)
**책임**: 고수준 애플리케이션 프레임워크 및 리소스 관리

**모듈 위치**: 최상위 통합 레이어
**의존성**: Framework → Graphics/Platform/Core/Math (단방향)

##### Application 클래스 (구현 완료)
**템플릿 메서드 패턴 적용**

```cpp
// Framework/include/Framework/Application.h
class Application
{
public:
    explicit Application(const ApplicationDesc& desc);
    virtual ~Application();
    
    int Run();
    
protected:
    // 사용자 커스터마이징 지점
    virtual bool OnInitialize() { return true; }
    virtual void OnUpdate(float32 deltaTime) {}
    virtual void OnRender() {}
    virtual void OnShutdown() {}
    
    // 접근자
    DX12Device* GetDevice() { return &mDevice; }
    DX12Renderer* GetRenderer() { return &mRenderer; }
    Window* GetWindow() { return &mWindow; }
    
private:
    void Initialize();
    void Update();
    void Render();
    void Shutdown();
    
    // 엔진 시스템
    Window mWindow;
    Input mInput;
    DX12Device mDevice;
    DX12Renderer mRenderer;
    Timer mTimer;
    
    bool mIsRunning = false;
};
```

**라이프사이클:**
1. `Initialize()` - 엔진 초기화
2. `OnInitialize()` - 사용자 초기화
3. Loop:
   - `Update()` - 입력 처리, 타이머 업데이트
   - `OnUpdate(deltaTime)` - 사용자 업데이트
   - `OnRender()` - 사용자 렌더링
   - `Render()` - 프레임 제출
4. `OnShutdown()` - 사용자 정리
5. `Shutdown()` - 엔진 정리

##### ResourceManager 클래스 (구현 완료)
**중앙 집중식 리소스 관리**

```cpp
// Framework/include/Framework/Resources/ResourceManager.h
class ResourceManager
{
public:
    ResourceManager(DX12Device* device, DX12Renderer* renderer);
    
    // Mesh 관리
    std::shared_ptr<Mesh> CreateMesh(const std::string& name);
    std::shared_ptr<Mesh> GetMesh(const std::string& name) const;
    bool RemoveMesh(const std::string& name);
    
    // Material 관리
    std::shared_ptr<Material> CreateMaterial(
        const std::string& name,
        const std::wstring& vertexShader,
        const std::wstring& pixelShader
    );
    std::shared_ptr<Material> GetMaterial(const std::string& name) const;
    bool RemoveMaterial(const std::string& name);
    
    // Texture 관리 (자동 캐싱)
    std::shared_ptr<Texture> LoadTexture(const std::wstring& path);
    std::shared_ptr<Texture> GetTexture(const std::wstring& path) const;
    bool RemoveTexture(const std::wstring& path);
    
    void Clear();
    
private:
    DX12Device* mDevice;
    DX12Renderer* mRenderer;
    
    std::unordered_map<std::string, std::shared_ptr<Mesh>> mMeshes;
    std::unordered_map<std::string, std::shared_ptr<Material>> mMaterials;
    std::unordered_map<std::wstring, std::shared_ptr<Texture>> mTextures;
};
```

**기능:**
- `std::shared_ptr` 기반 참조 카운팅
- 중복 로딩 방지 (경로 기반 캐싱)
- 생명주기 자동 관리

##### Scene/GameObject 시스템 (구현 완료)
**레거시 지원 (ECS와 병행 사용 가능)**

---

## ECS 아키텍처

### 개요
DevMiniEngine은 데이터 지향 설계(DOD) 원칙에 따른 ECS 아키텍처를 채택합니다.

```
┌─────────────────────────────────────────────────────────┐
│                      Registry                           │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐      │
│  │   Entity    │  │  Component  │  │   System    │      │
│  │   Manager   │  │   Storage   │  │   Manager   │      │
│  └─────────────┘  └─────────────┘  └─────────────┘      │
└─────────────────────────────────────────────────────────┘
```

### 핵심 개념

#### Entity
- **정의**: 고유 식별자 (ID + Version)
- **역할**: Component를 그룹화하는 핸들
- **특징**: 데이터 없음, 로직 없음

```cpp
struct Entity
{
    uint32 id;       // 고유 식별자
    uint32 version;  // 재활용 세대
    
    bool IsValid() const;
    static Entity Invalid();
};
```

#### Component
- **정의**: 순수 데이터 구조 (POD)
- **역할**: Entity의 속성/상태 저장
- **원칙**: 로직 금지, 데이터만

```cpp
// 위치/회전/스케일 데이터
struct TransformComponent
{
    Vector3 position;
    Quaternion rotation;
    Vector3 scale;
    
    // 캐시된 행렬 (Phase 3.5)
    Matrix4x4 localMatrix;
    Matrix4x4 worldMatrix;
    
    // Dirty Flags
    bool localDirty = true;
    bool worldDirty = true;
};

// 계층 구조 데이터 (Phase 3.5)
struct HierarchyComponent
{
    Entity parent = Entity::Invalid();
    std::vector<Entity> children;
};

// 조명 데이터
struct DirectionalLightComponent
{
    Vector3 direction;
    Vector3 color;
    float32 intensity;
};
```

#### System
- **정의**: Component 데이터를 처리하는 로직
- **역할**: 특정 Component 조합을 가진 Entity 처리
- **원칙**: 상태 최소화, 데이터는 Component에

```cpp
class TransformSystem : public ISystem
{
public:
    void Initialize() override;
    void Update(float32 deltaTime) override;
    void Shutdown() override;
    
    // 계층 구조 API
    bool SetParent(Entity child, Entity parent);
    Entity GetParent(Entity entity) const;
    const std::vector<Entity>& GetChildren(Entity entity) const;
    
    // Transform API
    bool SetPosition(Entity entity, const Vector3& position);
    bool SetRotation(Entity entity, const Quaternion& rotation);
    bool GetWorldMatrix(Entity entity, Matrix4x4& outMatrix) const;
    
    // 저수준 API (정적)
    static Matrix4x4 CalculateLocalMatrix(const TransformComponent& transform);
    static void SetRotationEuler(TransformComponent& transform, float x, float y, float z);
};
```

### Registry
ECS의 중앙 관리자로서 Entity 생명주기와 Component 저장을 담당합니다.

```cpp
class Registry
{
public:
    // Entity 관리
    Entity CreateEntity();
    void DestroyEntity(Entity entity);
    bool IsEntityValid(Entity entity) const;
    
    // Component 관리
    template<typename T>
    T* AddComponent(Entity entity, const T& component);
    
    template<typename T>
    T* GetComponent(Entity entity);
    
    template<typename T>
    bool HasComponent(Entity entity) const;
    
    // Query
    template<typename... Components>
    RegistryView<Components...> CreateView();
};
```

### SystemManager
System의 등록, 실행 순서, 생명주기를 관리합니다.

```cpp
class SystemManager
{
public:
    explicit SystemManager(Registry& registry);
    
    template<typename T, typename... Args>
    T* RegisterSystem(Args&&... args);
    
    template<typename T>
    T* GetSystem();
    
    void UpdateSystems(float32 deltaTime);
    void ShutdownSystems();
};
```

### 구현된 Component 목록

| Component | 용도 | 크기 |
|-----------|------|------|
| TransformComponent | 위치/회전/스케일 + 행렬 캐시 | ~160B |
| HierarchyComponent | 부모-자식 관계 | ~32B |
| MeshComponent | 메시 리소스 참조 | 8B |
| MaterialComponent | 머티리얼 리소스 참조 | 8B |
| CameraComponent | 카메라 파라미터 + 행렬 | ~200B |
| DirectionalLightComponent | 방향광 속성 | ~32B |
| PointLightComponent | 점광원 속성 | ~48B |

### 구현된 System 목록

| System | 역할 | 실행 순서 |
|--------|------|-----------|
| TransformSystem | 계층 구조 업데이트, World Matrix 계산 | 1 |
| CameraSystem | 카메라 행렬 업데이트 | 2 |
| LightingSystem | 조명 데이터 수집 | 3 |
| RenderSystem | 렌더링 데이터 수집, FrameData 생성 | 4 |

### Transform 계층 구조 (Phase 3.5)

**설계 결정:**
- HierarchyComponent를 TransformComponent와 분리 (ECS 원칙)
- children은 std::vector 사용 (캐시 친화적 순회)
- Root Entity 목록을 TransformSystem에서 관리

**Dirty Flag 최적화:**
```cpp
// localDirty: position/rotation/scale 변경 시
// worldDirty: local 변경 또는 parent 변경 시

void TransformSystem::Update(float32 deltaTime)
{
    // 1. Root Entity부터 DFS 순회
    for (Entity root : mRootEntities)
    {
        UpdateHierarchy(root, Matrix4x4::Identity());
    }
    
    // 2. Hierarchy 없는 Entity 단독 처리
    UpdateStandaloneEntities();
}
```

**World Matrix 계산:**
```cpp
// World = Local * ParentWorld
transform->worldMatrix = transform->localMatrix * parentWorldMatrix;
```

---

## 프레임 플로우


### 메인 루프 시퀀스
```
┌─────────────────────────────────────────┐
│  1. Platform: 윈도우 이벤트 처리         │
│     - 입력 이벤트                        │
│     - 윈도우 크기 변경 등                │
└────────────────┬────────────────────────┘
                 │
┌────────────────▼────────────────────────┐
│  2. Input: 입력 상태 업데이트            │
│     - 키보드, 마우스                     │
│     - 프레임 시작 시 Update()            │
│     - 프레임 종료 시 Reset()             │
└────────────────┬────────────────────────┘
                 │
┌────────────────▼────────────────────────┐
│  3. Timer: Delta Time 계산               │
│     - Tick()                             │
│     - 50 샘플 평균화                     │
└────────────────┬────────────────────────┘
                 │
┌────────────────▼────────────────────────┐
│  4. Application::OnUpdate(deltaTime)     │
│     - 게임 로직                          │
│     - GameObject 업데이트                │
│     - Scene 업데이트                     │
└────────────────┬────────────────────────┘
                 │
┌────────────────▼────────────────────────┐
│  5. Application::OnRender()              │
│     - Scene::CollectRenderData()         │
│     - Renderer::RenderFrame()            │
└────────────────┬────────────────────────┘
                 │
┌────────────────▼────────────────────────┐
│  6. Renderer::Present()                  │
│     - SwapChain::Present()               │
│     - Fence 시그널                       │
└─────────────────────────────────────────┘
```

### 렌더 프레임 플로우
```
┌──────────────────────────────────────────┐
│  Scene::CollectRenderData()              │
│  - GameObject 순회                       │
│  - Mesh/Material 수집                    │
│  - MVP 행렬 계산                         │
│  - RenderItem 생성                       │
└────────────────┬─────────────────────────┘
                 │
┌────────────────▼─────────────────────────┐
│  Renderer::BeginFrame()                  │
│  - WaitForFenceValue (CPU 동기화)        │
│  - Reset CommandAllocator/List           │
│  - Transition: Present → RenderTarget    │
└────────────────┬─────────────────────────┘
                 │
┌────────────────▼─────────────────────────┐
│  Renderer::RenderFrame(FrameData)        │
│  - ClearRenderTarget / DepthStencil      │
│  - SetViewport / Scissor                 │
│  - RenderItem 순회:                      │
│    * PSO 캐시 또는 생성                  │
│    * SetPipelineState                    │
│    * SetGraphicsRootSignature            │
│    * SetGraphicsRootConstantBufferView   │
│    * SetGraphicsRootDescriptorTable      │
│    * DrawIndexedInstanced                │
└────────────────┬─────────────────────────┘
                 │
┌────────────────▼─────────────────────────┐
│  Renderer::EndFrame()                    │
│  - Transition: RenderTarget → Present    │
│  - Execute CommandList                   │
└────────────────┬─────────────────────────┘
                 │
┌────────────────▼─────────────────────────┐
│  Renderer::Present()                     │
│  - SwapChain::Present()                  │
│  - Signal Fence                          │
│  - MoveToNextFrame()                     │
└──────────────────────────────────────────┘
```

### 메인 루프

```cpp
void Application::Run()
{
    while (!mShouldQuit)
    {
        // 1. 윈도우 메시지 처리
        mWindow->ProcessMessages();
        
        // 2. 입력 업데이트
        Input::BeginFrame();
        
        // 3. 타이머 업데이트
        mTimer.Tick();
        float deltaTime = mTimer.GetDeltaTime();
        
        // 4. 게임 로직 업데이트
        OnUpdate(deltaTime);
        
        // 5. ECS System 업데이트
        mSystemManager->UpdateSystems(deltaTime);
        
        // 6. 렌더링
        BeginFrame();
        OnRender();
        RenderDebugUI();
        EndFrame();
        Present();
        
        // 7. 입력 상태 정리
        Input::EndFrame();
    }
}
```

### 렌더링 파이프라인 (Phase 3.4 이후)

```cpp
// BeginFrame
mRenderer->BeginFrame();

// Scene 렌더링
const FrameData& frameData = mRenderSystem->GetFrameData();
mRenderer->RenderScene(frameData);

// ImGui 렌더링
mImGuiManager->BeginFrame();
OnRenderDebugUI();
mImGuiManager->EndFrame(commandList);

// EndFrame + Present
mRenderer->EndFrame();
mRenderer->Present();
```

---

## 메모리 관리

### 전략 개요

**할당 카테고리:**

1. **영구 메모리**: 전체 애플리케이션 수명 동안 유지
   - 엔진 코어 시스템
   - 로드된 리소스 (Mesh, Texture, Material)

2. **레벨 메모리**: 씬 변경 시 클리어
   - Scene GameObject
   - 레벨 특정 리소스

3. **프레임 메모리**: 매 프레임 클리어
   - 임시 데이터 (RenderItem 등)
   - UI 레이아웃 계산

### 할당자 사용 가이드

| 할당자 | 사용 사례 | 할당 해제 |
|--------|----------|----------|
| LinearAllocator | 프레임 임시 데이터, 레벨 로딩 | Reset (전체) |
| PoolAllocator | 엔티티, 컴포넌트, 파티클 | Deallocate (개별) |
| StackAllocator | 중첩 스코프, AI 탐색 트리 | FreeToMarker (부분) |

### GPU 메모리 관리

**버퍼 타입:**
- **DEFAULT Heap**: GPU 전용 메모리 (최적 성능)
- **UPLOAD Heap**: CPU → GPU 전송용
- **READBACK Heap**: GPU → CPU 전송용 (디버깅)

**프레임 리소스 관리:**
```cpp
struct FrameResource
{
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    std::unique_ptr<DX12ConstantBuffer> constantBuffer;
    uint64 fenceValue;
};

std::array<FrameResource, FRAME_BUFFER_COUNT> mFrameResources;
```

---

## 렌더링 아키텍처

### DirectX 12 동기화

**Fence 기반 동기화:**
```cpp
// 프레임 시작
uint64 currentFenceValue = mFrameResources[currentFrameIndex].fenceValue;
commandQueue->WaitForFenceValue(currentFenceValue);

// 명령 기록 및 실행
// ...

// 프레임 종료
uint64 newFenceValue = commandQueue->Signal();
mFrameResources[currentFrameIndex].fenceValue = newFenceValue;
```

**핵심 원칙:**
- WaitForIdle 사용 금지 (CPU 유휴 시간 증가)
- 프레임별 Fence Value 추적
- CPU-GPU 병렬 작업 최대화

### Pipeline State Object (PSO) 캐싱

**DX12PipelineStateCache:**
```cpp
class DX12PipelineStateCache
{
public:
    ID3D12PipelineState* GetOrCreate(
        const Material& material,
        DX12RootSignature* rootSignature,
        const D3D12_INPUT_LAYOUT_DESC& inputLayout
    );
    
private:
    std::unordered_map<size_t, ComPtr<ID3D12PipelineState>> mCache;
};
```

**해시 키 구성:**
- 셰이더 경로
- 블렌드 상태
- 래스터라이저 상태
- 깊이-스텐실 상태
- 입력 레이아웃

### Descriptor 관리

**Descriptor Heap 구조:**
```cpp
class DX12DescriptorHeap
{
public:
    // 연속 블록 할당 (Descriptor Table용)
    uint32 AllocateBlock(uint32 count);
    void FreeBlock(uint32 startIndex, uint32 count);
    
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32 index) const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32 index) const;
    
private:
    ComPtr<ID3D12DescriptorHeap> mHeap;
    uint32 mDescriptorSize;
    uint32 mCurrentOffset;
    std::queue<std::pair<uint32, uint32>> mFreeBlocks;  // 향후 재사용
};
```

**Material Descriptor 할당:**
- Material당 7개 연속 SRV (PBR 텍스처)
- 한 번의 `SetGraphicsRootDescriptorTable` 호출로 바인딩


### DX12 래퍼 클래스

```cpp
class DX12Device
{
    ID3D12Device* mDevice;
    IDXGIFactory4* mFactory;
    // ...
};

class DX12CommandQueue
{
    ID3D12CommandQueue* mCommandQueue;
    ID3D12Fence* mFence;
    // ...
};

class DX12Renderer
{
    void BeginFrame();
    void RenderScene(const FrameData& frameData);
    void EndFrame();
    void Present();
};
```

### 리소스 관리

**ResourceId 시스템:**
```cpp
struct ResourceId
{
    uint64 id;  // 64비트 해시
    bool IsValid() const { return id != 0; }
};

// Component에서 리소스 참조
struct MeshComponent { ResourceId meshId; };      // 8 bytes
struct MaterialComponent { ResourceId materialId; }; // 8 bytes
```

**ResourceManager:**
```cpp
class ResourceManager
{
    ResourceId CreateMesh(const std::string& name);
    ResourceId CreateMaterial(const std::string& name, ...);
    ResourceId LoadTexture(const std::string& path);
    
    Mesh* GetMesh(ResourceId id);
    Material* GetMaterial(ResourceId id);
    Texture* GetTexture(ResourceId id);
};
```

### Phong Shading (Phase 3.3)

**Constant Buffer 구조:**
```hlsl
// b0: Transform
cbuffer TransformCB : register(b0)
{
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 worldInvTranspose;
};

// b1: Material
cbuffer MaterialCB : register(b1)
{
    float3 albedo;
    float metallic;
    float roughness;
    float ao;
    uint materialFlags;
};

// b2: Lighting
cbuffer LightingCB : register(b2)
{
    DirectionalLight dirLight;
    PointLight pointLights[MAX_POINT_LIGHTS];
    uint numPointLights;
    float3 cameraPosition;
};
```

---

## 로드맵

### Phase 1: 기초 시스템 (완료)
- [x] 프로젝트 구조 설정
- [x] Core 시스템 (메모리, 로깅, 타이머)
- [x] Math 라이브러리 (SIMD)

### Phase 2: DirectX 12 기초 (완료)
- [x] 윈도우 생성 및 입력 처리
- [x] DX12 초기화
- [x] 기본 렌더링 파이프라인
- [x] 메시 및 텍스처 시스템
- [x] 카메라 시스템
- [x] Framework 아키텍처

### Phase 3: ECS 아키텍처 (완료)
- [x] **3.1 ECS Foundation**
  - Entity (ID + Version)
  - Registry, Component Storage
  - TransformComponent/System
  - ResourceId 시스템 (64비트 해시)
  - 09_ECSRotatingCube 샘플
  
- [x] **3.2 System Framework**
  - ISystem 인터페이스
  - SystemManager
  - RegistryView Query 패턴
  - CameraComponent/System
  - RenderSystem (자동 수집)
  
- [x] **3.3 Lighting**
  - DirectionalLightComponent
  - PointLightComponent
  - LightingSystem
  - Phong Shading
  - Normal Mapping
  - 10_PhongLighting 샘플
  
- [x] **3.4 Debug Tools**
  - ImGui 통합
  - ImGuiManager
  - PerformancePanel (FPS, 그래프)
  - ECSInspector (Entity/Component 편집)
  
- [x] **3.5 Advanced**
  - HierarchyComponent
  - Transform 계층 구조 (parent-child)
  - Dirty Flag 최적화
  - DFS 순회 World Matrix 업데이트
  - 10_PhongLighting에 계층 테스트 통합

---
### Phase 4: DX12 인프라 확장 & Asset Pipeline
**목표:** 생산성 향상을 위한 에셋 파이프라인

- [ ] **Descriptor 관리 고도화**
  - [ ] SRV/UAV Descriptor Heap 통합
  - [ ] Descriptor 풀 및 재활용
  - [ ] Frame-based 할당
- [ ] **리소스 업로드 최적화**
  - [ ] Upload 버퍼 링
  - [ ] Async Copy Queue (선택적)
- [ ] **Asset Pipeline**
  - [ ] glTF 2.0 로더 (메쉬, PBR 재질, 계층구조)
  - [ ] DDS/BCn 텍스처 파이프라인
  - [ ] Mipmap 생성 자동화
- [ ] **Shader 시스템 확장**
  - [ ] DXC 컴파일러 (Shader Model 6.6+)
  - [ ] Shader Reflection (자동 Root Signature)
  - [ ] Hot Reload (선택적)
- [ ] **ECS 렌더링 통합**
  - [ ] MeshRendererComponent
  - [ ] RenderSystem (ECS 쿼리 기반)

**완료 시:** Blender 모델 로딩, ECS 기반 렌더링

---

### Phase 4.5: Frame Graph & Culling
**목표:** 효율적인 렌더링 파이프라인

- [ ] **Frame Graph**
  - [ ] Pass/Resource 의존성 그래프
  - [ ] 자동 Barrier 삽입
  - [ ] Resource Lifetime 관리
- [ ] **Culling System**
  - [ ] Frustum Culling (CPU)
  - [ ] AABB Bounding Box
- [ ] **Asset Pipeline 확장 (선택적)**
  - [ ] Hot Reload (텍스처, 셰이더)

**완료 시:** 1000개 오브젝트 효율적 렌더링

---

### Phase 5: PBR & Post-Processing
**목표:** 사실적인 렌더링

- [ ] **PBR Material**
  - [ ] Metallic-Roughness 워크플로우
  - [ ] Cook-Torrance BRDF
  - [ ] IBL (Skybox, PMREM, Irradiance Map)
  - [ ] Uber Shader & Permutations
- [ ] **Post-Processing**
  - [ ] 톤매핑 (Reinhard, ACES)
  - [ ] Auto Exposure
  - [ ] Bloom
  - [ ] TAA (Temporal Anti-Aliasing)

**완료 시:** PBR 재질의 다양한 오브젝트, IBL 조명

---

### Phase 5.5: Physics Integration (포트폴리오 마일스톤)
**목표:** 플레이 가능한 프로토타입 제작

> **중요:** 이 시점부터 포트폴리오 영상 제작 가능!

- [ ] **Physics Engine 통합**
  - [ ] Jolt Physics 또는 Bullet 선택
  - [ ] Physics World 초기화
- [ ] **Physics Components (ECS)**
  - [ ] RigidBodyComponent (Mass, Velocity)
  - [ ] ColliderComponent (Box, Sphere, Capsule)
  - [ ] Physics Material (Friction, Restitution)
- [ ] **PhysicsSystem**
  - [ ] Transform ↔ Physics Engine 동기화
  - [ ] Fixed Timestep
  - [ ] 충돌 이벤트 (OnCollisionEnter/Exit)
- [ ] **샌드박스 데모**
  - [ ] 중력으로 떨어지는 큐브들
  - [ ] 플레이어가 오브젝트 밀기 (마우스 Ray)
  - [ ] ImGui로 RigidBody 속성 편집
- [ ] **물리 디버그 렌더링**
  - [ ] Collider 와이어프레임
  - [ ] Contact Point 표시

**완료 시:** PBR 재질 + Physics 상호작용 데모 (포트폴리오 소재 확보)

---

### Phase 6: Shadows & Ambient
**목표:** 시각적 완성도 향상

- [ ] **Shadow Mapping**
  - [ ] 기본 Shadow Map (Depth, PCF)
  - [ ] Cascaded Shadow Maps (CSM)
  - [ ] Cascade 경계 블렌딩
- [ ] **Ambient Occlusion**
  - [ ] SSAO (Screen Space Ambient Occlusion)
  - [ ] Bilateral Blur
- [ ] **추가 효과 (선택적)**
  - [ ] SSR (Screen Space Reflections)
  - [ ] DOF (Depth of Field)

**완료 시:** Physics 샌드박스 + 동적 그림자 + SSAO

---

### Phase 7: Compute Shaders & GPU 가속
**목표:** GPU 연산 활용

- [ ] **Compute Pipeline**
  - [ ] Dispatch 프레임워크
  - [ ] UAV (Unordered Access View) 관리
- [ ] **GPU Particle System**
  - [ ] Compute Shader 기반 시뮬레이션
  - [ ] Indirect Drawing
  - [ ] Physics 오브젝트와 상호작용
- [ ] **GPU Culling (선택적)**
  - [ ] Frustum Culling (Compute)
  - [ ] Hi-Z Occlusion Culling

**완료 시:** 10만 개 파티클 + Physics 상호작용

---

### Phase 8: Bindless & GPU Driven
**목표:** 최신 렌더링 기법

- [ ] **Bindless Resources**
  - [ ] Descriptor Indexing (SM 6.6+)
  - [ ] Unbounded Descriptor Arrays
  - [ ] Material ID → Descriptor Index 매핑
- [ ] **GPU Driven Rendering**
  - [ ] ExecuteIndirect / DrawIndirect
  - [ ] GPU가 Draw Call 생성
  - [ ] Mesh Shaders (선택적, DX12 Ultimate)
- [ ] **Resource Management**
  - [ ] GPU 메모리 할당자
  - [ ] Async Upload 튜닝

**완료 시:** 1만 개 고유 메쉬 + Physics 렌더링

---

### Phase 9: Job System (Phase 3.5에서 구현 X)
**목표:** 기본 멀티스레딩 인프라, CPU 병렬화

- [ ] 워커 스레드 풀
- [ ] Job 디스패처
- [ ] TransformSystem 병렬화
- [ ] 성능 벤치마크 (Single vs Multi-thread)
- [ ] Job System 구현 (Phase 3.5 참고)
- [ ] ECS System 병렬화
- [ ] PhysicsSystem 병렬화
- [ ] 렌더 스레드 분리

---

### Phase 10: AI & Gameplay Systems
**목표:** 게임 로직 프레임워크

- [ ] **Navigation**
  - [ ] NavMesh 생성 (Recast)
  - [ ] A* Pathfinding
  - [ ] Path Following
- [ ] **Behavior Tree**
  - [ ] Executor (Selector, Sequence, Action)
  - [ ] Blackboard (공유 데이터)
- [ ] **Gameplay Framework**
  - [ ] Event System (타입 안전)
  - [ ] Game State 관리

**완료 시:** AI 에이전트가 플레이어 추적, Physics 오브젝트 회피

---

### Phase 11: Audio & Tools
**목표:** 완성도 향상

- [ ] **Audio System**
  - [ ] XAudio2 통합
  - [ ] 3D Spatialization
  - [ ] 충돌 시 사운드 효과
- [ ] **Debug Tools 고도화**
  - [ ] ECS Inspector (컴포넌트 추가/삭제)
  - [ ] Profiler (CPU/GPU Timeline)
  - [ ] Console (명령어 시스템)

**완료 시:** 완전한 게임 엔진 경험

---

### Phase 12: 데모 게임 & 폴리싱
**목표:** 포트폴리오 완성

- [ ] **Demo Game (Simple TPS)**
  - [ ] Physics Character Controller
  - [ ] 발사 시스템 (Raycast + Force)
  - [ ] Enemy AI (NavMesh + Physics)
  - [ ] UI (체력, 탄약)
- [ ] **Optimization**
  - [ ] CPU/GPU Profiling
  - [ ] Physics 성능 최적화
  - [ ] LOD (Level of Detail)
- [ ] **포트폴리오 문서화**
  - [ ] 기술 블로그 (ECS + Physics 통합)
  - [ ] 영상 녹화 (플레이 + 기술 설명)
  - [ ] GitHub README 업데이트

**최종 결과:** 플레이 가능한 TPS 데모, 포트폴리오 제출 가능

### 백로그 (Backlog / On-Demand)

향후 필요에 따라 선택적으로 추가할 기능들:

- [ ] **Deferred Rendering** - Forward+와 비교/전환
- [ ] **Tessellation & Geometry Shaders** - 적응형 지형
- [ ] **독립형 Editor** - ImGui 툴을 넘어선 독립 에디터
- [ ] **Scripting** - Lua/Python 바인딩
- [ ] **Procedural Terrain** - Heightmap/Voxel 기반
- [ ] **Raytracing (DXR)** - DX12 Ultimate 기반 (RTX GPU 필요)
- [ ] **Advanced Animation** - 블렌딩 트리, IK
- [ ] **Volumetric Effects** - Fog, Clouds
- [ ] **Network Replication** - ECS 컴포넌트 복제, 멀티플레이어

---

## 구현 진행 상황

### 완료된 시스템
| 시스템 | 상태 | 테스트 커버리지 | 주요 기능 |
|--------|------|----------------|----------|
| **Core** |
| Memory | 완료 | 3개 테스트 | Linear/Pool/Stack 할당자 |
| Logging | 완료 | 1개 테스트 | 멀티 Sink, 카테고리별 필터링 |
| Timing | 완료 | - | 고정밀 타이머, 50 샘플 평균화 |
| **Math** |
| Math Library | 완료 | 1개 테스트 | SIMD 최적화, 연산자 오버로딩 |
| **Platform** |
| Window | 완료 | 2개 테스트 | Win32 윈도우, 메시지 처리 |
| Input | 완료 | 1개 테스트 | 키보드/마우스 |
| **Graphics** |
| DX12 Init | 완료 | 1개 테스트 | Device, Queue, SwapChain |
| Mesh System | 완료 | - | Vertex/Index Buffer, Tangent |
| Texture System | 완료 | - | WIC/DDS 로더, PBR 7종 텍스처 |
| Material System | 완료 | - | Descriptor Table, PSO 캐싱 |
| Renderer | 완료 | - | Scene/Renderer 분리 |
| **ECS** |
| Entity/Registry | 완료 | - | ID+Version, 재활용 |
| Component Storage | 완료 | - | 타입별 unordered_map |
| SystemManager | 완료 | - | 등록/실행/종료 관리 |
| TransformSystem | 완료 | - | 계층 구조, Dirty Flag |
| CameraSystem | 완료 | - | View/Projection 행렬 |
| LightingSystem | 완료 | - | 조명 데이터 수집 |
| RenderSystem | 완료 | - | FrameData 자동 수집 |
| **Framework** |
| Application | 완료 | - | 템플릿 메서드 패턴 |
| ResourceManager | 완료 | - | 중앙 집중식 리소스 관리 |
| ImGuiManager | 완료 | - | DX12 ImGui 통합 |
| ECSInspector | 완료 | - | Entity/Component 편집 |
| PerformancePanel | 완료 | - | FPS, 프레임 타임 그래프 |
| **합계** | **20+** | **8개 테스트** | **~15,000 라인** |

### 샘플 프로그램
1. `01_MemoryTest` - LinearAllocator
2. `02_PoolAllocatorTest` - PoolAllocator
3. `03_StackAllocatorTest` - StackAllocator
4. `04_MathTest` - Math 라이브러리
5. `05_LoggingTest` - 로깅 시스템
6. `06_WindowTest` - Window 생성
7. `07_InputTest` - 입력 처리
8. `08_TexturedCube` - 텍스처 큐브 렌더링
9. `09_ECSRotatingCube` - ECS 기반 회전 큐브
10. `10_PhongLighting` - Phong Shading + 계층 구조 데모

---

## 참고 자료

### 게임 엔진 아키텍처
- [Game Engine Architecture by Jason Gregory](https://www.gameenginebook.com/)
- [Game Programming Patterns by Robert Nystrom](https://gameprogrammingpatterns.com/)

### ECS 설계
- [Data-Oriented Design Book](https://www.dataorienteddesign.com/dodbook/)
- [EnTT Documentation](https://github.com/skypjack/entt)
- [Unity DOTS](https://unity.com/dots)

### DirectX 12
- [DirectX 12 Programming Guide](https://docs.microsoft.com/en-us/windows/win32/direct3d12/directx-12-programming-guide)
- [3D Game Programming with DirectX 12 by Frank Luna](http://www.d3dcoder.net/)
- [DirectX Graphics Samples](https://github.com/microsoft/DirectX-Graphics-Samples)

### PBR
- [Physically Based Rendering by Matt Pharr, Wenzel Jakob, Greg Humphreys](https://www.pbr-book.org/)
- [Real Shading in Unreal Engine 4](https://blog.selfshadow.com/publications/s2013-shading-course/)
- [LearnOpenGL - PBR Theory](https://learnopengl.com/PBR/Theory)

---

## 문서 히스토리

- **2025-12-18**: Phase 3 완료, ECS 아키텍처 섹션 추가, 계층 구조 문서화
- **2025-11-07**: Framework 모듈 추가, PBR 텍스처 시스템 완성, Phase 1-2 완료 상태 반영
- **2025-11-05**: Scene/Renderer 분리 완성, Timer 시스템 추가
- **2025-10-15**: DirectX 12 초기화 완료
- **2025-10-13**: Platform 레이어 구현, 입력 시스템 세부사항 업데이트
- **2025-10-11**: 구현된 시스템으로 업데이트 (Memory, Math, Logging)
- **2025-10-05**: 초기 아키텍처 설계 문서

---

**참고**: 이 아키텍처는 프로젝트가 발전하고 새로운 요구사항이 등장함에 따라 변경될 수 있습니다.

**최종 업데이트**: 2025-12-18