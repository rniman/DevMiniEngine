# DevMiniEngine 아키텍처

> **참고**: 이 문서는 계획된 아키텍처와 구현된 아키텍처를 모두 설명합니다. 구현 상태는 명확하게 표시되어 있습니다.

## 목차

- [개요](#개요)
- [설계 철학](#설계-철학)
- [핵심 아키텍처](#핵심-아키텍처)
- [모듈 상세](#모듈-상세)
- [데이터 흐름](#데이터-흐름)
- [메모리 관리](#메모리-관리)
- [스레딩 모델](#스레딩-모델)
- [향후 고려사항](#향후-고려사항)

---

## 개요

DevMiniEngine은 DirectX 12와 현대적인 C++ 관행을 기반으로 하는 학습 중심 게임 엔진으로 설계되었습니다. 아키텍처는 다음을 강조합니다:

- **데이터 지향 설계**: 최적의 캐시 성능을 위한 ECS 아키텍처
- **모듈화**: 엔진 서브시스템 간 명확한 관심사 분리
- **추상화**: 잠재적 그래픽스 API 유연성을 위한 RHI 레이어
- **현대적 C++**: 깔끔하고 효율적인 코드를 위한 C++20 기능 활용

### 주요 아키텍처 목표

1. **교육적 가치**: 학습을 위한 명확하고 읽기 쉬운 코드 구조
2. **성능**: 데이터 지향 설계와 효율적인 메모리 레이아웃
3. **유지보수성**: 최소한의 의존성을 가진 모듈식 설계
4. **확장성**: 새로운 기능과 시스템 추가 용이

---

## 설계 철학

### 1. Entity Component System (ECS)

**왜 ECS인가?**
- 캐시 친화적인 데이터 레이아웃
- 데이터와 로직의 명확한 분리
- 시스템 병렬화 용이
- 유연한 엔티티 구성

**핵심 개념:**
```cpp
// Entity: 단순한 ID
using Entity = uint32_t;

// Component: 순수 데이터
struct TransformComponent
{
    Vector3 position;
    Quaternion rotation;
    Vector3 scale;
};

// System: 순수 로직
class RenderSystem : public System
{
    void Update(World& world, float deltaTime) override;
};
```

### 2. 데이터 지향 설계

**원칙:**
- 연속된 배열에 저장되는 컴포넌트 (SoA - Structure of Arrays)
- 시스템이 컴포넌트 배열을 효율적으로 순회
- 포인터 추적 최소화
- 캐시 친화적인 메모리 접근 패턴

### 3. 의존성 관리

**모듈 계층:**
```
┌─────────────────────────────────────┐
│        애플리케이션 레이어            │
├─────────────────────────────────────┤
│      씬 / 게임플레이 시스템          │
├─────────────────────────────────────┤
│  그래픽스│ 물리 │ AI │ 지형         │
├─────────────────────────────────────┤
│         ECS 프레임워크               │
├─────────────────────────────────────┤
│   Core (메모리, 스레딩, 로깅)        │
├─────────────────────────────────────┤
│      Math (독립적)                  │
├─────────────────────────────────────┤
│       Platform (Win32)              │
└─────────────────────────────────────┘
```

**의존성 규칙:**
- 하위 레이어는 상위 레이어에 대해 알지 못함
- Math 모듈은 완전히 독립적
- Core는 Platform에만 의존
- 모든 게임 시스템은 ECS에 의존

---

## 핵심 아키텍처

### 레이어 분석

#### 1. Platform 레이어 (구현 완료)
**책임**: OS별 기능 추상화

**상태**: Windows용으로 완전히 구현됨

```cpp
// Platform/include/Platform/Window.h
class Window
{
public:
    virtual bool Create(const WindowDesc& desc) = 0;
    virtual void ProcessEvents() = 0;
    virtual WindowHandle GetNativeHandle() const = 0;
    virtual Input& GetInput() = 0;
};

// Platform/src/Windows/Win32Window.cpp
class Win32Window : public Window
{
    // Win32 특정 구현
};
```

**구성 요소:**
- 윈도우 관리 (Win32 구현)
- 이벤트 처리 시스템
- 입력 시스템 (키보드, 마우스)
  - 프레임 정확 입력을 위한 더블 버퍼링
  - Pressed/held/released 상태 추적
- 고해상도 타이머 (계획됨)

**테스트 커버리지**: 06_WindowTest, 07_InputTest

**설계 결정사항:**
- Window가 Input 인스턴스를 소유 (싱글톤 아님)
- 각 윈도우가 독립적인 입력 상태 보유
- 플랫폼별 생성을 위한 팩토리 패턴
- Windows.h 오염 최소화를 위한 전방 선언

**성능:**
- 윈도우 생성: ~5 ms
- ProcessEvents: 프레임당 ~0.01-0.05 ms
- 입력 추적: 무시할 수 있는 오버헤드 (~0.001 ms)

#### 2. Math 레이어 (구현 완료)
**책임**: 수학적 기본 요소 및 연산

**상태**: SIMD 최적화와 함께 완전히 구현됨

**기능:**
- SIMD 최적화 벡터/행렬 연산 (DirectXMath 래퍼)
- Vector2, Vector3, Vector4
- Matrix3x3, Matrix4x4 (행 우선)
- 회전을 위한 Quaternion
- Transform 유틸리티 (Translation, Rotation, Scaling)
- 카메라 행렬 (LookAt, Perspective, Orthographic)
- 일반적인 수학 유틸리티 (Lerp, Clamp, DegToRad 등)

**테스트 커버리지**: 04_MathTest

```cpp
// Math/include/Math/MathTypes.h
using Vector3 = DirectX::XMFLOAT3;
using Matrix4x4 = DirectX::XMFLOAT4X4;
using Quaternion = DirectX::XMFLOAT4;

// 계산용 SIMD 타입
using VectorSIMD = DirectX::XMVECTOR;
using MatrixSIMD = DirectX::XMMATRIX;

// Math/include/Math/MathUtils.h
Vector3 Add(const Vector3& a, const Vector3& b);
float Dot(const Vector3& a, const Vector3& b);
Matrix4x4 MatrixMultiply(const Matrix4x4& a, const Matrix4x4& b);
Quaternion QuaternionFromEuler(float pitch, float yaw, float roll);
```

**설계 결정사항:**
- DirectXMath를 기반으로 사용 (프로덕션 품질 SIMD)
- 저장 타입(XMFLOAT*)과 SIMD 타입(XMVECTOR) 분리
- 편의를 위한 래퍼 함수
- 모든 함수 인라인 (헤더 온리, 오버헤드 없음)

#### 3. Core 레이어 (부분 구현)
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

// Core/include/Core/Memory/PoolAllocator.h
class PoolAllocator : public Allocator
{
public:
    PoolAllocator(size_t chunkSize, size_t chunkCount);
    void* Allocate(size_t size, size_t alignment = DEFAULT_ALIGNMENT) override;
    void Deallocate(void* ptr) override;  // O(1) 해제
    
private:
    void* mMemory;
    size_t mChunkSize;
    void* mFreeList;  // 프리 청크의 연결 리스트
};

// Core/include/Core/Memory/StackAllocator.h
class StackAllocator : public Allocator
{
public:
    using Marker = size_t;
    
    explicit StackAllocator(size_t size);
    void* Allocate(size_t size, size_t alignment = DEFAULT_ALIGNMENT) override;
    Marker GetMarker() const;
    void FreeToMarker(Marker marker);  // LIFO 해제
    
private:
    void* mMemory;
    size_t mOffset;
};
```

**기능:**
- 커스텀 메모리 정렬 지원 (SIMD를 위한 16바이트 기본값)
- 포괄적인 로깅 통합
- 스레드 안전 어설션 매크로 (CORE_ASSERT, CORE_VERIFY)
- 메모리 추적 (할당 크기/횟수)

**향후 개선사항:**
- 디버그 경계 검사
- 메모리 누수 감지
- 할당 통계/프로파일링

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
    std::mutex mMutex;  // 스레드 안전
};

// Core/include/Core/Logging/LogMacros.h
#define LOG_INFO(format, ...) \
    Logger::GetInstance().Log(LogLevel::Info, LogCategory::Core, \
        FormatLog(format, ##__VA_ARGS__), __FILE__, __LINE__)

#define LOG_ERROR(format, ...) \
    Logger::GetInstance().Log(LogLevel::Error, LogCategory::Core, \
        FormatLog(format, ##__VA_ARGS__), __FILE__, __LINE__)

// 카테고리별 매크로
#define LOG_GRAPHICS_INFO(format, ...) \
    Logger::GetInstance().Log(LogLevel::Info, LogCategory::Graphics, \
        FormatLog(format, ##__VA_ARGS__), __FILE__, __LINE__)
```

**설계 결정사항:**
- 전역 접근성을 위한 싱글톤 패턴
- 확장 가능한 출력 시스템을 위한 Sink 패턴
- 편의성과 컴파일 타임 최적화를 위한 매크로 기반 API
- Printf 스타일 포매팅 (간단하고 외부 의존성 없음)
- Release 빌드에서 Trace/Debug 컴파일 타임 제거

**주요 기능:**
- Mutex 보호를 통한 스레드 안전 로깅
- 카테고리 기반 필터링
- 컬러 코딩된 콘솔 출력 (Windows Terminal 호환)
- 전체 타임스탬프와 소스 위치가 있는 파일 로깅
- Release 빌드에서 Trace/Debug 로그의 오버헤드 제로 (~0 ns)
- Debug 오버헤드: 로그당 ~50 마이크로초

**성능:**
```cpp
// Debug 빌드
LOG_INFO("Message");  // ~50 µs 오버헤드

// Release 빌드
LOG_TRACE("Debug info");  // 컴파일 제거됨, 0 오버헤드
LOG_DEBUG("Debug info");  // 컴파일 제거됨, 0 오버헤드
LOG_INFO("Message");      // ~50 µs 오버헤드
```

**사용 예시:**
```cpp
// 기본 로깅
LOG_INFO("엔진이 성공적으로 초기화되었습니다");
LOG_WARN("메모리 예산에 맞추기 위해 텍스처 품질이 감소되었습니다");
LOG_ERROR("에셋 로드 실패: %s", filename);

// 카테고리별 로깅
LOG_GRAPHICS_INFO("%d개의 삼각형 렌더링 중", count);
LOG_PHYSICS_WARN("충돌 감지됨 위치 (%.2f, %.2f, %.2f)", x, y, z);
LOG_MEMORY_DEBUG("LinearAllocator가 %zu 바이트를 할당했습니다", size);

// Memory allocators와의 통합
LinearAllocator frameAlloc(10 * MB);
void* data = frameAlloc.Allocate(1024);
// 자동으로 로그: "LinearAllocator가 0x...에 1024 바이트를 할당했습니다"
```

**향후 개선사항:**
- 고빈도 시나리오를 위한 비동기 로깅
- 로그 파일 회전 (크기/날짜 기반)
- 로그 분석 도구를 위한 JSON 출력 Sink

##### 스레딩 (계획됨)
**상태**: 시작 안 됨

```cpp
// Core/include/Core/Threading/JobSystem.h
class JobSystem
{
public:
    void Schedule(Job* job);
    void Wait(JobHandle handle);
};
```

**컨테이너 (계획됨)**
- 엔진 사용에 최적화된 커스텀 컨테이너

#### 4. ECS 레이어 (계획됨)
**책임**: Entity-Component-System 프레임워크

**핵심 클래스:**

```cpp
// ECS/include/ECS/World.h
class World
{
public:
    Entity CreateEntity();
    void DestroyEntity(Entity entity);
    
    template<typename T>
    T* AddComponent(Entity entity);
    
    template<typename T>
    T* GetComponent(Entity entity);
    
    void RegisterSystem(std::unique_ptr<System> system);
    void Update(float deltaTime);
    
private:
    EntityManager mEntityManager;
    ComponentManager mComponentManager;
    SystemManager mSystemManager;
};
```

**컴포넌트 저장:**
```cpp
// ECS/include/ECS/ComponentArray.h
template<typename T>
class ComponentArray
{
private:
    std::array<T, MAX_ENTITIES> mComponents;
    std::unordered_map<Entity, size_t> mEntityToIndex;
    std::unordered_map<size_t, Entity> mIndexToEntity;
    size_t mSize = 0;
};
```

#### 5. Graphics 레이어 (계획됨)

**RHI (Render Hardware Interface):**
```cpp
// Graphics/include/Graphics/RHI/Device.h
class Device
{
public:
    virtual CommandList* CreateCommandList() = 0;
    virtual Buffer* CreateBuffer(const BufferDesc& desc) = 0;
    virtual Texture* CreateTexture(const TextureDesc& desc) = 0;
};

// Graphics/include/Graphics/DX12/DX12Device.h
class DX12Device : public Device
{
    // DirectX 12 구현
};
```

**렌더링 아키텍처:**
```
┌──────────────────────────────────┐
│      고수준 렌더러                │ ← Material, Mesh, Camera
├──────────────────────────────────┤
│   Render Graph (계획됨)          │ ← Frame graph, pass 순서
├──────────────────────────────────┤
│         RHI 레이어               │ ← API 추상화
├──────────────────────────────────┤
│      DirectX 12 API              │ ← 저수준 구현
└──────────────────────────────────┘
```

#### 6. Physics 레이어 (계획됨)

**아키텍처:**
```cpp
// Physics/include/Physics/PhysicsWorld.h
class PhysicsWorld
{
public:
    void Step(float deltaTime);
    void AddRigidBody(RigidBody* body);
    void RemoveRigidBody(RigidBody* body);
    
private:
    void BroadPhase();
    void NarrowPhase();
    void SolveConstraints();
};
```

**ECS 통합:**
```cpp
// Physics/include/Physics/Systems/PhysicsSystem.h
class PhysicsSystem : public System
{
public:
    void Update(World& world, float deltaTime) override
    {
        // ECS 컴포넌트와 물리 월드 동기화
        SyncTransforms(world);
        
        // 물리 시뮬레이션 스텝
        mPhysicsWorld.Step(deltaTime);
        
        // 결과를 ECS에 다시 작성
        UpdateTransforms(world);
    }
    
private:
    PhysicsWorld mPhysicsWorld;
};
```

#### 7. Scene 레이어 (계획됨)

**Scene Graph vs ECS:**
- ECS가 엔티티 데이터와 로직 처리
- Scene은 고수준 조직화 제공
- SceneManager가 씬 전환 처리

```cpp
// Scene/include/Scene/Scene.h
class Scene
{
public:
    void Load();
    void Unload();
    void Update(float deltaTime);
    
    Entity CreateGameObject(const std::string& name);
    
private:
    World mWorld;
    std::string mName;
    bool mIsLoaded = false;
};
```

---

## 데이터 흐름

### 프레임 업데이트 사이클

```
┌─────────────────────────────────────────┐
│  1. Platform: 윈도우 이벤트 처리         │
│     - 입력 이벤트                       │
│     - 윈도우 크기 변경 등               │
└────────────────┬────────────────────────┘
                 │
┌────────────────▼────────────────────────┐
│  2. Input: 입력 상태 업데이트            │
│     - 키보드, 마우스                    │
│     - 프레임 시작 시 Update()           │
│     - 프레임 종료 시 Reset()            │
└────────────────┬────────────────────────┘
                 │
┌────────────────▼────────────────────────┐
│  3. 게임 로직 시스템 (ECS)              │
│     - AISystem                          │
│     - PhysicsSystem                     │
│     - GameplaySystem                    │
└────────────────┬────────────────────────┘
                 │
┌────────────────▼────────────────────────┐
│  4. 렌더링 (ECS)                        │
│     - CameraSystem                      │
│     - RenderSystem                      │
│       → 컬링                            │
│       → 렌더 큐 정렬                    │
│       → 드로우 콜 제출                  │
└────────────────┬────────────────────────┘
                 │
┌────────────────▼────────────────────────┐
│  5. 프레임 제출                         │
└─────────────────────────────────────────┘
```

### 렌더 프레임 플로우 (계획됨)

```
┌──────────────────────────────────────────┐
│  RenderSystem::Update()                  │
└────────────────┬─────────────────────────┘
                 │
┌────────────────▼─────────────────────────┐
│  렌더 큐 빌드                            │
│  - 가시 엔티티 순회                      │
│  - 렌더 컴포넌트 추출                    │
│  - 머티리얼/깊이로 정렬                  │
└────────────────┬─────────────────────────┘
                 │
┌────────────────▼─────────────────────────┐
│  커맨드 리스트 기록                      │
│  - 렌더 패스 시작                        │
│  - 파이프라인 상태 바인딩                │
│  - Indexed/Instanced 드로우              │
│  - 렌더 패스 종료                        │
└────────────────┬─────────────────────────┘
                 │
┌────────────────▼─────────────────────────┐
│  제출 & 제시                             │
└──────────────────────────────────────────┘
```

---

## 메모리 관리

### 전략 개요

**할당 카테고리:**

1. **영구 메모리**: 전체 애플리케이션 수명 동안 유지
   - 엔진 코어 시스템
   - 로드된 리소스

2. **레벨 메모리**: 씬 변경 시 클리어
   - 씬 엔티티
   - 레벨 특정 리소스

3. **프레임 메모리**: 매 프레임 리셋
   - 임시 계산
   - 렌더 커맨드 버퍼

**할당자 타입 (모두 구현됨):**

```cpp
// Core/include/Core/Memory/Allocators.h

// 프레임별 데이터를 위한 Linear 할당자
class LinearAllocator
{
    // 빠른 할당, 대량 해제
    // O(1) 할당, 개별 할당 해제 불가
};

// 고정 크기 객체를 위한 Pool 할당자
template<typename T>
class PoolAllocator
{
    // O(1) 할당/해제
    // 프리 리스트 구현
};

// 스코프 할당을 위한 Stack 할당자
class StackAllocator
{
    // LIFO 할당 패턴
    // 부분 해제를 위한 마커 지원
};
```

**사용 예시:**
```cpp
// 프레임별 할당
void RenderSystem::Update(World& world, float deltaTime)
{
    LinearAllocator frameAlloc(10 * MB);
    
    // 임시 렌더 커맨드 할당
    auto* commands = static_cast<RenderCommand*>(
        frameAlloc.Allocate(sizeof(RenderCommand) * entityCount)
    );
    
    // ... commands 사용
    
    // Reset()을 통해 스코프 종료 시 자동 해제
    frameAlloc.Reset();
}

// 객체 풀링
PoolAllocator particlePool(sizeof(Particle), 1000);
Particle* p = static_cast<Particle*>(particlePool.Allocate());
// ... 파티클 사용
particlePool.Deallocate(p);  // 풀로 반환

// 중첩된 스코프
StackAllocator stack(1 * MB);
auto marker1 = stack.GetMarker();
{
    void* temp = stack.Allocate(1024);
    // ... temp 사용
    
    auto marker2 = stack.GetMarker();
    {
        void* innerTemp = stack.Allocate(512);
        // ... innerTemp 사용
    }
    stack.FreeToMarker(marker2);  // 내부 스코프 해제
}
stack.FreeToMarker(marker1);  // 모두 해제
```

**성능 특성:**
- LinearAllocator: 10,000회 할당에 ~0.5 ms
- PoolAllocator: 10,000회 할당/해제 사이클에 ~2 ms
- StackAllocator: 10,000회 할당에 ~0.8 ms
- 표준 new/delete: ~50-100 ms (10-100배 느림)

---

## 스레딩 모델 (계획됨)

### 스레드 아키텍처

```
┌────────────────────────────────────────┐
│          메인 스레드                   │
│  - 게임 로직                           │
│  - ECS 시스템 업데이트 (순차)          │
│  - 렌더 커맨드 기록                    │
└────────────────────────────────────────┘

┌────────────────────────────────────────┐
│         렌더 스레드                    │
│  - 커맨드 리스트 제출                  │
│  - 리소스 업로드                       │
│  - GPU 동기화                          │
└────────────────────────────────────────┘

┌────────────────────────────────────────┐
│          Job 스레드들                  │
│  - 병렬 시스템 실행                    │
│  - 물리 broad phase                    │
│  - 컬링                                │
│  - 에셋 로딩                           │
└────────────────────────────────────────┘
```

### Job 시스템

```cpp
// Core/include/Core/Threading/Job.h
struct Job
{
    std::function<void()> function;
    std::atomic<int> unfinishedJobs;
    Job* parent = nullptr;
};

class JobSystem
{
public:
    JobHandle Schedule(Job* job);
    void Wait(JobHandle handle);
    
    // Parallel-for 헬퍼
    template<typename Func>
    void ParallelFor(size_t count, size_t batchSize, Func func);
};
```

**사용법:**
```cpp
// 병렬 컴포넌트 처리
jobSystem.ParallelFor(entityCount, 64, [&](size_t i)
{
    ProcessEntity(entities[i]);
});
```

---

## 향후 고려사항

### Phase 1: 기반 (75% 완료)
- [x] 프로젝트 구조
- [x] Core 시스템
  - [x] 메모리 할당자 (Linear, Pool, Stack)
  - [x] 로깅 시스템 (Console, File sinks)
  - [x] 어설션 매크로
- [x] Math 라이브러리 (SIMD와 함께 Vector, Matrix, Quaternion)
- [x] Platform 레이어 (Window, Input)
- [ ] 스레딩 시스템 (Job system, thread pool)

### Phase 2: 그래픽스 (시작 안 됨)
- [ ] DirectX 12 초기화
- [ ] 기본 렌더링 파이프라인
- [ ] 메시 및 텍스처 로딩
- [ ] 카메라 시스템

### Phase 3: 고급 ECS (시작 안 됨)
- [ ] 컴포넌트 쿼리
- [ ] 시스템 의존성
- [ ] 이벤트 시스템
- [ ] Archetype 최적화

### Phase 4: 게임플레이 시스템 (시작 안 됨)
- [ ] 물리 통합
- [ ] 충돌 감지
- [ ] AI 길찾기 (A*)
- [ ] 지형 생성

### Phase 5: 고급 렌더링 (시작 안 됨)
- [ ] PBR 머티리얼
- [ ] 그림자 매핑
- [ ] 포스트 프로세싱
- [ ] Render graph

### Phase 6: 툴 (시작 안 됨)
- [ ] 에셋 파이프라인
- [ ] 리소스 핫 리로딩
- [ ] ImGui 디버그 UI
- [ ] 기본 에디터 프로토타입

---

## 구현 진행 상황

### 완료된 시스템
| 시스템 | 상태 | 테스트 커버리지 | 코드 라인 수 |
|--------|------|----------------|-------------|
| Platform 레이어 | 완료 | 2개 테스트 | ~650 |
| 메모리 관리 | 완료 | 3개 테스트 | ~600 |
| Math 라이브러리 | 완료 | 1개 테스트 | ~400 |
| 로깅 시스템 | 완료 | 1개 테스트 | ~300 |
| **합계** | **12개 중 4개 서브시스템** | **7개 테스트** | **~2,200** |

### 진행 중
- 없음

### 계획됨
- 스레딩 시스템
- DirectX 12 초기화
- ECS 프레임워크
- 물리 엔진
- AI 시스템
- 씬 관리

---

## 참고 자료 및 영감

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

### 스레딩 & 성능
- [C++ Concurrency in Action by Anthony Williams](https://www.manning.com/books/c-plus-plus-concurrency-in-action)
- [Intel Threading Building Blocks](https://www.intel.com/content/www/us/en/developer/tools/oneapi/onetbb.html)

---

## 문서 히스토리

- **2025-10-13**: Platform 레이어 구현, 입력 시스템 세부사항 업데이트
- **2025-10-11**: 구현된 시스템으로 업데이트 (Memory, Math, Logging)
- **2025-10-05**: 초기 아키텍처 설계 문서
- 향후 업데이트는 여기에 기록됩니다

---

**참고**: 이 아키텍처는 프로젝트가 발전하고 새로운 요구사항이 등장함에 따라 변경될 수 있습니다. 실제 경험이 설계 개선으로 이어질 수 있습니다.

**최종 업데이트**: 2025-10-14