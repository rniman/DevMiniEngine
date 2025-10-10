# DevMiniEngine 코딩 컨벤션

**한국어** | **[English](./CodingConvention.md)**

## 📁 1. 파일 및 디렉토리 구조

- `.h`, `.cpp` 확장자 사용
- `pch.h`, `pch.cpp` → 프리컴파일 헤더
- 시스템/모듈 단위 디렉토리 구성 (예: `Renderer/`, `Core/`, `Resource/`)

---

## 🧱 2. 네이밍 규칙

### ✅ 클래스
- `PascalCase`

### ✅ 함수
- `PascalCase`
- **항상 동사로 시작**
- `bool` 반환 함수는 `Is`, `Has`, `Can`, `Should` 등 의미 있는 접두어 사용

### ✅ 변수
- 지역 변수: `camelCase`
- 멤버 변수: `m` 접두어 + `PascalCase`
  - 예: `mFrameIndex`, `mDevice`
- 정적 멤버 변수: `s` 접두어 + `PascalCase`

### ✅ 전역 변수
- `g` 접두어 + `PascalCase`
  - 예: `gRenderer`, `gInputSystem`

### ✅ 구조체 멤버
- `camelCase`, 접두어 없음

### ✅ 열거형
```cpp
enum class ResourceState
{
    Common,
    RenderTarget,
    CopyDest
};
```
- 열거형 클래스 이름: `PascalCase`
- 열거형 멤버: `PascalCase`
- 열거형 이름 반복 금지 (타입 안전성이 보장되므로)

---

## 🎨 3. 코드 스타일

### ✅ 기본 스타일
- 중괄호 `{}`는 항상 사용 (한 줄 if문도 예외 없음)
- 포인터는 타입과 붙인다 (`ID3D12Device* device`)
- `nullptr` 사용, `NULL` 금지
- `auto`는 타입이 명확할 때만 사용
- C 스타일 배열 지양, STL 컨테이너 우선
  - 정적 크기: `std::array`
  - 가변 크기: `std::vector`

### ✅ 변수 초기화 규칙 (단순화)

**기본 원칙: `=` 초기화 우선 사용**

```cpp
// ✅ 기본 스타일
int count = 0;
float pi = 3.14f;
auto result = Calculate();

// ✅ 컨테이너 초기화
std::vector<int> values = {1, 2, 3};
std::array<float, 3> pos = {0.0f, 1.0f, 0.0f};

// ✅ 멤버 변수 (클래스 정의 시)
class Renderer
{
private:
    UINT mFrameIndex = 0;
    float mDeltaTime = 0.0f;
};
```

**예외: 다음 경우에만 `{}` 직접 사용**
1. 집합 초기화(aggregate initialization)가 필요한 구조체
2. explicit 생성자 호출 시

```cpp
// 구조체 집합 초기화
Vertex vertex{position, normal, uv};

// explicit 생성자
std::vector<int> sizes(10);  // 10개 요소, 괄호 사용
```

> 📌 narrowing 방지는 컴파일러 경고(`/W4` 또는 `/Wall`)로 해결

### ✅ 복합 객체 초기화 스타일

- 초기화 항목이 여러 개이거나 복잡한 경우 줄바꿈 권장

```cpp
// ✅ 여러 항목일 경우
std::array<CD3DX12_DESCRIPTOR_RANGE, 3> descriptorRanges = {
    CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0),
    CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0),
    CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0),
};
```

> 📌 마지막 항목 뒤에도 쉼표(,) 권장 (버전 관리 편의성)

---

## 🔐 4. 메모리 관리

### ✅ 기본 원칙
- 로우 포인터 최소화, 참조(`&`) 적극 사용
- 읽기 전용 인자는 `const T&` 기본
- 스마트 포인터는 `const&`로 전달 (복사 비용 방지)
- 포인터는 null 가능성 또는 명시적 소유권 전달 시에만 사용

