# DevMiniEngine 코딩 컨벤션

## 목차

1. [파일 및 디렉토리 구조](#1-파일-및-디렉토리-구조)
2. [네이밍 규칙](#2-네이밍-규칙)
3. [코드 스타일](#3-코드-스타일)
4. [메모리 관리](#4-메모리-관리)
5. [상수 정의](#5-상수-정의)
6. [const 사용 원칙](#6-const-사용-원칙)
7. [주석 작성 규칙](#7-주석-작성-규칙)
8. [네임스페이스](#8-네임스페이스)
9. [클래스 멤버 접근](#9-클래스-멤버-접근)
10. [함수 인자 정렬](#10-함수-인자-정렬)
11. [클래스 구성](#11-클래스-구성)
12. [함수 접근 수준](#12-함수-접근-수준)
13. [헤더 파일 include](#13-헤더-파일-include)
14. [의존성 및 싱글톤 접근](#14-의존성-및-싱글톤-접근)
15. [예외 처리 및 에러 핸들링](#15-예외-처리-및-에러-핸들링)
16. [템플릿 사용](#16-템플릿-사용)
17. [성능 최적화](#17-성능-최적화)
18. [멀티스레딩](#18-멀티스레딩)
19. [안티패턴](#19-안티패턴)
20. [전처리 지시자 및 매크로](#20-전처리-지시자-및-매크로)

---

## 1. 파일 및 디렉토리 구조

- `.h`, `.cpp` 확장자 사용
- `pch.h`, `pch.cpp` → 프리컴파일 헤더
- 시스템/모듈 단위 디렉토리 구성 (예: `Renderer/`, `Core/`, `Resource/`)

---

## 2. 네이밍 규칙

### 클래스
- `PascalCase`

### 함수
- `PascalCase`
- **항상 동사로 시작**
- `bool` 반환 함수는 `Is`, `Has`, `Can`, `Should` 등 의미 있는 접두어 사용

### 변수
- 지역 변수: `camelCase`
- 멤버 변수: `m` 접두어 + `PascalCase`
  - 예: `mFrameIndex`, `mDevice`
- 정적 멤버 변수: `s` 접두어 + `PascalCase`

### 전역 변수
- `g` 접두어 + `PascalCase`
  - 예: `gRenderer`, `gInputSystem`

### 구조체 멤버
- `camelCase`, 접두어 없음

### 열거형
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

## 3. 코드 스타일

### 기본 스타일
- 중괄호 `{}`는 항상 사용 (한 줄 if문도 예외 없음)
- 포인터는 타입과 붙인다 (`ID3D12Device* device`)
- `nullptr` 사용, `NULL` 금지
- `auto`는 타입이 명확할 때만 사용
- C 스타일 배열 지양, STL 컨테이너 우선
  - 정적 크기: `std::array`
  - 가변 크기: `std::vector`

---

### 변수 초기화 규칙

#### 초기화 철학

**왜 `=` 초기화를 우선하는가?**
- **가독성**: 전통적이고 익숙한 문법
- **의도 명확**: 값 대입이라는 의도가 분명
- **암시적 변환 허용**: 자연스러운 타입 변환
- **일관성**: 대부분의 케이스에 적용 가능

> **핵심 원칙**: "명시적 생성자 호출이 필요한 경우를 제외하고는 `=` 초기화 우선"

---

#### 초기화 스타일 우선순위

##### 1순위: 단순 값 초기화 - `=` 사용

```cpp
// 기본 타입
int count = 0;
float pi = 3.14f;
double epsilon = 1e-6;
bool enabled = true;

// 문자열
std::string name = "DevEngine";
const char* title = "Game Window";

// auto 초기화
auto result = Calculate();
auto device = CreateDevice();

// 멤버 변수 (클래스 정의 시)
class Renderer
{
private:
    UINT mFrameIndex = 0;
    float mDeltaTime = 0.0f;
    bool mInitialized = false;
};
```

---

##### 2순위: 집합 초기화 - `= {}` 조합

```cpp
// 컨테이너 초기화 리스트
std::vector<int> values = {1, 2, 3, 4, 5};
std::array<float, 3> position = {0.0f, 1.0f, 0.0f};

// POD 구조체 (Plain Old Data)
struct Vertex 
{
    float x, y, z;
    float r, g, b;
};
Vertex vertex = {0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f};

// 멤버 변수 배열 초기화
class Material
{
private:
    std::array<float, 4> mAmbient = {0.2f, 0.2f, 0.2f, 1.0f};
    std::vector<Texture*> mTextures = {};  // 빈 벡터
};
```

---

##### 3순위: 생성자 호출 - `{}` 단독 사용

**언제 사용:**
- `explicit` 생성자 호출 시
- 클래스 타입의 명시적 초기화
- 복합 객체를 직접 생성할 때

```cpp
// explicit 생성자
class Allocator
{
public:
    explicit Allocator(size_t size);  // explicit 키워드
};

Allocator allocator{1024};  // {} 필요
// Allocator allocator = 1024;  // ❌ 컴파일 에러

// 클래스 타입 직접 생성
Vector3 position{0.0f, 1.0f, 0.0f};
Matrix4x4 transform{Matrix4x4::Identity()};

// 복합 객체 생성
ComPtr<ID3D12Device> device{CreateDevice()};
```

---

#### 복합 객체 초기화 패턴

##### 패턴 1: 외부 변수 선언은 `=` 사용

```cpp
// 추천: 외부 선언에는 = 사용
std::array<CD3DX12_DESCRIPTOR_RANGE, 3> descriptorRanges = {
    CD3DX12_DESCRIPTOR_RANGE{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0},
    CD3DX12_DESCRIPTOR_RANGE{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0},
    CD3DX12_DESCRIPTOR_RANGE{D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0},
};
```

**규칙:**
- 변수 선언: `=` 사용 (일관성)
- 내부 원소: `{}` 사용 (명시적 생성자 호출)

---

##### 패턴 2: 여러 줄 초기화 시 줄바꿈

```cpp
// 여러 항목일 경우 줄바꿈 권장
std::vector<Vertex> vertices = {
    Vertex{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    Vertex{{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    Vertex{{ 0.0f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
};

// 파이프라인 상태 정의처럼 복잡한 구조체
D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};  // 먼저 제로 초기화
psoDesc.InputLayout = {inputElements, elementCount};
psoDesc.pRootSignature = rootSignature.Get();
psoDesc.VS = vertexShader;
psoDesc.PS = pixelShader;
// ... 나머지 멤버 설정
```

> **팁**: 마지막 항목 뒤에도 쉼표(,) 추가 권장 (버전 관리 diff 최소화)

---

#### 피해야 할 패턴

##### 안티패턴 1: Most Vexing Parse

```cpp
// ❌ 함수 선언으로 해석됨!
Widget widget();

// ✅ 해결책 1: = 초기화
Widget widget = Widget();

// ✅ 해결책 2: {} 초기화
Widget widget{};

// ✅ 해결책 3: 직접 호출 (C++11+)
Widget widget;
```

---

##### 안티패턴 2: 불필요한 {} 남용

```cpp
// ❌ 가독성 떨어짐
int count{0};
float pi{3.14f};
std::string name{"Engine"};

// ✅ 단순한 값은 = 사용
int count = 0;
float pi = 3.14f;
std::string name = "Engine";
```

---

##### 안티패턴 3: 혼재된 스타일

```cpp
// ❌ 일관성 없음
class BadExample
{
private:
    int mCount{0};           // {} 사용
    float mDelta = 0.0f;     // = 사용
    bool mEnabled{false};    // {} 사용
    std::string mName = "";  // = 사용
};

// ✅ 일관된 스타일
class GoodExample
{
private:
    int mCount = 0;
    float mDelta = 0.0f;
    bool mEnabled = false;
    std::string mName = "";
};
```

---

#### 초기화 결정 플로우차트

```
초기화할 변수가 있는가?
    ↓
┌───────────────────────────────────┐
│ explicit 생성자인가?               │
│ YES → {} 사용                     │
│ NO  → 다음 단계                   │
└─────────────┬─────────────────────┘
              ↓
┌───────────────────────────────────┐
│ 복합 객체 생성인가?                │
│ (클래스 생성자 직접 호출)          │
│ YES → {} 사용                     │
│ NO  → 다음 단계                   │
└─────────────┬─────────────────────┘
              ↓
┌───────────────────────────────────┐
│ 집합 초기화인가?                   │
│ (리스트로 여러 값)                 │
│ YES → = {} 조합                   │
│ NO  → 다음 단계                   │
└─────────────┬─────────────────────┘
              ↓
┌───────────────────────────────────┐
│ 단순 값 초기화                     │
│ → = 사용                          │
└───────────────────────────────────┘
```

---

#### 실전 예시 모음

```cpp
class InitializationExample
{
public:
    void DemonstrateInitialization()
    {
        // 1. 단순 값: = 사용
        int frameCount = 0;
        float deltaTime = 0.016f;
        
        // 2. 집합 초기화: = {} 조합
        std::vector<int> indices = {0, 1, 2, 2, 1, 3};
        std::array<float, 4> clearColor = {0.1f, 0.1f, 0.1f, 1.0f};
        
        // 3. explicit 생성자: {} 사용
        std::unique_ptr<Allocator> allocator{new Allocator(1024)};
        
        // 4. 클래스 타입: {} 사용
        Vector3 position{0.0f, 1.0f, 0.0f};
        
        // 5. 복합 객체 배열: = {} 조합
        std::array<Vector3, 3> triangle = {
            Vector3{-0.5f, -0.5f, 0.0f},
            Vector3{ 0.5f, -0.5f, 0.0f},
            Vector3{ 0.0f,  0.5f, 0.0f},
        };
    }

private:
    // 멤버 변수: = 우선
    UINT mWidth = 1280;
    UINT mHeight = 720;
    bool mVSync = true;
    
    // 컨테이너: = {} 조합
    std::vector<ComPtr<ID3D12Resource>> mBuffers = {};
    std::array<float, 16> mViewMatrix = {};
};
```

---

### 체크리스트

초기화 코드를 작성할 때:

- [ ] 단순 값 초기화는 `=`를 사용했는가?
- [ ] 집합 초기화는 `= {}`를 사용했는가?
- [ ] `explicit` 생성자는 `{}`만 사용했는가?
- [ ] 클래스 내에서 일관된 스타일을 유지하는가?
- [ ] Most Vexing Parse 문제를 피했는가?
- [ ] 불필요한 `{}` 남용을 피했는가?

---

### 요약

| 상황 | 사용 문법 | 예시 |
|------|----------|------|
| **단순 값** | `=` | `int x = 0;` |
| **집합 초기화** | `= {}` | `std::vector<int> v = {1,2,3};` |
| **explicit 생성자** | `{}` | `Allocator alloc{1024};` |
| **클래스 타입** | `{}` | `Vector3 pos{0,1,0};` |
| **복합 객체 배열** | `= {}` + 내부 `{}` | `std::array<T, 3> arr = {T{}, T{}, T{}};` |

**기억할 것:**
> "기본은 `=`, 명시적 생성이 필요할 때만 `{}`"

---

## 4. 메모리 관리

### 기본 원칙
- 로우 포인터 최소화, 참조(`&`) 적극 사용
- 읽기 전용 인자는 `const T&` 기본
- 스마트 포인터는 `const&`로 전달 (복사 비용 방지)
- 포인터는 null 가능성 또는 명시적 소유권 전달 시에만 사용

---

### 스마트 포인터 종류 및 선택

| 타입 | 용도 | 소유권 | 오버헤드 |
|------|------|--------|----------|
| `std::unique_ptr` | 단독 소유 | 독점 | 최소 (거의 없음) |
| `std::shared_ptr` | 공유 소유 | 공유 | 참조 카운트 |
| `ComPtr` | COM 객체 | COM 규칙 | 참조 카운트 + COM |

**선택 가이드:**
```cpp
// 기본: unique_ptr
std::unique_ptr<Texture> texture = LoadTexture("texture.png");

// 여러 곳에서 공유: shared_ptr
std::shared_ptr<Material> material = std::make_shared<Material>();
mMeshes[0]->SetMaterial(material);
mMeshes[1]->SetMaterial(material);  // 같은 material 공유

// DirectX/COM 객체: ComPtr
ComPtr<ID3D12Device> device;
```

---

### ComPtr 상세 가이드

#### ComPtr이란?

**Microsoft::WRL::ComPtr**은 COM(Component Object Model) 객체의 수명을 자동으로 관리하는 스마트 포인터입니다.
```cpp
#include <wrl/client.h>

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

// 사용
ComPtr<ID3D12Device> device;
ComPtr<IDXGIFactory4> factory;
```

**왜 ComPtr을 사용하는가?**
- DirectX/COM 객체는 `AddRef()`/`Release()` 수동 관리 필요
- ComPtr이 자동으로 참조 카운팅 처리
- 메모리 누수 방지

---

#### ComPtr 기본 사용법

##### 1. 생성 및 초기화
```cpp
// 기본 생성 (nullptr)
ComPtr<ID3D12Device> device;

// 함수에서 받기
HRESULT hr = D3D12CreateDevice(
    adapter,
    D3D_FEATURE_LEVEL_11_0,
    IID_PPV_ARGS(&device)  // &로 받음
);

// 다른 ComPtr에서 복사
ComPtr<ID3D12Device> device2 = device;  // AddRef() 자동 호출
```

##### 2. Get() - 로우 포인터 얻기
```cpp
ComPtr<ID3D12Device> device;

// DirectX API 호출 시
ID3D12Device* rawPtr = device.Get();
rawPtr->CreateCommandQueue(&desc, IID_PPV_ARGS(&queue));

// 함수에 전달
void DoSomething(ID3D12Device* device);
DoSomething(device.Get());  // 관찰만, 소유권 유지
```

**언제 사용:**
- 로우 포인터가 필요한 DirectX API 호출
- 함수에 전달 (소유권 이전 없이)
- null 체크 (`if (device.Get())`)

**주의:**
```cpp
// 나쁜 예: Get()으로 받은 포인터 저장
ID3D12Device* mDevice = device.Get();  // ❌ 위험! 수명 관리 안 됨

// 좋은 예: ComPtr로 저장
ComPtr<ID3D12Device> mDevice = device;  // ✅ 안전
```

##### 3. GetAddressOf() - 이중 포인터 얻기
```cpp
ComPtr<ID3D12Device> device;

// 잘못된 방법
D3D12CreateDevice(adapter, level, IID_PPV_ARGS(&device));  // ❌ 컴파일 에러

// 올바른 방법 1: GetAddressOf()
D3D12CreateDevice(
    adapter, 
    level, 
    __uuidof(ID3D12Device),
    reinterpret_cast<void**>(device.GetAddressOf())
);

// 올바른 방법 2: IID_PPV_ARGS 매크로 (권장)
D3D12CreateDevice(
    adapter,
    level,
    IID_PPV_ARGS(&device)  // 내부적으로 GetAddressOf() 호출
);
```

**언제 사용:**
- COM 객체를 생성하는 함수 호출
- 함수가 `void**` 또는 `T**` 파라미터를 요구할 때

**GetAddressOf() vs &연산자:**
```cpp
ComPtr<ID3D12Device> device;

// GetAddressOf(): 기존 참조 유지
ID3D12Device** ptr1 = device.GetAddressOf();  // AddRef/Release 안 함

// ReleaseAndGetAddressOf(): 기존 참조 해제
ID3D12Device** ptr2 = device.ReleaseAndGetAddressOf();  // Release() 먼저
```

##### 4. ReleaseAndGetAddressOf() - 재할당 전 해제
```cpp
ComPtr<ID3D12Device> device;

// 첫 번째 할당
D3D12CreateDevice(adapter1, level, IID_PPV_ARGS(&device));

// 두 번째 할당 - 기존 device 해제 필요
// 나쁜 예
D3D12CreateDevice(adapter2, level, IID_PPV_ARGS(&device));  // 메모리 누수!

// 좋은 예 1: Reset() 후 재할당
device.Reset();
D3D12CreateDevice(adapter2, level, IID_PPV_ARGS(&device));

// 좋은 예 2: ReleaseAndGetAddressOf() 사용
D3D12CreateDevice(
    adapter2,
    level,
    __uuidof(ID3D12Device),
    reinterpret_cast<void**>(device.ReleaseAndGetAddressOf())
);
```

**언제 사용:**
- ComPtr이 이미 객체를 보유하고 있을 때 재할당
- 함수가 출력 파라미터로 사용될 때

**GetAddressOf() vs ReleaseAndGetAddressOf():**
```cpp
ComPtr<ID3D12Device> device;

// 시나리오 1: 첫 할당 (device가 nullptr)
CreateDevice(IID_PPV_ARGS(&device));  // GetAddressOf() - OK

// 시나리오 2: 재할당 (device가 이미 객체 보유)
device.Reset();
CreateDevice(IID_PPV_ARGS(&device));  // GetAddressOf() - OK (Reset 후)

// 또는
CreateDevice(
    __uuidof(ID3D12Device),
    reinterpret_cast<void**>(device.ReleaseAndGetAddressOf())
);  // 자동으로 Release 후 재할당
```

##### 5. Reset() - 참조 해제
```cpp
ComPtr<ID3D12Device> device;

// 참조 해제 (Release() 호출)
device.Reset();  // device는 이제 nullptr

// 특정 포인터로 재설정
ID3D12Device* rawDevice = GetDeviceSomehow();
device.Reset();  // 기존 해제
device.Attach(rawDevice);  // 새 포인터 할당 (AddRef 안 함)
```

##### 6. As() - 인터페이스 캐스팅
```cpp
ComPtr<ID3D12Device> device;

// ID3D12Device5로 캐스팅
ComPtr<ID3D12Device5> device5;
HRESULT hr = device.As(&device5);

if (SUCCEEDED(hr))
{
    // device5 사용 가능
    device5->CreateMetaCommand(...);
}
```

---

#### ComPtr 함수 파라미터 전달

##### 패턴 1: 관찰만 (권장)
```cpp
// 로우 포인터로 전달
void RenderMesh(ID3D12Device* device, const Mesh& mesh)
{
    // device 사용, 소유권 변경 없음
    device->CreateCommandList(...);
}

// 호출
RenderMesh(mDevice.Get(), mesh);
```

**장점:**
- 오버헤드 없음
- 의도 명확 (관찰만)

##### 패턴 2: 소유권 공유
```cpp
// const ComPtr& 전달 (복사 방지)
void StoreDevice(const ComPtr<ID3D12Device>& device)
{
    mDevice = device;  // 참조 카운트 증가
}

// 또는 값으로 전달 후 이동
void StoreDevice(ComPtr<ID3D12Device> device)
{
    mDevice = std::move(device);  // 이동, 참조 카운트 유지
}
```

##### 패턴 3: 출력 파라미터
```cpp
// 이중 포인터로 반환
bool CreateDevice(ID3D12Device** outDevice)
{
    ComPtr<ID3D12Device> device;
    
    HRESULT hr = D3D12CreateDevice(
        adapter,
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&device)
    );
    
    if (FAILED(hr))
    {
        return false;
    }
    
    *outDevice = device.Detach();  // 소유권 이전
    return true;
}

// 호출
ComPtr<ID3D12Device> device;
if (CreateDevice(&device))
{
    // device 사용
}
```

---

#### 실전 예시

##### 예시 1: DX12Device 클래스
```cpp
// DX12Device.h
class DX12Device
{
public:
    bool Initialize();
    
    // Get() 사용 - 관찰만
    ID3D12Device* GetDevice() const { return mDevice.Get(); }
    IDXGIFactory4* GetFactory() const { return mFactory.Get(); }
    
private:
    // 멤버로 ComPtr 보유
    ComPtr<ID3D12Device> mDevice;
    ComPtr<IDXGIFactory4> mFactory;
};

// DX12Device.cpp
bool DX12Device::Initialize()
{
    // 1. Factory 생성
    HRESULT hr = CreateDXGIFactory2(
        0,
        IID_PPV_ARGS(&mFactory)  // GetAddressOf() 자동
    );
    
    if (FAILED(hr))
    {
        return false;
    }
    
    // 2. Adapter 선택
    ComPtr<IDXGIAdapter1> adapter;
    for (UINT i = 0; 
         mFactory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; 
         ++i)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);
        
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            continue;
        }
        
        // 3. Device 생성
        hr = D3D12CreateDevice(
            adapter.Get(),  // Get() 사용
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&mDevice)
        );
        
        if (SUCCEEDED(hr))
        {
            break;
        }
    }
    
    return mDevice != nullptr;
}
```

##### 예시 2: CommandQueue 생성
```cpp
class DX12CommandQueue
{
public:
    bool Initialize(ID3D12Device* device);  // 로우 포인터 받음
    
    ID3D12CommandQueue* GetQueue() const { return mQueue.Get(); }
    
private:
    ComPtr<ID3D12CommandQueue> mQueue;
    ComPtr<ID3D12Fence> mFence;
};

bool DX12CommandQueue::Initialize(ID3D12Device* device)
{
    // Queue 생성
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    
    HRESULT hr = device->CreateCommandQueue(
        &desc,
        IID_PPV_ARGS(&mQueue)  // ComPtr로 받기
    );
    
    if (FAILED(hr))
    {
        return false;
    }
    
    // Fence 생성
    hr = device->CreateFence(
        0,
        D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS(&mFence)
    );
    
    return SUCCEEDED(hr);
}
```

##### 예시 3: 리소스 관리
```cpp
class TextureManager
{
public:
    ComPtr<ID3D12Resource> LoadTexture(
        ID3D12Device* device,
        const std::string& path
    );
    
private:
    // 캐시: 경로 → 리소스
    std::unordered_map<std::string, ComPtr<ID3D12Resource>> mTextureCache;
};

ComPtr<ID3D12Resource> TextureManager::LoadTexture(
    ID3D12Device* device,
    const std::string& path
)
{
    // 캐시 확인
    auto it = mTextureCache.find(path);
    if (it != mTextureCache.end())
    {
        return it->second;  // ComPtr 복사 반환 (참조 카운트 증가)
    }
    
    // 텍스처 로드
    ComPtr<ID3D12Resource> texture;
    
    // ... 텍스처 생성 로직
    
    // 캐시에 저장
    mTextureCache[path] = texture;
    
    return texture;  // 반환 시 참조 카운트 증가
}
```

---

#### ComPtr vs std::shared_ptr 비교

| 기능 | ComPtr | shared_ptr |
|------|--------|-----------|
| **용도** | COM 객체 전용 | 일반 C++ 객체 |
| **참조 카운팅** | COM의 AddRef/Release | 자체 제어 블록 |
| **오버헤드** | COM 호출 비용 | 제어 블록 할당 |
| **스레드 안전** | COM 규칙 따름 | 참조 카운트만 안전 |
| **커스텀 삭제자** | 불가 | 가능 |
```cpp
// ComPtr: DirectX 객체
ComPtr<ID3D12Device> device;

// shared_ptr: 일반 객체
std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();

// 혼용 금지
// std::shared_ptr<ID3D12Device> device;  // ❌ 안 됨!
```

---

#### 일반적인 실수와 해결책

##### 실수 1: Get()으로 받은 포인터 저장
```cpp
// 나쁜 예
class BadExample
{
private:
    ID3D12Device* mDevice;  // ❌ 위험!
    
public:
    void Init(const ComPtr<ID3D12Device>& device)
    {
        mDevice = device.Get();  // 수명 관리 안 됨
    }
};

// 좋은 예
class GoodExample
{
private:
    ComPtr<ID3D12Device> mDevice;  // ✅ 안전
    
public:
    void Init(const ComPtr<ID3D12Device>& device)
    {
        mDevice = device;  // 참조 카운트 증가
    }
};
```

##### 실수 2: 메모리 누수 - 재할당 시 Release 안 함
```cpp
ComPtr<ID3D12Device> device;

// 첫 번째 할당
CreateDevice(&device);

// 두 번째 할당
// 나쁜 예
CreateDevice(&device);  // ❌ 첫 번째 device 누수!

// 좋은 예 1
device.Reset();
CreateDevice(&device);  // ✅

// 좋은 예 2
CreateDevice(device.ReleaseAndGetAddressOf());  // ✅
```

##### 실수 3: nullptr 체크 없이 사용
```cpp
ComPtr<ID3D12Device> device;

// 나쁜 예
device->CreateCommandQueue(...);  // ❌ nullptr일 수 있음!

// 좋은 예
if (device)
{
    device->CreateCommandQueue(...);  // ✅
}

// 또는
if (device.Get() != nullptr)
{
    device->CreateCommandQueue(...);  // ✅
}
```

---

### 스마트 포인터 파라미터 전달 (통합)

| 의도 | std::unique_ptr | std::shared_ptr | ComPtr |
|------|----------------|----------------|--------|
| **관찰만** | `T*` 또는 `T&` | `T*` 또는 `T&` | `T*` (Get()) |
| **소유권 공유** | N/A (불가) | `shared_ptr<T>` | `const ComPtr<T>&` |
| **소유권 이전** | `unique_ptr<T>` | N/A (이동) | `ComPtr<T>` (이동) |
| **출력 파라미터** | `unique_ptr<T>&` | `shared_ptr<T>&` | `T**` |
```cpp
// unique_ptr
void Process(const Mesh& mesh);  // 관찰
std::unique_ptr<Mesh> Create();  // 소유권 이전

// shared_ptr
void Store(std::shared_ptr<Texture> tex);  // 공유

// ComPtr
void Render(ID3D12Device* device);  // 관찰 (Get())
void Store(const ComPtr<ID3D12Device>& device);  // 공유
bool Create(ID3D12Device** outDevice);  // 출력
```

---

### 체크리스트

ComPtr 사용 시:

- [ ] 멤버 변수로 저장할 때 ComPtr 사용했는가?
- [ ] Get()은 관찰만 필요할 때만 사용했는가?
- [ ] IID_PPV_ARGS 매크로를 올바르게 사용했는가?
- [ ] 재할당 전에 Reset() 또는 ReleaseAndGetAddressOf() 사용했는가?
- [ ] nullptr 체크를 적절히 수행했는가?
- [ ] 로우 포인터를 멤버로 저장하지 않았는가?

---

### 요약

**ComPtr 핵심 메서드:**
- `Get()`: 로우 포인터 얻기 (관찰만, 수명 유지)
- `GetAddressOf()`: 이중 포인터 얻기 (할당용)
- `ReleaseAndGetAddressOf()`: 해제 후 재할당
- `Reset()`: 참조 해제
- `As()`: 인터페이스 캐스팅

**기본 원칙:**
- 저장은 ComPtr, 전달은 로우 포인터
- IID_PPV_ARGS 매크로 활용
- Get()으로 받은 포인터는 저장하지 말 것

---

## 5. 상수 정의

### 기본 원칙
- `#define` 금지
- 컴파일 타임 상수: `constexpr` (전역은 **ALL_CAPS**)
- 런타임 상수: `const`

```cpp
constexpr int MAX_FRAME_COUNT = 3;
constexpr float PI = 3.14159f;
```

### 예외
- 외부 API 매크로는 허용 (`MAX_PATH`, `SUCCEEDED(hr)` 등)
- **상수 정의 용도로 `enum` 사용 금지**

---

## 6. const 사용 원칙

- 읽기 전용 매개변수: `const T&`
- 읽기 전용 멤버 함수: `...() const`
- 불변 지역 변수: `const` 또는 `constexpr`
- 포인터는 대상/자체 불변을 명확히 구분

---

## 7. 주석 작성 규칙

### 언어 선택
- 모든 주석은 한글로 작성합니다
- 업계 표준 용어는 영어 그대로 사용 (예: ECS, RAII, PSO, DirectX)
- 영어-한글 혼재는 지양합니다

### 기본 철학
1. **Self-documenting code 우선** - 주석보다 명확한 코드가 최선
2. **What보다 Why** - 무엇을 하는지보다 왜 하는지 설명
3. **최소주의** - 코드로 표현 불가능한 것만 주석으로

---

### 주석 스타일 가이드

#### Public API 문서화 주석
```cpp
/**
 * @brief 프레임 임시 데이터를 위한 선형 할당자
 * 
 * @details 빠른 범프 포인터 할당 제공. 개별 해제 불가능하며
 *          Reset()을 통한 일괄 해제만 지원.
 * 
 * @note 스레드 안전하지 않음
 * @warning 할당된 메모리는 Reset() 전까지 유효
 */
class LinearAllocator 
{
public:
    /**
     * @brief 메모리 블록 할당
     * @param size 할당할 바이트 크기
     * @param alignment 정렬 요구사항 (기본: 16)
     * @return 할당된 메모리 포인터, 실패 시 nullptr
     * @throws std::bad_alloc 메모리 부족 시
     */
    void* Allocate(size_t size, size_t alignment = 16);
};
```

**문서화가 필요한 Public API:**
- 클래스/인터페이스 목적과 사용법
- 복잡한 파라미터나 반환값
- 전제조건, 부작용, 예외
- 성능 특성이나 제약사항

**문서화가 불필요한 경우:**
```cpp
// 자명한 함수는 주석 없음
size_t GetCapacity() const { return mCapacity; }
void SetEnabled(bool enabled) { mEnabled = enabled; }

// 이름과 타입으로 충분히 명확
Vector3 GetPosition() const;
void SetPosition(const Vector3& pos);
```

---

#### 구현 설명 주석 (Implementation Comments)
```cpp
void RenderSystem::Update()
{
    // GPU 펜스 값 증가로 프레임 동기화
    mFenceValue++;
    
    // CRITICAL: 이전 프레임 완료 대기 (병목 가능)
    // TODO: Triple buffering으로 개선 필요
    WaitForPreviousFrame();
    
    // 현재 백 버퍼에 렌더링
    auto backBuffer = mSwapChain->GetCurrentBackBuffer();
    RenderToTarget(backBuffer);
}
```

**좋은 구현 주석:**
- 비직관적인 알고리즘 설명
- 성능 최적화 이유
- 임시 해결책(workaround) 설명
- 중요한 전제조건이나 순서

**나쁜 구현 주석:**
```cpp
// 나쁜 예: 코드 그대로 반복
i++;  // i를 1 증가시킨다

// 좋은 예: 의도 설명
i++;  // 다음 프레임 인덱스로 이동
```

---

#### 복잡한 로직 설명

**패턴: 단계별 설명**
```cpp
// GPU 메모리 부족 시 3단계 fallback 처리:
// 1. 낮은 해상도 텍스처 로드 시도
// 2. 실패 시 기본 placeholder 텍스처 사용
// 3. 경고 로그 남기고 계속 진행
if (!AllocateHighResTexture())
{
    if (!AllocateLowResTexture())
    {
        UseDefaultTexture();
        LOG_WARN("텍스처 메모리 부족 - 기본 텍스처 사용");
    }
}
```

**패턴: 알고리즘 출처 명시**
```cpp
// Separating Axis Theorem (SAT)를 이용한 OBB 충돌 검사
// 참고: Real-Time Collision Detection (Christer Ericson, 2005) Chapter 4.4
bool CheckOBBCollision(const OBB& a, const OBB& b)
{
    // 15개 분리축 검사 (면 법선 6개 + 엣지 교차 9개)
    ...
}
```

**패턴: 성능 트레이드오프 설명**
```cpp
// 선형 탐색 사용 (O(n))
// 이유: 평균 엔티티 수 < 100이므로 해시맵보다 캐시 효율 좋음
// 벤치마크: 선형 탐색 ~50ns, unordered_map ~200ns
for (auto& entity : mEntities)
{
    ...
}
```

---

### TODO 태그 시스템

**우선순위별 태그**
```cpp
// FIXME: [P0] 즉시 수정 필요 - 메모리 누수 발생 (Issue #42)
// TODO: [P1] 다음 스프린트 - 멀티스레드 렌더링 지원
// OPTIMIZE: [P2] 성능 개선 - 현재 100ms, 목표 16ms
// HACK: [P2] 임시 해결책 - DX12 버그 우회, SDK 업데이트 시 제거
// NOTE: [정보] 이 함수는 메인 스레드에서만 호출 가능
```

**태그 사용 가이드:**
- `FIXME`: 버그, 크래시, 데이터 손실 등 치명적 문제
- `TODO`: 구현 예정 기능, 리팩토링
- `OPTIMIZE`: 성능 개선 여지
- `HACK`: 임시 해결책, 기술 부채
- `NOTE`: 중요한 정보, 제약사항

---

### 디버깅은 로깅으로

**안티패턴: 주석으로 디버깅**
```cpp
// 나쁜 예
void Update()
{
    // mOffset = 1024
    // mCapacity = 2048
    mOffset += size;
}
```

**올바른 방법: 로깅 시스템**
```cpp
// 좋은 예
void Update()
{
    LOG_TRACE("LinearAllocator: 할당 전 - Offset=%zu, Capacity=%zu", 
              mOffset, mCapacity);
    mOffset += size;
    LOG_TRACE("LinearAllocator: 할당 후 - Offset=%zu, Size=%zu", 
              mOffset, size);
}
```

---

### 레퍼런스 및 출처

**외부 자료 참조**
```cpp
/**
 * @brief PBR 기반 쿡-토런스 BRDF 구현
 * 
 * @see "Real Shading in Unreal Engine 4" - Brian Karis (SIGGRAPH 2013)
 * @see https://blog.selfshadow.com/publications/s2013-shading-course/
 */
Vector3 CookTorranceBRDF(...)
{
    // Epic Games 최적화 버전
    // 원본: D * F * G / (4 * NdotL * NdotV)
    // 최적화: D * Vis * F (Vis = G / (4 * NdotL * NdotV))
    ...
}
```

**표준 문서 참조**
```cpp
// DirectX 12 공식 문서:
// https://docs.microsoft.com/en-us/windows/win32/direct3d12/
// 
// Resource Barrier 최적화 가이드 참고
// 섹션: "Split Barriers for Improved Performance"
```

---

### 주석 체크리스트

작성 전 자문:
- [ ] 이 주석 없이 코드만으로 이해 가능한가?
- [ ] 함수/변수 이름 개선으로 주석을 제거할 수 있는가?
- [ ] "무엇"이 아닌 "왜"를 설명하는가?
- [ ] 6개월 후 내가 읽어도 이해할 수 있는가?
- [ ] 팀원이 읽어도 오해 없이 명확한가?

---

## 8. 네임스페이스

### 기본 원칙

**프로젝트 네임스페이스**
- 현재 프로젝트는 글로벌 네임스페이스 사용
- 학습 목적 프로젝트이므로 네임스페이스 오버헤드 최소화
- 향후 규모 확대 시 도입 검토 예정

**헤더 파일 규칙 (엄격)**
```cpp
// 헤더 파일 (.h)

// 금지: using namespace는 절대 사용 불가
// using namespace std;  // ❌ 컴파일 에러 유발 가능

// 허용: 명시적 타입 사용
std::vector<int> GetData() const;
std::string GetName() const;

// 허용: 타입 별칭 (header-only 유틸리티)
using ComPtr = Microsoft::WRL::ComPtr;
```

**구현 파일 규칙 (유연)**
```cpp
// 구현 파일 (.cpp)

// 허용: using namespace
using namespace std;
using Microsoft::WRL::ComPtr;

// 사용
vector<int> MyClass::GetData() const
{
    return mData;
}
```

---

### 왜 헤더에서 using namespace를 금지하는가?

**문제 1: 네임스페이스 오염**
```cpp
// MyClass.h
#pragma once
using namespace std;  // ❌ 나쁜 예

class MyClass
{
public:
    vector<int> GetData() const;  // std::vector인지 불명확
};

// 다른 파일에서 include
#include "MyClass.h"

// 의도치 않게 std 네임스페이스 전체가 노출됨
vector<int> data;  // std::를 안 붙여도 동작 (혼란)
```

**문제 2: 이름 충돌**
```cpp
// Graphics.h
#pragma once
using namespace std;  // ❌
using namespace DirectX;  // ❌

// 두 네임스페이스 모두 'min', 'max' 함수 보유
// 어느 것이 호출될지 모호함 - 컴파일 에러!
auto result = min(a, b);  // std::min? DirectX::min?
```

**문제 3: 연쇄 전파**
```cpp
// A.h
using namespace std;

// B.h
#include "A.h"  // B.h를 include한 모든 파일이 std를 상속받음

// C.h
#include "B.h"  // 의도치 않게 std 네임스페이스 노출

// 100개 파일 후...
// 누가 언제 using namespace를 선언했는지 추적 불가능
```

---

### 올바른 사용 패턴

#### 패턴 1: 헤더에서 명시적 타입
```cpp
// Device.h
#pragma once
#include <memory>
#include <vector>
#include <wrl/client.h>

// 타입 별칭은 허용 (간결성)
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class Device
{
public:
    // 명시적 std::
    std::vector<std::string> GetAdapterNames() const;
    std::shared_ptr<CommandQueue> CreateCommandQueue();
    
    // ComPtr 별칭 사용
    ComPtr<ID3D12Device> GetD3D12Device() const;
    
private:
    ComPtr<ID3D12Device> mDevice;
    std::vector<std::string> mAdapterNames;
};
```

#### 패턴 2: 구현 파일에서 using namespace
```cpp
// Device.cpp
#include "Device.h"
#include <dxgi1_6.h>

// .cpp에서는 자유롭게 사용
using namespace std;
using Microsoft::WRL::ComPtr;

vector<string> Device::GetAdapterNames() const
{
    // std:: 생략 가능
    vector<string> names;
    
    for (const auto& name : mAdapterNames)
    {
        names.push_back(name);
    }
    
    return names;
}
```

#### 패턴 3: 함수 내부 스코프 제한 (선택적)
```cpp
// Device.cpp
#include "Device.h"

// 함수 내부로 제한 (선택적 패턴)
void Device::Initialize()
{
    using namespace std;
    
    vector<string> names;
    cout << "Initializing..." << endl;
    
}  // 여기서 using namespace 효과 종료

// 다른 함수는 영향 없음
void Device::Shutdown()
{
    std::cout << "Shutting down..." << std::endl;  // std:: 필요
}
```

---

### 허용되는 using 선언

#### 전역 수준 (헤더)
```cpp
// 타입 별칭만 허용
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

using uint = unsigned int;
using byte = unsigned char;
```

#### 전역 수준 (구현 파일)
```cpp
// .cpp에서는 자유롭게
using namespace std;
using namespace DirectX;
using Microsoft::WRL::ComPtr;

// 또는 선택적으로
using std::vector;
using std::string;
using std::shared_ptr;
```

---

### 실제 프로젝트 예시

#### 예시 1: Platform 레이어
```cpp
// Win32Window.h
#pragma once
#include <string>

// 전방 선언
struct HWND__;
struct HINSTANCE__;

class Win32Window
{
public:
    bool Create(const std::string& title, int width, int height);
    
private:
    HWND__* mHwnd;
    std::string mTitle;  // 명시적 std::
};

// Win32Window.cpp
#include "Win32Window.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using namespace std;  // .cpp에서만 사용

bool Win32Window::Create(const string& title, int width, int height)
{
    mTitle = title;  // std:: 생략
    // ...
}
```

#### 예시 2: Graphics 레이어
```cpp
// DX12Device.h
#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class DX12Device
{
public:
    bool Initialize();
    ComPtr<ID3D12Device> GetDevice() const { return mDevice; }
    
private:
    ComPtr<ID3D12Device> mDevice;
    ComPtr<IDXGIFactory4> mFactory;
};

// DX12Device.cpp
#include "DX12Device.h"

using Microsoft::WRL::ComPtr;  // .cpp에서 사용

bool DX12Device::Initialize()
{
    ComPtr<IDXGIFactory4> factory;
    // ...
}
```

---

### 향후 고려사항

프로젝트 규모가 커지면 네임스페이스 도입 검토:
```cpp
// 미래의 구조 (예시)
namespace DevMini
{
    namespace Core
    {
        class Logger { ... };
        class MemoryManager { ... };
    }
    
    namespace Graphics
    {
        class Device { ... };
        class Renderer { ... };
    }
    
    namespace Physics
    {
        class World { ... };
    }
}

// 사용
DevMini::Core::Logger& logger = DevMini::Core::Logger::GetInstance();

// 또는 별칭
namespace DM = DevMini;
DM::Core::Logger& logger = DM::Core::Logger::GetInstance();
```

**현재는 도입하지 않는 이유:**
- 학습 목적 프로젝트
- 규모가 크지 않음 (현재 ~3,000 라인)
- 타이핑 오버헤드
- 외부 배포 없음 (이름 충돌 위험 낮음)

---

### 체크리스트

새로운 헤더 파일을 작성할 때:

- [ ] `using namespace`를 헤더에 사용하지 않았는가?
- [ ] 모든 타입에 네임스페이스를 명시했는가? (std::, DirectX:: 등)
- [ ] 타입 별칭이 필요하다면 명시적으로 선언했는가?
- [ ] .cpp 파일에서만 `using namespace`를 사용했는가?

---

### 요약

| 위치 | using namespace | 명시적 타입 | 타입 별칭 |
|------|----------------|------------|----------|
| **헤더 파일 (.h)** | ❌ 금지 | ✅ 필수 | ✅ 허용 |
| **구현 파일 (.cpp)** | ✅ 허용 | ✅ 선택 | ✅ 허용 |
| **함수 내부** | ✅ 허용 | ✅ 선택 | ✅ 허용 |

**기억할 것:**
- 헤더 = Public API → 명시적이고 안전하게
- 구현 = Internal → 편의성 우선

**원칙:**
> "헤더는 남을 위해, 구현은 나를 위해"
  
---

## 9. 클래스 멤버 접근

### 기본 원칙
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

### 스마트 포인터 Get() 허용
```cpp
ID3D12CommandQueue* GetCmdQueue() const { return mCommandQueue.Get(); }
```

---

## 10. 함수 인자 정렬

### 줄바꿈 기준

다음 중 하나라도 해당하면 인자를 줄바꿈을 **권장**합니다:
1. 인자 4개 이상
2. 한 줄 길이 100자 초과
3. 템플릿 타입 2개 이상 포함

> **Note**: 기준에 해당하더라도 가독성이 좋다면 한 줄 작성 가능

### 줄바꿈 스타일 (단일 패턴)

**모든 인자를 새 줄에 작성**
```cpp
// 함수 선언
void CreateBuffer(
    ID3D12Device* device,
    size_t size,
    D3D12_RESOURCE_FLAGS flags,
    D3D12_RESOURCE_STATES initialState
);

// 함수 호출
CreateBuffer(
    mDevice.Get(),
    bufferSize,
    D3D12_RESOURCE_FLAG_NONE,
    D3D12_RESOURCE_STATE_COMMON
);
```

### 줄바꿈 하지 않아도 되는 경우
```cpp
// 인자 4개지만 짧고 명확한 경우
void SetRect(int x, int y, int w, int h);
void SetColor(float r, float g, float b, float a);

// 인자 3개 이하이고 한 줄이 100자 이하
void SetPosition(float x, float y, float z);
void Init(Device* device, Queue* queue);

// 타입이 짧고 의미가 명확
bool CheckCollision(const AABB& a, const AABB& b, Vector3& normal);
```

### 반드시 줄바꿈 해야 하는 경우
```cpp
// 복잡한 템플릿이나 긴 타입명
std::shared_ptr<CommandQueue> CreateCommandQueue(
    const ComPtr<ID3D12Device>& device,
    D3D12_COMMAND_LIST_TYPE type,
    D3D12_COMMAND_QUEUE_PRIORITY priority
);

// 한 줄이 100자를 명백히 초과
void InitializeGraphicsSystem(
    const WindowDesc& windowDesc,
    const RenderSettings& settings,
    bool enableValidation
);
```

### 판단 기준

**한 줄 작성 가능한 경우:**
- 모든 인자 타입과 이름이 간결함
- 전체 길이가 80-100자 이내
- 의미 파악이 즉시 가능

**줄바꿈 해야 하는 경우:**
- 인자를 읽기 위해 스크롤 필요
- 타입이나 이름이 복잡함
- 비슷한 타입 인자가 많아 구분 어려움

### 예시 비교
```cpp
// OK: 짧고 명확
void SetViewport(float x, float y, float w, float h);

// 권장하지 않음: 스크롤 필요, 가독성 떨어짐
void CreateGraphicsPipelineState(ID3D12Device* device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC* desc, ID3D12PipelineState** outState);

// 권장: 인자별로 명확히 구분
void CreateGraphicsPipelineState(
    ID3D12Device* device,
    const D3D12_GRAPHICS_PIPELINE_STATE_DESC* desc,
    ID3D12PipelineState** outState
);
```

### 실전 팁

**일관성 우선**
```cpp
// 같은 클래스 내 비슷한 함수는 같은 스타일 사용
class Renderer
{
public:
    // 둘 다 한 줄 또는 둘 다 여러 줄
    void SetViewport(float x, float y, float w, float h);
    void SetScissor(int x, int y, int w, int h);
    
    // 또는
    void SetViewport(
        float x, float y, float w, float h
    );
    void SetScissor(
        int x, int y, int w, int h
    );
};
```

**애매하면 줄바꿈**
- 확신이 서지 않으면 줄바꿈 선택
- 나중에 인자 추가 시 수정 최소화

### if/while 조건문에서 함수 호출

**권장: 조건 변수 추출**
```cpp
// 가장 권장 - 가독성과 디버깅
bool success = mSwapChain->Initialize(
    mFactory.Get(),
    mQueue->GetQueue(),
    hwnd,
    width,
    height
);

if (!success)
{
    LOG_ERROR("Initialization failed");
    return false;
}
```

**허용: if문 안에서 직접 호출**
```cpp
// 허용 - 닫는 괄호는 함수 시작과 같은 들여쓰기
if (!mSwapChain->Initialize(
        mFactory.Get(),
        mQueue->GetQueue(),
        hwnd,
        width,
        height
    ))
{
    LOG_ERROR("Initialization failed");
    return false;
}
```

**피해야 할 패턴**
```cpp
// 나쁜 예: 닫는 괄호 위치 불명확
if (!mSwapChain->Initialize(
        mFactory.Get(),
        mQueue->GetQueue()
        ))  // ❌ 혼란스러움
{
}

// 나쁜 예: 에러 처리 없음
if (!mSwapChain->Initialize(...))  // ❌ 실패 이유 모름
{
    return false;
}
```

---

## 11. 클래스 구성

### 접근 지정자 순서
```cpp
public:      // 외부 인터페이스
protected:   // 파생 클래스용
private:     // 내부 구현
```

---

### 각 블록 내 정렬 순서

#### public 블록
1. 타입 정의 (using, enum class)
2. 생성자, 소멸자
3. 복사/이동 연산자 (삭제 포함)
4. 정적 생성 함수
5. 주요 API 함수 (수명 주기 순서)
6. Getter / Setter
7. 연산자 오버로딩

#### protected 블록
1. 파생 클래스용 헬퍼 함수
2. 파생 클래스용 멤버 변수

#### private 블록
1. 내부 헬퍼 함수
2. 멤버 변수 (맨 아래)

---

### 기본 템플릿
```cpp
class ClassName
{
public:
    // 1. 타입 정의
    using ResourcePtr = std::shared_ptr<Resource>;
    enum class State { Idle, Active, Paused };
    
    // 2. 생성자/소멸자
    ClassName();
    explicit ClassName(int size);
    ~ClassName();
    
    // 3. 복사/이동
    ClassName(const ClassName&) = delete;
    ClassName& operator=(const ClassName&) = delete;
    ClassName(ClassName&&) noexcept = default;
    ClassName& operator=(ClassName&&) noexcept = default;
    
    // 4. 정적 생성
    static std::unique_ptr<ClassName> Create(int size);
    
    // 5. 주요 API (수명 주기 순서)
    bool Initialize();
    void Update(float deltaTime);
    void Render();
    void Shutdown();
    
    // 6. Getter/Setter
    int GetSize() const { return mSize; }
    void SetSize(int size) { mSize = size; }
    
    // 7. 연산자
    bool operator==(const ClassName& other) const;

protected:
    // 파생 클래스용 헬퍼
    void ProtectedHelper();
    
    // 파생 클래스용 멤버
    int mProtectedValue = 0;

private:
    // 내부 헬퍼
    void InitializeInternal();
    void CleanupResources();
    
    // 멤버 변수 (맨 아래)
    int mSize = 0;
    State mState = State::Idle;
    std::vector<ResourcePtr> mResources;
};
```

---

### 실제 프로젝트 예시

#### 예시 1: Platform 레이어 - Input 시스템
```cpp
// Platform/include/Platform/Input.h
#pragma once
#include "KeyCode.h"
#include <array>

class Input
{
public:
    // 타입 정의
    static constexpr size_t KeyCount = 256;
    
    // 생성자/소멸자
    Input() = default;
    ~Input() = default;
    
    // 복사 금지 (상태 관리 클래스)
    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;
    
    // 프레임 수명 주기 (호출 순서대로)
    void Update();  // 프레임 시작: 이전 상태 저장
    void Reset();   // 프레임 종료: 이벤트 상태 초기화
    
    // 키보드 쿼리 (프레임 중간)
    bool IsKeyPressed(KeyCode key) const;   // 이번 프레임에 눌림
    bool IsKeyDown(KeyCode key) const;      // 현재 눌려있음
    bool IsKeyReleased(KeyCode key) const;  // 이번 프레임에 떼어짐
    
    // 마우스 쿼리
    bool IsMouseButtonPressed(int button) const;
    bool IsMouseButtonDown(int button) const;
    bool IsMouseButtonReleased(int button) const;
    
    // 마우스 위치
    int GetMouseX() const { return mMouseX; }
    int GetMouseY() const { return mMouseY; }
    float GetMouseWheelDelta() const { return mMouseWheelDelta; }
    
    // 내부 이벤트 처리 (Win32Window가 호출)
    void OnKeyDown(KeyCode key);
    void OnKeyUp(KeyCode key);
    void OnMouseMove(int x, int y);
    void OnMouseButton(int button, bool pressed);
    void OnMouseWheel(float delta);

private:
    // 내부 헬퍼
    bool IsValidKey(KeyCode key) const;
    bool IsValidMouseButton(int button) const;
    
    // 키보드 상태 (더블 버퍼링)
    std::array<bool, KeyCount> mKeyState = {};
    std::array<bool, KeyCount> mPrevKeyState = {};
    
    // 마우스 상태
    std::array<bool, 3> mMouseButtonState = {};
    std::array<bool, 3> mPrevMouseButtonState = {};
    int mMouseX = 0;
    int mMouseY = 0;
    float mMouseWheelDelta = 0.0f;
};
```

---

#### 예시 2: Graphics 레이어 - DX12Device
```cpp
// Graphics/include/Graphics/DX12/DX12Device.h
#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <memory>

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class DX12Device
{
public:
    // 타입 정의
    struct AdapterInfo
    {
        std::wstring description;
        size_t dedicatedVideoMemory;
        size_t dedicatedSystemMemory;
        size_t sharedSystemMemory;
    };
    
    // 생성자/소멸자
    DX12Device() = default;
    ~DX12Device() = default;
    
    // 복사/이동 금지
    DX12Device(const DX12Device&) = delete;
    DX12Device& operator=(const DX12Device&) = delete;
    
    // 초기화/정리
    bool Initialize();
    void Shutdown();
    
    // 디바이스 쿼리
    ID3D12Device* GetDevice() const { return mDevice.Get(); }
    IDXGIFactory4* GetFactory() const { return mFactory.Get(); }
    D3D_FEATURE_LEVEL GetFeatureLevel() const { return mFeatureLevel; }
    
    // 어댑터 정보
    const AdapterInfo& GetAdapterInfo() const { return mAdapterInfo; }
    bool IsRaytracingSupported() const { return mRaytracingSupported; }

private:
    // 초기화 헬퍼
    bool CreateFactory();
    bool SelectAdapter();
    bool CreateDevice();
    bool CheckFeatureSupport();
    
    // 디바이스 정보 수집
    void QueryAdapterInfo(IDXGIAdapter1* adapter);
    void CheckRaytracingSupport();
    
    // DirectX 객체
    ComPtr<ID3D12Device> mDevice;
    ComPtr<IDXGIFactory4> mFactory;
    ComPtr<IDXGIAdapter1> mAdapter;
    
    // 디바이스 정보
    D3D_FEATURE_LEVEL mFeatureLevel = D3D_FEATURE_LEVEL_11_0;
    AdapterInfo mAdapterInfo = {};
    bool mRaytracingSupported = false;
};
```

---

#### 예시 3: Core 레이어 - LinearAllocator
```cpp
// Core/include/Core/Memory/LinearAllocator.h
#pragma once
#include "Allocator.h"
#include <cstddef>

class LinearAllocator : public Allocator
{
public:
    // 타입 정의
    static constexpr size_t DefaultAlignment = 16;
    
    // 생성자/소멸자
    explicit LinearAllocator(size_t size);
    ~LinearAllocator() override;
    
    // 복사/이동
    LinearAllocator(const LinearAllocator&) = delete;
    LinearAllocator& operator=(const LinearAllocator&) = delete;
    LinearAllocator(LinearAllocator&& other) noexcept;
    LinearAllocator& operator=(LinearAllocator&& other) noexcept;
    
    // Allocator 인터페이스 구현
    void* Allocate(size_t size, size_t alignment = DefaultAlignment) override;
    void Deallocate(void* ptr) override;  // No-op
    void Reset() override;
    
    // 상태 쿼리
    size_t GetSize() const { return mSize; }
    size_t GetUsed() const { return mOffset; }
    size_t GetRemaining() const { return mSize - mOffset; }
    bool CanAllocate(size_t size, size_t alignment = DefaultAlignment) const;

private:
    // 내부 헬퍼
    size_t AlignOffset(size_t offset, size_t alignment) const;
    void LogAllocation(void* ptr, size_t size) const;
    
    // 메모리 블록
    void* mMemory = nullptr;
    size_t mSize = 0;
    size_t mOffset = 0;
};
```

---

#### 예시 4: 싱글톤 패턴 - Logger
```cpp
// Core/include/Core/Logging/Logger.h
#pragma once
#include "LogSink.h"
#include <memory>
#include <vector>
#include <mutex>
#include <string>

class Logger
{
public:
    // 타입 정의
    enum class Level { Trace, Debug, Info, Warning, Error, Fatal };
    enum class Category { Core, Graphics, Physics, AI, Audio, Input, Memory };
    
    // 싱글톤 접근
    static Logger& GetInstance();
    
    // 복사/이동 금지
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    // Sink 관리
    void AddSink(std::unique_ptr<LogSink> sink);
    void RemoveSink(LogSink* sink);
    void ClearSinks();
    
    // 로깅
    void Log(Level level, Category category, 
             const std::string& message,
             const char* file, int line);
    
    void Flush();
    
    // 설정
    void SetMinLevel(Level level) { mMinLevel = level; }
    Level GetMinLevel() const { return mMinLevel; }
    
    void SetEnabled(bool enabled) { mEnabled = enabled; }
    bool IsEnabled() const { return mEnabled; }

private:
    // 생성자/소멸자 (private - 싱글톤)
    Logger() = default;
    ~Logger() = default;
    
    // 내부 헬퍼
    bool ShouldLog(Level level) const;
    std::string FormatMessage(
        Level level,
        Category category,
        const std::string& message,
        const char* file,
        int line
    ) const;
    
    const char* LevelToString(Level level) const;
    const char* CategoryToString(Category category) const;
    
    // Sinks
    std::vector<std::unique_ptr<LogSink>> mSinks;
    
    // 설정
    Level mMinLevel = Level::Trace;
    bool mEnabled = true;
    
    // 스레드 안전
    mutable std::mutex mMutex;
};
```

---

#### 예시 5: 추상 인터페이스 - Allocator
```cpp
// Core/include/Core/Memory/Allocator.h
#pragma once
#include <cstddef>

class Allocator
{
public:
    // 타입 정의
    static constexpr size_t DefaultAlignment = 16;
    
    // 가상 소멸자
    virtual ~Allocator() = default;
    
    // 순수 가상 인터페이스
    virtual void* Allocate(size_t size, size_t alignment = DefaultAlignment) = 0;
    virtual void Deallocate(void* ptr) = 0;
    virtual void Reset() = 0;
    
    // 공통 유틸리티 (non-virtual)
    template<typename T>
    T* AllocateObject()
    {
        return static_cast<T*>(Allocate(sizeof(T), alignof(T)));
    }
    
    template<typename T>
    T* AllocateArray(size_t count)
    {
        return static_cast<T*>(Allocate(sizeof(T) * count, alignof(T)));
    }

protected:
    // 파생 클래스용 헬퍼
    static size_t AlignUp(size_t value, size_t alignment);
    static bool IsAligned(const void* ptr, size_t alignment);
    
    // 파생 클래스용 통계
    size_t mTotalAllocated = 0;
    size_t mAllocationCount = 0;
};
```

---

### 클래스 구성 패턴별 가이드

#### 패턴 1: 데이터 클래스 (POD-like)
```cpp
struct Vertex
{
    // public 멤버만
    Vector3 position;
    Vector3 normal;
    Vector2 texCoord;
    Color color;
    
    // 간단한 헬퍼 함수는 OK
    bool IsValid() const 
    {
        return !std::isnan(position.x);
    }
};
```

#### 패턴 2: RAII 리소스 관리 클래스
```cpp
class Texture
{
public:
    // 생성/소멸이 리소스 수명과 직결
    explicit Texture(const std::string& path);
    ~Texture();
    
    // 복사 금지, 이동 허용
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&&) noexcept;
    Texture& operator=(Texture&&) noexcept;
    
    // 리소스 접근
    ID3D12Resource* GetResource() const { return mResource.Get(); }
    
private:
    ComPtr<ID3D12Resource> mResource;
    std::string mPath;
};
```

#### 패턴 3: 시스템 클래스 (수명 주기 관리)
```cpp
class RenderSystem
{
public:
    // 명시적 초기화/정리
    bool Initialize(Device* device);
    void Shutdown();
    
    // 프레임 주기
    void BeginFrame();
    void Render(const Scene& scene);
    void EndFrame();
    
private:
    bool mInitialized = false;
    Device* mDevice = nullptr;
};
```

---

### 멤버 변수 정렬 규칙

**우선순위:**
1. 크기가 큰 것부터 (캐시 라인 최적화)
2. 자주 함께 사용되는 것끼리
3. 관련된 것끼리 그룹화 (빈 줄로 구분)
```cpp
class Example
{
private:
    // DirectX 객체 (큰 ComPtr)
    ComPtr<ID3D12Device> mDevice;
    ComPtr<ID3D12CommandQueue> mQueue;
    
    // STL 컨테이너 (큰 객체)
    std::vector<Resource> mResources;
    std::string mName;
    
    // 기본 타입 (작은 객체)
    uint64_t mFrameCount = 0;
    float mDeltaTime = 0.0f;
    int mWidth = 0;
    int mHeight = 0;
    
    // bool은 맨 아래 (1바이트)
    bool mInitialized = false;
    bool mEnabled = true;
};
```

---

### 체크리스트

새로운 클래스를 작성할 때:

- [ ] 접근 지정자 순서가 올바른가? (public → protected → private)
- [ ] 각 블록 내 순서가 올바른가?
- [ ] 생성자/소멸자가 리소스 관리와 일치하는가?
- [ ] 복사/이동 의미론을 명시했는가?
- [ ] 멤버 변수가 맨 아래에 있는가?
- [ ] 관련된 멤버들이 그룹화되어 있는가?
- [ ] 주요 API 함수가 수명 주기 순서로 정렬되어 있는가?

---

### 요약

**기본 원칙:**
- public이 먼저 (사용자가 먼저 볼 것)
- 수명 주기 순서대로 (생성 → 사용 → 소멸)
- 멤버 변수는 마지막 (구현 세부사항)

**실전 팁:**
- 비슷한 클래스는 비슷한 구조 유지
- 논리적 그룹화로 가독성 향상
- 빈 줄로 섹션 구분

---

## 12. 함수 접근 수준

### 기본 원칙
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

## 13. 헤더 파일 include

### 기본 원칙
- 헤더에서는 전방 선언(forward declaration) 우선
- 실제 구현 필요 시에만 include

```cpp
// 전방 선언
class Device;
class CommandQueue;

// 필요 시에만 include
#include <d3d12.h>
```

### include가 필요한 경우
- 클래스 크기/레이아웃 정보 필요
- 상속 관계 정의
- 템플릿 사용
- 인라인 함수 구현

### Platform 레이어 특수 사례

```cpp
// Win32 API 오염 최소화를 위한 전방 선언
// Platform/include/Platform/Windows/Win32Window.h
struct HWND__;      // Windows 타입 전방 선언
struct HINSTANCE__;

class Win32Window
{
private:
    HWND__* mHwnd;  // 헤더에서 void* 대신 타입 안전한 전방 선언
};

// 실제 Windows.h include는 .cpp에서만
// Platform/src/Windows/Win32Window.cpp
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
```

---

## 14. 의존성 및 싱글톤 접근

### 의존성 주입 vs 전역 접근 결정 가이드

새로운 시스템이나 클래스를 만들 때 다음 순서로 판단합니다:
```
┌─────────────────────────────────┐
│ 시스템이 필요한가?              │
└────────────┬────────────────────┘
             │
             ▼
┌─────────────────────────────────┐
│ 1. 디버깅/로깅/어설션인가?      │
│    YES → 싱글톤 전역 접근       │
│    NO  → 다음 단계              │
└────────────┬────────────────────┘
             │
             ▼
┌─────────────────────────────────┐
│ 2. 수명이 명확한 소유자가       │
│    있는가? (예: Window→Input)   │
│    YES → 소유자가 관리          │
│    NO  → 다음 단계              │
└────────────┬────────────────────┘
             │
             ▼
┌─────────────────────────────────┐
│ 3. 상태가 있는가?               │
│    YES → 의존성 주입            │
│    NO  → 순수 함수/정적 함수    │
└─────────────────────────────────┘
```

---

### 판단 기준표

다음 질문에 답하며 판단합니다:

| 질문 | YES → 전역 접근 | NO → 의존성 주입 |
|------|----------------|-----------------|
| 디버깅/로깅/어설션인가? | ✓ | |
| 애플리케이션 전체 수명인가? | ✓ | |
| 모든 곳에서 사용하는가? | ✓ | |
| 단위 테스트가 필요한가? | | ✓ |
| Mock 객체로 대체 가능해야 하는가? | | ✓ |
| 상태가 변경되는가? | | ✓ |
| 여러 인스턴스가 가능한가? | | ✓ |
| 게임 로직인가? | | ✓ |

---

### 실제 프로젝트 패턴

#### 패턴 1: 싱글톤 전역 접근 (허용)

**언제 사용:**
- 디버깅, 로깅, 어설션
- 전역 유틸리티 (메모리 프로파일러 등)
- 애플리케이션 전체 수명
```cpp
// Logger - 어디서든 사용
void SomeFunction()
{
    LOG_INFO("함수 시작");
    LOG_ERROR("에러 발생: %s", error);
}

// Assertion - 디버깅 전용
void Process(Entity* entity)
{
    CORE_ASSERT(entity != nullptr, "Entity는 null일 수 없음");
}
```

**왜 허용?**
- 로깅은 비즈니스 로직이 아님
- 모든 함수에 Logger를 전달하는 것은 비실용적
- 테스트 시 로그 출력 여부는 중요하지 않음

---

#### 패턴 2: 명확한 소유권 (권장)

**언제 사용:**
- 수명이 명확한 소유 관계
- 생성/소멸이 함께 이루어지는 경우
```cpp
// Window가 Input을 소유
class Window
{
private:
    Input mInput;  // Window 수명 = Input 수명
    
public:
    Input& GetInput() { return mInput; }
};

// 사용
void GameLoop(Window& window)
{
    if (window.GetInput().IsKeyPressed(KeyCode::Escape))
    {
        window.Close();
    }
}
```

**장점:**
- 소유권 명확
- 자동 생성/소멸
- 싱글톤 불필요

---

#### 패턴 3: 의존성 주입 (권장)

**언제 사용:**
- 게임 로직, 시스템
- 단위 테스트 필요
- 여러 인스턴스 가능
```cpp
// 생성자 주입
class RenderSystem
{
private:
    Device* mDevice;
    CommandQueue* mQueue;
    
public:
    RenderSystem(Device* device, CommandQueue* queue)
        : mDevice(device)
        , mQueue(queue)
    {
    }
    
    void Render(const Scene& scene)
    {
        mQueue->ExecuteCommands(...);
    }
};

// 사용
int main()
{
    auto device = CreateDevice();
    auto queue = CreateCommandQueue(device.get());
    
    RenderSystem renderSystem(device.get(), queue.get());
    renderSystem.Render(scene);
}
```

**장점:**
- 테스트 가능 (Mock 주입)
- 의존성 명확
- 재사용 가능

---

#### 패턴 4: 순수 함수 (최고)

**언제 사용:**
- 상태 없음
- 입력 → 출력만 수행
```cpp
// Math - 전역 상태 없음
Vector3 result = Add(v1, v2);
Matrix4x4 viewMatrix = CreateLookAt(eye, target, up);

// Utility 함수
std::string FormatString(const char* format, ...);
bool FileExists(const std::string& path);
```

**장점:**
- 테스트 간단
- 부작용 없음
- 스레드 안전

---

### 안티패턴 및 해결책

#### 안티패턴 1: GameObject가 모든 것을 알고 있음
```cpp
// 나쁜 예
class GameObject
{
public:
    void Update()
    {
        // 직접 싱글톤 접근 - 테스트 불가능
        Renderer::GetInstance().DrawDebug(mPosition);
        PhysicsWorld::GetInstance().CheckCollision(mCollider);
        AudioEngine::GetInstance().PlaySound(mSound);
    }
};
```

**문제점:**
- GameObject가 너무 많이 알고 있음
- 테스트 시 모든 싱글톤 초기화 필요
- Mock 불가능

**해결책: System 분리 + 의존성 주입**
```cpp
// 좋은 예
class GameObject
{
public:
    // 데이터만 보유
    Vector3 position;
    Collider* collider;
    AudioClip* sound;
};

// 각 System이 처리
class RenderSystem
{
private:
    Renderer* mRenderer;
    
public:
    RenderSystem(Renderer* renderer) : mRenderer(renderer) {}
    
    void Update(const std::vector<GameObject>& objects)
    {
        for (auto& obj : objects)
        {
            mRenderer->DrawDebug(obj.position);
        }
    }
};

class PhysicsSystem
{
private:
    PhysicsWorld* mWorld;
    
public:
    PhysicsSystem(PhysicsWorld* world) : mWorld(world) {}
    
    void Update(std::vector<GameObject>& objects)
    {
        for (auto& obj : objects)
        {
            mWorld->CheckCollision(obj.collider);
        }
    }
};
```

---

#### 안티패턴 2: 매개변수 지옥 (Parameter Hell)
```cpp
// 나쁜 예: 너무 많은 의존성
void Render(
    Device* device,
    CommandQueue* queue,
    SwapChain* swapChain,
    DescriptorHeap* heap,
    PipelineState* pso,
    RootSignature* rootSig,
    const Scene& scene
)
{
    // ...
}
```

**문제점:**
- 매개변수가 너무 많음
- 호출할 때마다 모두 전달 필요

**해결책 1: 컨텍스트 객체로 묶기**
```cpp
// 좋은 예
struct RenderContext
{
    Device* device;
    CommandQueue* queue;
    SwapChain* swapChain;
    DescriptorHeap* heap;
    PipelineState* pso;
    RootSignature* rootSig;
};

void Render(const RenderContext& ctx, const Scene& scene)
{
    ctx.queue->ExecuteCommands(...);
}
```

**해결책 2: 클래스로 캡슐화**
```cpp
// 더 좋은 예
class Renderer
{
private:
    Device* mDevice;
    CommandQueue* mQueue;
    SwapChain* mSwapChain;
    // ... 기타 의존성
    
public:
    Renderer(Device* device, CommandQueue* queue, ...)
        : mDevice(device), mQueue(queue), ...
    {
    }
    
    void Render(const Scene& scene)
    {
        mQueue->ExecuteCommands(...);
    }
};
```

---

### 실전 의사결정 예시

#### 예시 1: ResourceManager

**질문:**
- 디버깅/로깅? → NO
- 수명 소유자? → NO (애플리케이션 전체)
- 여러 인스턴스? → NO (하나만 필요)
- 상태 변경? → YES (리소스 추가/제거)
- 테스트 필요? → YES (Mock으로 대체)

**결론:** 의존성 주입 사용
```cpp
class ResourceManager
{
public:
    Texture* LoadTexture(const std::string& path);
    void UnloadTexture(const std::string& path);
};

// 사용
class Game
{
private:
    std::unique_ptr<ResourceManager> mResourceManager;
    
public:
    Game()
        : mResourceManager(std::make_unique<ResourceManager>())
    {
    }
};
```

---

#### 예시 2: Logger

**질문:**
- 디버깅/로깅? → YES

**결론:** 싱글톤 전역 접근 허용
```cpp
// 어디서든 사용
LOG_INFO("게임 시작");
LOG_ERROR("리소스 로드 실패: %s", path);
```

---

#### 예시 3: Input

**질문:**
- 디버깅/로깅? → NO
- 수명 소유자? → YES (Window)

**결론:** Window가 소유
```cpp
class Window
{
private:
    Input mInput;
    
public:
    Input& GetInput() { return mInput; }
};
```

---

### 마이그레이션 가이드

**기존 싱글톤을 의존성 주입으로 변경하기:**
```cpp
// Before: 싱글톤
class AudioEngine
{
public:
    static AudioEngine& GetInstance();
    void PlaySound(const std::string& name);
};

// Everywhere:
AudioEngine::GetInstance().PlaySound("explosion");

// After: 의존성 주입
class AudioEngine
{
public:
    void PlaySound(const std::string& name);
};

class Game
{
private:
    std::unique_ptr<AudioEngine> mAudioEngine;
    
public:
    Game()
        : mAudioEngine(std::make_unique<AudioEngine>())
    {
    }
    
    void OnExplosion()
    {
        mAudioEngine->PlaySound("explosion");
    }
};
```

---

### 체크리스트

새로운 시스템을 만들기 전에:

- [ ] 이것이 정말 싱글톤이어야 하는가?
- [ ] 테스트할 필요가 있는가?
- [ ] 여러 인스턴스가 필요할 가능성은?
- [ ] 명확한 소유자가 있는가?
- [ ] 디버깅/로깅 같은 유틸리티인가?

**기본 원칙: 의심스러우면 의존성 주입을 사용하라**

---

## 15. 예외 처리 및 에러 핸들링

### HRESULT 처리
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

### 예외 사용 원칙
- **초기화 실패**: 예외 throw
- **런타임 에러**: 로깅 + 복구 또는 안전 종료
- **게임 로직**: 예외 지양, 반환값 사용

```cpp
// 초기화
void Renderer::Init()
{
    ThrowIfFailed(CreateDevice());
}

// 런타임
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

### noexcept 사용
- 소멸자: 항상 `noexcept`
- 이동 연산자: 가능하면 `noexcept`
- 성능 중요 함수: `noexcept` 고려

---

## 16. 템플릿 사용

### 템플릿 네이밍
```cpp
// 간단한 경우
template<typename T>
class Array { };

// 의미 중요 시
template<typename VertexType, typename IndexType>
class Mesh { };

// C++20 Concepts
template<std::integral T>
T Add(T a, T b);
```

### 정의 위치
- 헤더에 전체 정의 (기본)
- 명시적 인스턴스화 필요 시에만 cpp 분리

---

## 17. 성능 최적화

### 복사 최소화
```cpp
// const reference 반환
const std::vector<Vertex>& GetVertices() const
{
    return mVertices;
}

// 이동 의미론
std::vector<Vertex> ExtractVertices()
{
    return std::move(mVertices);
}
```

### Reserve와 Emplace
```cpp
std::vector<Vertex> vertices;
vertices.reserve(1000);  // 사전 할당
vertices.emplace_back(pos, normal, uv);  // 직접 생성
```

### 인라인 함수
- 간단한 Getter/Setter는 헤더 인라인
- 3줄 이하 간단 로직만

---

## 18. 멀티스레딩

### 동기화 프리미티브
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

### Thread-safe 싱글톤
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

## 19. 안티패턴

### 매크로 남용
```cpp
// 전역 접근 매크로
#define DEVICE Engine::GetInstance().GetDevice()

// 명시적 전달
void Init(Device* device);
```

### 불필요한 스마트 포인터
```cpp
// 소유권 필요 없는데 shared_ptr
void Draw(const std::shared_ptr<Mesh>& mesh);

// 참조로 충분
void Draw(const Mesh& mesh);
```

### 무의미한 주석
```cpp
// 당연한 것 설명
i++;  // i를 1 증가

// Why 설명
i++;  // 다음 프레임 인덱스로 이동
```

## 20. 전처리 지시자 및 매크로

### 전처리 지시자 위치

**규칙: 항상 맨 왼쪽 (들여쓰기 없음)**
```cpp
// 좋은 예: 전처리문은 맨 왼쪽
#if defined(_DEBUG)
if (condition)
{
    DebugFunction();
}
#endif

// 나쁜 예: 전처리문 들여쓰기
    #if defined(_DEBUG)
    if (condition)
    {
        DebugFunction();
    }
    #endif
```

**이유:**
- 전처리는 컴파일 전 단계로 코드 구조와 별개
- 들여쓰면 오히려 혼란스러움
- 대부분의 IDE가 자동으로 맨 왼쪽 정렬

---

### 매크로 작성 규칙

#### 기본 원칙: inline 함수 우선
```cpp
// 권장: inline 함수
inline void ThrowIfFailed(HRESULT hr, const char* msg)
{
    if (FAILED(hr))
    {
        LOG_ERROR("DirectX 12 Error: %s (HRESULT: 0x%08X)", msg, hr);
        throw std::runtime_error(msg);
    }
}

// 사용
ThrowIfFailed(hr, "Failed to create device");
```

**inline 함수의 장점:**
- 타입 안전성
- 디버깅 용이 (스택 추적 가능)
- 네임스페이스 사용 가능
- IDE 자동완성 지원
- 컴파일러 최적화

**매크로가 필요한 경우만 사용:**
```cpp
// __FILE__, __LINE__ 같은 메타 정보 필요
#define LOG_LOCATION(msg) \
    LOG_INFO("[%s:%d] %s", __FILE__, __LINE__, msg)

// 조건부 컴파일
#if defined(_DEBUG)
    #define DEBUG_ONLY(x) x
#else
    #define DEBUG_ONLY(x)
#endif
```

---

### 여러 줄 매크로 작성법

#### 패턴: do-while(0) 사용 (필수)
```cpp
// 권장: do-while(0)로 감싸기
#define MACRO_NAME(param)                              \
    do {                                               \
        Statement1(param);                             \
        Statement2(param);                             \
    } while(0)

// 사용 - 세미콜론 안전
if (condition)
    MACRO_NAME(value);  // 안전
else
    OtherFunction();    // OK
```

**do-while(0)이 없으면 문제 발생:**
```cpp
// 나쁜 예: do-while(0) 없음
#define BAD_MACRO(x) \
    Foo(x); \
    Bar(x)

// 사용 - 버그!
if (condition)
    BAD_MACRO(value);  // Foo만 if 안에, Bar는 밖에!
else
    DoSomething();     // 컴파일 에러!

// 좋은 예: do-while(0) 사용
#define GOOD_MACRO(x)  \
    do {               \
        Foo(x);        \
        Bar(x);        \
    } while(0)

if (condition)
    GOOD_MACRO(value);  // 안전
else
    DoSomething();      // OK
```

---

### 백슬래시 정렬
```cpp
// 좋은 예: 백슬래시 세로로 정렬 (가독성)
#define GRAPHICS_THROW_IF_FAILED(hr, msg)                                    \
    do {                                                                     \
        if (FAILED(hr)) {                                                    \
            LOG_ERROR("DirectX 12 Error: %s (HRESULT: 0x%08X)", msg, hr);   \
            throw std::runtime_error(msg);                                   \
        }                                                                    \
    } while(0)

// 허용: 짧은 매크로는 정렬 안 해도 됨
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define SHORT_MACRO(x) \
    do { \
        Foo(x); \
    } while(0)
```

**정렬 기준:**
- 매크로가 5줄 이상: 백슬래시 정렬 권장
- 매크로가 3줄 이하: 정렬 선택 사항

---

### 조건부 컴파일 패턴

#### 패턴 1: Debug 전용 코드
```cpp
// Debug 빌드에만 포함
#if defined(_DEBUG)
if (enableDebugLayer)
{
    EnableDebugLayer();
    LOG_DEBUG("Debug layer enabled");
}
#endif

// Release 빌드에는 완전히 제거됨
```

#### 패턴 2: Platform 분기
```cpp
// Platform별 구현
#if defined(_WIN32)
    #include <Windows.h>
    #define PLATFORM_WINDOWS 1
#elif defined(__linux__)
    #include <unistd.h>
    #define PLATFORM_LINUX 1
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #define PLATFORM_MACOS 1
#else
    #error "Unsupported platform"
#endif
```

#### 패턴 3: Feature Toggle
```cpp
// Config.h - 프로젝트 설정
#pragma once

// 기능 플래그
#define ENABLE_RAYTRACING 1
#define ENABLE_DLSS 0
#define ENABLE_PROFILING 1

// 사용
#if ENABLE_RAYTRACING
void InitRaytracing() 
{
    LOG_INFO("Raytracing enabled");
}
#endif

#if ENABLE_PROFILING
    #define PROFILE_SCOPE(name) ProfilerScope _scope(name)
#else
    #define PROFILE_SCOPE(name)  // No-op in non-profiling builds
#endif
```

---

### 매크로 네이밍 규칙

**규칙:**
- 항상 대문자 + 언더스코어
- 프로젝트/모듈 접두어 사용 (충돌 방지)
- 짧지만 명확하게
```cpp
// 좋은 예: 접두어 + 대문자
#define DX12_THROW_IF_FAILED(hr, msg) ...
#define CORE_ASSERT(condition, msg) ...
#define GRAPHICS_LOG_INFO(msg) ...

// 나쁜 예
#define throwIfFailed(hr, msg) ...  // ❌ 소문자 (함수와 혼동)
#define ASSERT(x) ...               // ❌ 접두어 없음 (다른 라이브러리와 충돌)
#define T(x) ...                    // ❌ 너무 짧음 (의미 불명확)
```

**프로젝트 접두어:**
- `CORE_`: Core 시스템 매크로
- `GRAPHICS_`: Graphics 시스템 매크로
- `DX12_`: DirectX 12 특화 매크로
- `PLATFORM_`: Platform 레이어 매크로

---

### 매크로 인자 보호

**규칙: 모든 인자를 괄호로 감싸기**
```cpp
// 나쁜 예: 괄호 없음
#define SQUARE(x) x * x

// 사용
int result = SQUARE(1 + 2);  // 예상: 9, 실제: 1 + 2 * 1 + 2 = 5 ❌

// 좋은 예: 괄호로 보호
#define SQUARE(x) ((x) * (x))

// 사용
int result = SQUARE(1 + 2);  // ((1 + 2) * (1 + 2)) = 9 ✅
```

**여러 인자도 동일:**
```cpp
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(x, min, max) (((x) < (min)) ? (min) : (((x) > (max)) ? (max) : (x)))
```

---

### 실전 예시

#### 예시 1: 공통 유틸리티 헤더
```cpp
// Graphics/include/Graphics/DX12/DX12Common.h
#pragma once
#include <d3d12.h>
#include <stdexcept>
#include "Core/Logging/Logger.h"

namespace DX12
{
    // inline 함수 (권장)
    inline void ThrowIfFailed(HRESULT hr, const char* msg)
    {
        if (FAILED(hr))
        {
            LOG_ERROR("DirectX 12 Error: %s (HRESULT: 0x%08X)", msg, hr);
            throw std::runtime_error(msg);
        }
    }
    
    // 매크로 (파일/라인 정보 필요)
    #define DX12_THROW_IF_FAILED_LOC(hr, msg)                                \
        do {                                                                 \
            if (FAILED(hr)) {                                                \
                LOG_ERROR("[%s:%d] DX12 Error: %s (HRESULT: 0x%08X)",       \
                          __FILE__, __LINE__, msg, hr);                      \
                throw std::runtime_error(msg);                               \
            }                                                                \
        } while(0)
}
```

#### 예시 2: Debug 전용 매크로
```cpp
// Core/include/Core/Assert.h
#pragma once
#include "Core/Logging/Logger.h"

#if defined(_DEBUG)
    #define CORE_ASSERT(condition, msg)                                      \
        do {                                                                 \
            if (!(condition)) {                                              \
                LOG_ERROR("[ASSERT FAILED] %s:%d - %s",                      \
                          __FILE__, __LINE__, msg);                          \
                __debugbreak();                                              \
            }                                                                \
        } while(0)
    
    #define CORE_VERIFY(condition, msg) CORE_ASSERT(condition, msg)
#else
    #define CORE_ASSERT(condition, msg)  // No-op in Release
    #define CORE_VERIFY(condition, msg) (condition)  // 조건만 평가
#endif
```

#### 예시 3: 조건부 로깅
```cpp
// Core/include/Core/Logging/LogMacros.h
#pragma once

#if defined(_DEBUG)
    #define LOG_TRACE(format, ...) \
        Logger::GetInstance().Log(LogLevel::Trace, LogCategory::Core, \
                                  format, ##__VA_ARGS__)
    
    #define LOG_DEBUG(format, ...) \
        Logger::GetInstance().Log(LogLevel::Debug, LogCategory::Core, \
                                  format, ##__VA_ARGS__)
#else
    // Release 빌드에서 완전히 제거
    #define LOG_TRACE(format, ...)
    #define LOG_DEBUG(format, ...)
#endif

// 항상 포함
#define LOG_INFO(format, ...) \
    Logger::GetInstance().Log(LogLevel::Info, LogCategory::Core, \
                              format, ##__VA_ARGS__)

#define LOG_ERROR(format, ...) \
    Logger::GetInstance().Log(LogLevel::Error, LogCategory::Core, \
                              format, ##__VA_ARGS__)
```

---

### 안티패턴

#### 안티패턴 1: 함수 같은 매크로
```cpp
// 나쁜 예: 복잡한 로직을 매크로로
#define CALCULATE_TRANSFORM(pos, rot, scale) \
    do { \
        Matrix4x4 translation = CreateTranslation(pos); \
        Matrix4x4 rotation = CreateRotation(rot); \
        Matrix4x4 scaling = CreateScaling(scale); \
        result = Multiply(Multiply(scaling, rotation), translation); \
    } while(0)

// 좋은 예: inline 함수 사용
inline Matrix4x4 CalculateTransform(
    const Vector3& pos,
    const Quaternion& rot,
    const Vector3& scale
)
{
    Matrix4x4 translation = CreateTranslation(pos);
    Matrix4x4 rotation = CreateRotation(rot);
    Matrix4x4 scaling = CreateScaling(scale);
    return Multiply(Multiply(scaling, rotation), translation);
}
```

#### 안티패턴 2: 부작용 있는 인자
```cpp
// 나쁜 예: 부작용 발생 가능
#define MAX(a, b) ((a) > (b) ? (a) : (b))

int x = 5;
int result = MAX(x++, 10);  // x가 두 번 증가될 수 있음!

// 좋은 예: inline 함수 (부작용 안전)
template<typename T>
inline T Max(T a, T b)
{
    return a > b ? a : b;
}

int x = 5;
int result = Max(x++, 10);  // x는 한 번만 증가
```

#### 안티패턴 3: 매크로 남용
```cpp
// 나쁜 예: 모든 것을 매크로로
#define DEVICE mDevice
#define QUEUE mCommandQueue
#define SWAP_CHAIN mSwapChain

void Render()
{
    DEVICE->Method();  // 혼란스러움
    QUEUE->Execute();
    SWAP_CHAIN->Present();
}

// 좋은 예: 명확한 변수명 사용
void Render()
{
    mDevice->Method();
    mCommandQueue->Execute();
    mSwapChain->Present();
}
```

---

### 체크리스트

매크로를 작성하기 전에:

- [ ] inline 함수로 대체 가능한가?
- [ ] 정말 매크로가 필요한가? (`__FILE__`, `__LINE__` 필요?)
- [ ] do-while(0)로 감쌌는가?
- [ ] 모든 인자를 괄호로 보호했는가?
- [ ] 대문자 + 접두어 네이밍을 사용했는가?
- [ ] 부작용이 없는가?
- [ ] 백슬래시가 정렬되어 있는가? (5줄 이상 시)

---

### 요약

**핵심 원칙:**

| 항목 | 규칙 |
|------|------|
| **전처리 위치** | 항상 맨 왼쪽 (들여쓰기 X) |
| **우선순위** | inline 함수 > 매크로 |
| **여러 줄 매크로** | do-while(0) 필수 |
| **인자 보호** | 모든 인자 괄호로 감싸기 |
| **네이밍** | 대문자 + 접두어 |
| **정렬** | 5줄 이상 시 백슬래시 정렬 |

**기억할 것:**
> "매크로는 최후의 수단. 가능하면 inline 함수를 사용하라."

---

### 향후 계획

DirectX 12 렌더링 코드가 증가하면 다음 매크로 패턴 추가 예정:
- HRESULT 에러 핸들링 (DX12_THROW_IF_FAILED)
- PIX 이벤트 마킹 (DX12_BEGIN_EVENT)
- 리소스 네이밍 (DX12_SET_NAME)
```

---

## 관련 섹션

- [섹션 5: 상수 정의](#5-상수-정의) - constexpr 사용
- [섹션 15: 예외 처리 및 에러 핸들링](#15-예외-처리-및-에러-핸들링) - ThrowIfFailed 패턴
- [섹션 7: 주석 작성 규칙](#7-주석-작성-규칙) - 매크로 문서화