### ✅ 스마트 포인터 권장
- `std::unique_ptr` (기본 선택)
- `std::shared_ptr` (공유 필요 시)
- `Microsoft::WRL::ComPtr` (DirectX COM 객체)

```cpp
using Microsoft::WRL::ComPtr;
ComPtr<ID3D12Device> mDevice;
```

### ✅ 스마트 포인터 파라미터 전달

| 의도 | 시그니처 | 사용 시점 |
|------|---------|----------|
| 관찰만 함 (소유권 무관) | `T*` 또는 `T&` | **가장 일반적**, 객체 존재 보장 시 |
| 내부에 저장 (소유권 공유) | `shared_ptr<T>` | 참조 카운트 증가 필요 시 |
| 소유권 이전 | `unique_ptr<T>` | 단독 소유권 넘김 |
| 옵셔널 관찰 | `T*` | null 가능성 있음 |

```cpp
// ✅ 읽기만 할 때
void Draw(const Mesh& mesh);

// ✅ 소유권 공유
void SetTexture(std::shared_ptr<Texture> texture)
{
    mTexture = std::move(texture);
}

// ✅ 소유권 이전
std::unique_ptr<Mesh> CreateMesh()
{
    return std::make_unique<Mesh>();
}
```

---

## 📌 5. 상수 정의

### ✅ 기본 원칙
- `#define` 금지
- 컴파일 타임 상수: `constexpr` (전역은 **ALL_CAPS**)
- 런타임 상수: `const`

```cpp
constexpr int MAX_FRAME_COUNT = 3;
constexpr float PI = 3.14159f;
```

### ✅ 예외
- 외부 API 매크로는 허용 (`MAX_PATH`, `SUCCEEDED(hr)` 등)
- **상수 정의 용도로 `enum` 사용 금지**

---

## 🔍 6. const 사용 원칙

- 읽기 전용 매개변수: `const T&`
- 읽기 전용 멤버 함수: `...() const`
- 불변 지역 변수: `const` 또는 `constexpr`
- 포인터는 대상/자체 불변을 명확히 구분

---

## 🧭 7. 네임스페이스

- `using namespace std;` 허용
- `using Microsoft::WRL::ComPtr;` 선언 허용

---

## 🧩 8. 클래스 멤버 접근

### ✅ 기본 원칙
- `private` 멤버는 직접 접근 금지
- `SetX()`, `GetX()` 접근자/설정자 사용

```cpp
class Renderer
{
public:
    void SetFrameIndex(UINT index);
    UINT GetFrameIndex() const;

private:
    UINT mFrameIndex;
};
```

### ✅ 스마트 포인터 Get() 허용
```cpp
ID3D12CommandQueue* GetCmdQueue() const { return mCommandQueue.Get(); }
```

---

## 🧾 9. 함수 인자 정렬

### ✅ 줄바꿈 기준
- 인자 4개 이상 시 줄바꿈 권장
- 인자가 3개 이하라도 복잡한 타입이나 주석이 필요하면 줄바꿈

```cpp
auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
    backBuffer.Get(),
    D3D12_RESOURCE_STATE_RENDER_TARGET,
    D3D12_RESOURCE_STATE_PRESENT
);
```

### ✅ 함수 정의부 줄바꿈
```cpp
void CommandQueue::Init(
    const ComPtr<ID3D12Device>& device,
    const std::shared_ptr<SwapChain>& swapChain,
    const std::shared_ptr<DescriptorHeap>& descHeap
)
{
    // ...
}
```

---

## 🧱 10. 클래스 구성

### ✅ 접근 지정자 순서
```cpp
public:
protected:
private:
```

### ✅ 각 블록 내 정렬 순서

**public / protected:**
1. 생성자, 소멸자
2. 정적 생성 함수 (`Create()`, `GetInstance()`)
3. 주요 API 함수 (`Init()`, `Render()`, `Update()`)
4. Getter / Setter

**private:**
1. 내부 유틸 함수
2. 멤버 변수 (맨 아래)

```cpp
class Example
{
public:
    Example();
    ~Example();
    
    static std::shared_ptr<Example> Create();
    
    void Initialize();
    void Update();
    void Render();
    
    const ComPtr<ID3D12Device>& GetDevice() const;

protected:
    void ProtectedHelper();
    
private:
    void InitInternalResources();
    
    ComPtr<ID3D12Device> mDevice;
};
```

---

## 🛡️ 11. 함수 접근 수준

### ✅ 기본 원칙
- 외부 인터페이스: `public`
- 내부 구현 전용: `private`
- 파생 클래스용: `protected`

```cpp
class SwapChain
{
public:
    void Init(const WindowInfo& info);

private:
    void CreateSwapChain(...);
    void CreateRtv(...);
};
```

---

## 🧩 12. 헤더 파일 include

### ✅ 기본 원칙
- 헤더에서는 전방 선언(forward declaration) 우선
- 실제 구현 필요 시에만 include

```cpp
// ✅ 전방 선언
class Device;
class CommandQueue;

// 필요 시에만 include
#include <d3d12.h>
```

### ✅ include가 필요한 경우
- 클래스 크기/레이아웃 정보 필요
- 상속 관계 정의
- 템플릿 사용
- 인라인 함수 구현

---

## 🧭 13. 의존성 및 싱글톤 접근

| 계층 | 전역 접근 | 설명 |
|------|---------|------|
| **Engine Root / App** | ✅ 허용 | 애플리케이션 초기화 계층 |
| **Manager** | ⚠️ 제한적 | 명시적 전달 이상적, 전역 접근은 임시 |
| **Component / Object** | ❌ 지양 | 파라미터로 명시적 전달 |
| **Utility / Math** | 🚫 금지 | 완전한 독립성 유지 |

```cpp
// ❌ Component가 직접 싱글톤 접근
Renderer::GetInstance().Draw();

// ✅ 명시적 전달
component->Render(renderer);
```

### ✅ 테스트 가능성 고려
```cpp
// ✅ 의존성 주입으로 테스트 가능하게
class PhysicsSystem
{
private:
    Renderer* mRenderer;
    
public:
    PhysicsSystem(Renderer* renderer) : mRenderer(renderer) {}
    
    void Update()
    {
        mRenderer->DebugDraw(mColliders);
    }
};
```

---

## 🛡️ 14. 예외 처리 및 에러 핸들링

### ✅ HRESULT 처리
```cpp
inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw std::runtime_error("DirectX API call failed");
    }
}

// 사용
ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&mQueue)));
```

### ✅ 예외 사용 원칙
- **초기화 실패**: 예외 throw
- **런타임 에러**: 로깅 + 복구 또는 안전 종료
- **게임 로직**: 예외 지양, 반환값 사용

```cpp
// ✅ 초기화
void Renderer::Init()
{
    ThrowIfFailed(CreateDevice());
}

// ✅ 런타임
bool ResourceManager::LoadTexture(const std::string& path)
{
    if (!std::filesystem::exists(path))
    {
        LOG_ERROR("Texture not found: {}", path);
        return false;
    }
    return true;
}
```

### ✅ noexcept 사용
- 소멸자: 항상 `noexcept`
- 이동 연산자: 가능하면 `noexcept`
- 성능 중요 함수: `noexcept` 고려

---

## 📝 15. 주석 작성 규칙

### 기본 원칙
- **Self-documenting code 우선** - 주석보다 명확한 코드
- **What보다 Why** 설명
- **최소한으로** - 필요한 곳에만
- TODO/FIXME 태그 활용

### 주석이 필요한 경우
- 복잡한 알고리즘
- 비직관적인 결정 (성능 최적화 등)
- Public API의 제약사항
- 예외를 던질 수 있는 함수

### 주석이 불필요한 경우
- 자명한 Getter/Setter
- 생성자/소멸자 (특별한 이유 없으면)
- Private 내부 구현 함수
- 함수명과 파라미터로 의미가 명확한 경우

### 클래스 및 함수 주석
```cpp
// ✅ 클래스: 간단한 설명만
/**
 * @brief Linear allocator for frame-temporary data
 * Cannot free individual allocations. Use Reset().
 */
class LinearAllocator { ... };

// ✅ 복잡한 함수만 문서화
/**
 * @brief Renders triangles with custom transform
 * @note Requires active pipeline state
 */
bool DrawTriangles(const Vertex* vertices, size_t count, const Matrix& transform);

// ✅ 자명한 함수는 주석 없음
size_t GetCapacity() const { return mSize; }

// ✅ 중요한 동작만 inline 주석
void Deallocate(void* ptr) override;  // No-op
```

### TODO 태그
```cpp
// TODO: 멀티스레드 렌더링 지원 추가
// FIXME: 창 크기 변경 시 크래시 (Issue #42)
// NOTE: 최적화 필요 - 현재 100ms 소요
// HACK: 임시 해결책, 추후 수정 필요
```

### 피해야 할 주석
```cpp
// ❌ 당연한 것 설명
i++;  // i를 1 증가

// ✅ Why 설명
i++;  // 다음 프레임 인덱스로 이동

// ❌ 절대적 표현
// Perfect for all use cases

// ✅ 구체적 표현
// Well-suited for per-frame allocations (O(1) allocation)
```

---

## 🔧 16. 템플릿 사용

### ✅ 템플릿 네이밍
```cpp
// ✅ 간단한 경우
template<typename T>
class Array { };

// ✅ 의미 중요 시
template<typename VertexType, typename IndexType>
class Mesh { };

// ✅ C++20 Concepts
template<std::integral T>
T Add(T a, T b);
```

### ✅ 정의 위치
- 헤더에 전체 정의 (기본)
- 명시적 인스턴스화 필요 시에만 cpp 분리

---

## ⚡ 17. 성능 최적화

### ✅ 복사 최소화
```cpp
// ✅ const reference 반환
const std::vector<Vertex>& GetVertices() const
{
    return mVertices;
}

// ✅ 이동 의미론
std::vector<Vertex> ExtractVertices()
{
    return std::move(mVertices);
}
```

### ✅ Reserve와 Emplace
```cpp
std::vector<Vertex> vertices;
vertices.reserve(1000);  // 사전 할당
vertices.emplace_back(pos, normal, uv);  // 직접 생성
```

### ✅ 인라인 함수
- 간단한 Getter/Setter는 헤더 인라인
- 3줄 이하 간단 로직만

---

## 🔀 18. 멀티스레딩

### ✅ 동기화 프리미티브
```cpp
class ResourceManager
{
private:
    mutable std::shared_mutex mMutex;
    std::unordered_map<std::string, Resource> mResources;
    
public:
    Resource* Find(const std::string& name) const
    {
        std::shared_lock lock(mMutex);
        auto it = mResources.find(name);
        return it != mResources.end() ? &it->second : nullptr;
    }
    
    void Add(const std::string& name, Resource resource)
    {
        std::unique_lock lock(mMutex);
        mResources.emplace(name, std::move(resource));
    }
};
```

### ✅ Thread-safe 싱글톤
```cpp
class Engine
{
public:
    static Engine& GetInstance()
    {
        static Engine instance;  // C++11 이후 thread-safe
        return instance;
    }
    
private:
    Engine() = default;
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
};
```

---

## 🚫 19. 안티패턴

### ❌ 매크로 남용
```cpp
// ❌ 전역 접근 매크로
#define DEVICE Engine::GetInstance().GetDevice()

// ✅ 명시적 전달
void Init(Device* device);
```

### ❌ 불필요한 스마트 포인터
```cpp
// ❌ 소유권 필요 없는데 shared_ptr
void Draw(const std::shared_ptr<Mesh>& mesh);

// ✅ 참조로 충분
void Draw(const Mesh& mesh);
```

### ❌ 무의미한 주석
```cpp
// ❌ 당연한 것 설명
i++;  // i를 1 증가

// ✅ Why 설명
i++;  // 다음 프레임 인덱스로 이동
```