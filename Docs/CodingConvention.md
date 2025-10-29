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
21. [기본 타입 사용 규칙](#21-기본-타입-사용-규칙)

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

### 변수 초기화

**기본 원칙**: 단순 값은 `=`, 명시적 생성이 필요할 때만 `{}`

| 상황 | 사용 문법 | 예시 |
|------|----------|------|
| **단순 값** | `=` | `int x = 0;` |
| **집합 초기화** | `= {}` | `std::vector<int> v = {1,2,3};` |
| **explicit 생성자** | `{}` | `Allocator alloc{1024};` |
| **복합 객체 배열** | `= {}` + 내부 `{}` | `std::array<T, 3> arr = {T{}, T{}, T{}};` |
```cpp
// 단순 값: = 사용
int count = 0;
float pi = 3.14f;
std::string name = "Engine";

// 멤버 변수
class Renderer
{
private:
    UINT mFrameIndex = 0;
    float mDeltaTime = 0.0f;
    bool mInitialized = false;
};

// 집합 초기화: = {} 조합
std::vector<int> values = {1, 2, 3};
std::array<float, 3> position = {0.0f, 1.0f, 0.0f};

// explicit 생성자: {} 사용
class Allocator
{
public:
    explicit Allocator(size_t size);
};
Allocator allocator{1024};  // {} 필요

// 복합 객체 배열
std::array<CD3DX12_DESCRIPTOR_RANGE, 3> descriptorRanges = {
    CD3DX12_DESCRIPTOR_RANGE{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0},
    CD3DX12_DESCRIPTOR_RANGE{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0},
    CD3DX12_DESCRIPTOR_RANGE{D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0},
};
```

**Most Vexing Parse 주의**:
```cpp
// ❌ 함수 선언으로 해석됨!
Widget widget();

// ✅ 해결책
Widget widget = Widget();  // 또는
Widget widget{};
```

---

## 4. 메모리 관리

### 기본 원칙
- 로우 포인터 최소화, 참조(`&`) 적극 사용
- 읽기 전용 인자는 `const T&` 기본
- 스마트 포인터는 `const&`로 전달 (복사 비용 방지)
- 포인터는 null 가능성 또는 명시적 소유권 전달 시에만 사용

---

### 스마트 포인터 선택

| 타입 | 용도 | 소유권 |
|------|------|--------|
| `std::unique_ptr` | 단독 소유 | 독점 |
| `std::shared_ptr` | 공유 소유 | 공유 |
| `ComPtr` | COM/DirectX 객체 | COM 규칙 |
```cpp
// 기본: unique_ptr
std::unique_ptr<Texture> texture = LoadTexture("texture.png");

// 공유 필요: shared_ptr
std::shared_ptr<Material> material = std::make_shared<Material>();

// DirectX/COM: ComPtr
ComPtr<ID3D12Device> device;
```

---

### ComPtr 사용법
```cpp
#include <wrl/client.h>

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;
```

#### 주요 메서드

| 메서드 | 용도 | 사용 예시 |
|--------|------|----------|
| `Get()` | 로우 포인터 얻기 (관찰) | `device.Get()` |
| `GetAddressOf()` | 이중 포인터 (할당) | `IID_PPV_ARGS(&device)` |
| `Reset()` | 참조 해제 | `device.Reset()` |
```cpp
// 1. 생성
ComPtr<ID3D12Device> device;
D3D12CreateDevice(adapter, level, IID_PPV_ARGS(&device));

// 2. Get() - 관찰만
ID3D12Device* rawPtr = device.Get();
rawPtr->CreateCommandQueue(&desc, IID_PPV_ARGS(&queue));

// 3. 함수 전달 (소유권 유지)
void DoSomething(ID3D12Device* device);
DoSomething(device.Get());

// 4. 재할당 시 Reset() 필요
device.Reset();
D3D12CreateDevice(adapter2, level, IID_PPV_ARGS(&device));

// 5. nullptr 체크
if (device)
{
    device->CreateCommandQueue(...);
}
```

#### 함수 파라미터 전달
```cpp
// 관찰만 (권장)
void RenderMesh(ID3D12Device* device, const Mesh& mesh)
{
    device->CreateCommandList(...);
}
RenderMesh(mDevice.Get(), mesh);

// 소유권 공유
void StoreDevice(const ComPtr<ID3D12Device>& device)
{
    mDevice = device;  // 참조 카운트 증가
}

// 출력 파라미터
bool CreateDevice(ID3D12Device** outDevice)
{
    ComPtr<ID3D12Device> device;
    // ... 생성 로직
    *outDevice = device.Detach();
    return true;
}
```

#### 주의사항
```cpp
// ❌ 나쁜 예: Get()으로 받은 포인터 저장
class BadExample
{
private:
    ID3D12Device* mDevice;  // 위험!
public:
    void Init(const ComPtr<ID3D12Device>& device)
    {
        mDevice = device.Get();  // 수명 관리 안 됨
    }
};

// ✅ 좋은 예: ComPtr로 저장
class GoodExample
{
private:
    ComPtr<ID3D12Device> mDevice;  // 안전
public:
    void Init(const ComPtr<ID3D12Device>& device)
    {
        mDevice = device;  // 참조 카운트 증가
    }
};
```

---

### 스마트 포인터 파라미터 전달 규칙

| 의도 | std::unique_ptr | std::shared_ptr | ComPtr |
|------|----------------|----------------|--------|
| **관찰만** | `T*` 또는 `T&` | `T*` 또는 `T&` | `T*` (Get()) |
| **소유권 공유** | N/A | `shared_ptr<T>` | `const ComPtr<T>&` |
| **소유권 이전** | `unique_ptr<T>` | N/A | `ComPtr<T>` (이동) |

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

### 기본 원칙
1. **Self-documenting code 우선** - 주석보다 명확한 코드가 최선
2. **최소주의** - 코드로 표현 불가능한 것만 주석으로
3. **What보다 Why** - 무엇을 하는지보다 왜 하는지 설명
4. **한글 작성** - 모든 주석은 한글로 작성 (업계 표준 용어는 영어 유지)

---

### Public API 문서화 - Doxygen 스타일

**헤더 파일(.h)의 public 인터페이스에만 적용**
```cpp
/**
 * @brief 프레임 임시 데이터를 위한 선형 할당자
 * 
 * 상세 설명이 필요한 경우 여기에 작성
 * 
 * @param size 할당할 바이트 크기
 * @param alignment 정렬 요구사항 (기본: 16)
 * @return 할당된 메모리 포인터, 실패 시 nullptr
 * 
 * @note 스레드 안전하지 않음
 * @warning 할당된 메모리는 Reset() 전까지 유효
 */
void* Allocate(size_t size, size_t alignment = 16);
```

**필수 태그**: `@brief`, `@param`, `@return`  
**선택 태그**: `@note`, `@warning`, `@throws`

**예외**: 다음의 경우 `@param`, `@return` 생략 가능
- 매개변수/반환값 이름이 목적을 완전히 설명하는 경우
- Getter/Setter 같은 자명한 함수

---

### 문서화가 불필요한 경우

다음은 Doxygen 주석을 생략합니다:
```cpp
// 1. 자명한 Getter/Setter
size_t GetCapacity() const { return mCapacity; }
void SetEnabled(bool enabled) { mEnabled = enabled; }

// 2. Override 함수 (기본 클래스에 문서화되어 있는 경우)
void Update() override;

// 3. private 헬퍼 함수 중 이름이 명확한 경우
private:
    void SortByPriority();
    void ClearExpiredEntries();
```

---

### 구현 파일(.cpp) 주석 - // 스타일

**복잡한 로직이나 의도 설명에만 사용**
```cpp
void RenderSystem::Update()
{
    // GPU 펜스 값 증가로 프레임 동기화
    mFenceValue++;
    
    // CRITICAL: 이전 프레임 완료 대기 (병목 가능)
    WaitForPreviousFrame();
}
```

**좋은 주석**: 비직관적인 알고리즘, 최적화 이유, 제약사항  
**나쁜 주석**: 코드를 그대로 반복하는 설명
```cpp
// ❌ 나쁜 예: 코드 반복
i++;  // i를 1 증가시킨다

// ✅ 좋은 예: 의도 설명
i++;  // 다음 프레임 인덱스로 이동
```

---

### 복잡한 알고리즘 설명
```cpp
// Separating Axis Theorem (SAT)를 이용한 OBB 충돌 검사
// 참고: Real-Time Collision Detection (Christer Ericson) Chapter 4.4
bool CheckOBBCollision(const OBB& a, const OBB& b)
{
    // 15개 분리축 검사 (면 법선 6개 + 엣지 교차 9개)
    for (int i = 0; i < 15; ++i)
    {
        ...
    }
}
```

---

### TODO 태그
```cpp
// TODO: [P1] 다음 스프린트 - 멀티스레드 렌더링 지원
// FIXME: [P0] 즉시 수정 필요 - 메모리 누수 발생 (Issue #42)
// OPTIMIZE: [P2] 성능 개선 - 현재 100ms, 목표 16ms
// HACK: [P2] 임시 해결책 - DX12 버그 우회
// NOTE: 이 함수는 메인 스레드에서만 호출 가능
```

**우선순위**: P0(긴급) > P1(높음) > P2(중간) > P3(낮음)

---

### 디버깅은 로깅 시스템 사용
```cpp
// ❌ 나쁜 예: 주석으로 디버깅
void Update()
{
    // mOffset = 1024
    // mCapacity = 2048
    mOffset += size;
}

// ✅ 좋은 예: 로깅 시스템
void Update()
{
    LOG_TRACE("Allocator: Offset=%zu, Capacity=%zu", mOffset, mCapacity);
    mOffset += size;
}
```

---

### 요약

| 위치 | 스타일 | 사용 시점 |
|------|--------|----------|
| **헤더 public** | `/** */` Doxygen | Public API만 |
| **헤더 private** | 주석 생략 | 이름이 명확하면 불필요 |
| **구현 파일** | `//` 일반 주석 | 복잡한 로직, Why 설명 |
| **TODO 태그** | `// TODO:` | 향후 작업 표시 |

---

## 8. 네임스페이스

### 기본 원칙

**프로젝트 네임스페이스**
- 모듈별 네임스페이스 사용 (Core, Graphics, Physics 등)
- 네임스페이스 중첩 최대 2단계까지 권장
- 예: `Core::Memory`, `Graphics::D3D12`, `Physics::Collision`

**헤더 파일 규칙 (엄격)**
```cpp
// 헤더 파일 (.h)

// ❌ 금지: using namespace는 절대 사용 불가
// using namespace std;
// using namespace Graphics;

// ✅ 허용: 명시적 타입 사용
std::vector<int> GetData() const;
std::string GetName() const;

// ✅ 허용: 타입 별칭
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;
```

**구현 파일 규칙 (유연)**
```cpp
// 구현 파일 (.cpp)

// ✅ 허용: using namespace
using namespace std;
using namespace Graphics;

vector<int> Device::GetData() const
{
    return mData;
}
```

---

### 네임스페이스 구조

#### 모듈별 네임스페이스(예시)
```cpp
// Core 모듈
namespace Core
{
    class Types { };
    class Time { };
}

namespace Core::Memory
{
    class LinearAllocator { };
    class PoolAllocator { };
}

namespace Core::Logging
{
    class Logger { };
    enum class LogLevel { };
}

// Graphics 모듈
namespace Graphics
{
    class Device { };
    class SwapChain { };
}

namespace Graphics::D3D12
{
    class CommandQueue { };
    class DescriptorHeap { };
}

// Physics 모듈
namespace Physics
{
    class RigidBody { };
}

namespace Physics::Collision
{
    class CollisionDetector { };
    struct AABB { };
}
```

#### 중첩 제한
```cpp
// ✅ 좋은 예: 2단계 중첩
namespace Graphics::D3D12
{
    class Device { };
}

// ⚠️ 피해야 할 예: 과도한 중첩
namespace Engine::Graphics::D3D12::Impl::Details
{
    // 너무 깊음
}

// ✅ 해결책: 의미 있는 2단계로 재구성
namespace Graphics::Internal
{
    // 내부 구현
}
```

---

### 헤더 파일에서 네임스페이스 사용

```cpp
// Device.h
#pragma once
#include <vector>
#include <string>

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

namespace Graphics
{
    class Device
    {
    public:
        // 명시적 std:: 사용
        std::vector<std::string> GetAdapterNames() const;
        std::shared_ptr<CommandQueue> CreateCommandQueue();
        
        // ComPtr 별칭 사용
        ComPtr<ID3D12Device> GetD3D12Device() const;
        
    private:
        std::vector<std::string> mAdapterNames;
        ComPtr<ID3D12Device> mDevice;
    };
    
} // namespace Graphics
```

---

### 구현 파일에서 네임스페이스 사용

```cpp
// Device.cpp
#include "Graphics/Device.h"

// .cpp에서는 자유롭게 using 사용
using namespace std;

namespace Graphics
{
    vector<string> Device::GetAdapterNames() const
    {
        vector<string> names;
        for (const auto& name : mAdapterNames)
        {
            names.push_back(name);
        }
        return names;
    }
    
    ComPtr<ID3D12Device> Device::GetD3D12Device() const
    {
        return mDevice;
    }
    
} // namespace Graphics
```

---

### 왜 헤더에서 using namespace를 금지하는가?

```cpp
// ❌ 나쁜 예: 헤더에서 using namespace
// Device.h
#pragma once
using namespace std;

namespace Graphics
{
    class Device
    {
    public:
        vector<int> GetData() const;  // std::인지 불명확
    };
}

// 다른 파일에서 include 시 의도치 않게 std 전체가 노출됨
#include "Graphics/Device.h"
vector<int> data;  // 혼란 발생
```

**문제점**:
- 네임스페이스 오염
- 이름 충돌 위험 (std::min vs DirectX::min)
- 연쇄 전파 (include chain을 따라 확산)

---

### 익명 네임스페이스 (구현 파일 전용)

```cpp
// Device.cpp
#include "Graphics/Device.h"

using namespace std;

namespace Graphics
{
    namespace  // 익명 네임스페이스 - 파일 내부에서만 사용
    {
        constexpr uint32_t MAX_ADAPTERS = 8;
        
        bool IsHardwareAdapter(IDXGIAdapter1* adapter)
        {
            // ...
        }
    }
    
    bool Device::Initialize()
    {
        // 익명 네임스페이스의 함수 사용
        if (IsHardwareAdapter(mAdapter.Get()))
        {
            // ...
        }
        return true;
    }
    
} // namespace Graphics
```

---

### 요약

| 위치 | using namespace | 명시적 타입 | 타입 별칭 | 네임스페이스 중첩 |
|------|----------------|------------|----------|------------------|
| **헤더 파일 (.h)** | ❌ 금지 | ✅ 필수 | ✅ 허용 | 최대 2단계 권장 |
| **구현 파일 (.cpp)** | ✅ 허용 | ✅ 선택 | ✅ 허용 | 자유 |
| **익명 네임스페이스** | - | - | - | .cpp 전용 |

**원칙**: 
- 헤더는 남을 위해 명시적으로
- 구현은 나를 위해 편하게
- 모듈별로 네임스페이스 구분하여 코드 구조 명확화
  
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
4. 주요 API 함수 (수명 주기 순서: Initialize → Update → Render → Shutdown)
5. Getter / Setter
6. 연산자 오버로딩

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
    
    // 4. 주요 API (수명 주기 순서)
    bool Initialize();
    void Update(float deltaTime);
    void Render();
    void Shutdown();
    
    // 5. Getter/Setter
    int GetSize() const { return mSize; }
    void SetSize(int size) { mSize = size; }
    
    // 6. 연산자
    bool operator==(const ClassName& other) const;

protected:
    // 파생 클래스용
    void ProtectedHelper();
    int mProtectedValue = 0;

private:
    // 내부 헬퍼
    void InitializeInternal();
    
    // 멤버 변수 (맨 아래)
    int mSize = 0;
    State mState = State::Idle;
    std::vector<ResourcePtr> mResources;
};
```

---

### 멤버 변수 정렬

**우선순위**: 크기가 큰 것부터 → 자주 함께 사용되는 것끼리 그룹화
```cpp
class Example
{
private:
    // DirectX 객체 (큰 ComPtr)
    ComPtr<ID3D12Device> mDevice;
    ComPtr<ID3D12CommandQueue> mQueue;
    
    // STL 컨테이너
    std::vector<Resource> mResources;
    std::string mName;
    
    // 기본 타입
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

### 요약

**기본 원칙**:
- public이 먼저 (사용자가 먼저 볼 것)
- 수명 주기 순서대로 (생성 → 사용 → 소멸)
- 멤버 변수는 마지막 (구현 세부사항)

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

### 기본 원칙

다음 순서로 판단합니다:

1. **디버깅/로깅/어설션** → 싱글톤 전역 접근 허용
2. **명확한 소유자 존재** → 소유자가 관리
3. **게임 로직/시스템** → 의존성 주입
4. **상태 없음** → 순수 함수/정적 함수

---

### 패턴 1: 싱글톤 전역 접근 (허용)
```cpp
// 디버깅, 로깅, 어설션
void SomeFunction()
{
    LOG_INFO("함수 시작");
    LOG_ERROR("에러 발생: %s", error);
    CORE_ASSERT(entity != nullptr, "Entity는 null일 수 없음");
}
```

**허용 이유**: 로깅은 비즈니스 로직이 아니며, 모든 함수에 Logger를 전달하는 것은 비실용적

---

### 패턴 2: 명확한 소유권 (권장)
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

---

### 패턴 3: 의존성 주입 (권장)
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

**장점**: 테스트 가능 (Mock 주입), 의존성 명확, 재사용 가능

---

### 패턴 4: 순수 함수 (최고)
```cpp
// 상태 없음, 입력 → 출력만
Vector3 result = Add(v1, v2);
Matrix4x4 viewMatrix = CreateLookAt(eye, target, up);
bool FileExists(const std::string& path);
```

---

### 매개변수 지옥 해결
```cpp
// ❌ 나쁜 예: 매개변수가 너무 많음
void Render(
    Device* device,
    CommandQueue* queue,
    SwapChain* swapChain,
    DescriptorHeap* heap,
    PipelineState* pso,
    RootSignature* rootSig,
    const Scene& scene
);

// ✅ 해결책: 컨텍스트 객체
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

---

### 요약

**기본 원칙**: 의심스러우면 의존성 주입을 사용하라

| 상황 | 방법 |
|------|------|
| 디버깅/로깅 | 싱글톤 허용 |
| 명확한 소유 관계 | 소유자가 관리 |
| 게임 로직/시스템 | 의존성 주입 |
| 상태 없음 | 순수 함수 |

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

### 헤더 가드
- `#pragma once` 사용 (간결하고 에러 방지)
- `#ifndef` 가드 지양 (레거시)

```cpp
// 권장
#pragma once

// 지양
#ifndef MY_HEADER_H
#define MY_HEADER_H
// ...
#endif
```

---

### 매크로 네이밍
- 함수형 매크로: `ALL_CAPS` with underscore
  - 예: `LOG_INFO`, `CORE_ASSERT`, `CHECK_HRESULT`
- 조건부 컴파일: `ALL_CAPS`
  - 예: `NDEBUG`, `_DEBUG`, `USE_SIMD`

---

### 매크로 사용 지침
- `#define` 상수 금지 → `constexpr` 사용
- 디버그 전용 매크로는 Release에서 `((void)0)`으로 치환
- 여러 줄 매크로는 `\` 로 정렬

```cpp
// 좋은 예: 여러 줄 매크로 정렬
#define LOG_ERROR(format, ...) \
    Core::Logging::Logger::GetInstance().Log( \
        Core::Logging::LogLevel::Error, \
        Core::Logging::LogCategory::Core, \
        Core::Logging::FormatLog(format, ##__VA_ARGS__), \
        Core::Logging::GetFileName(__FILE__), __LINE__)

// Release 최적화
#ifdef NDEBUG
#define LOG_TRACE(format, ...) ((void)0)
#define LOG_DEBUG(format, ...) ((void)0)
#endif
```

---

## 21. 기본 타입 사용 규칙

### 타입 정의 (Core::Types.h)

프로젝트는 플랫폼 독립성과 일관성을 위해 `Types.h`에서 정의된 타입을 사용합니다.

#### 정수 타입
```cpp
// 부호 있는 정수
Core::int8   // -128 ~ 127
Core::int16  // -32,768 ~ 32,767
Core::int32  // -2,147,483,648 ~ 2,147,483,647
Core::int64  // 64비트 부호 있는 정수

// 부호 없는 정수
Core::uint8   // 0 ~ 255
Core::uint16  // 0 ~ 65,535
Core::uint32  // 0 ~ 4,294,967,295
Core::uint64  // 64비트 부호 없는 정수
```

#### 부동소수점 타입
```cpp
Core::float32  // 32비트 부동소수점 (IEEE 754)
Core::float64  // 64비트 부동소수점 (IEEE 754)
```

#### 메모리 및 크기 타입
```cpp
Core::size_t  // 메모리 크기, 배열 크기, 오프셋 (플랫폼 종속)
Core::byte    // 원시 바이트 데이터 (숫자 연산 X)
```

---

### 사용 원칙

#### 1. 엔진 내부 코드
**모든 엔진 코드는 `Core::` 타입 사용**
```cpp
// ✅ 좋은 예
Core::uint32 frameIndex = 0;
Core::float32 deltaTime = 0.016f;
Core::int32 vertexCount = 1024;

// ❌ 나쁜 예
int frameIndex = 0;           // 크기 플랫폼 종속
unsigned int count = 0;       // 가독성 떨어짐
float deltaTime = 0.016f;     // float32인지 불명확
```

#### 2. API 경계 (External API Boundary)
**외부 API와의 경계에서는 `static_cast<타입>()` 사용**

외부 API(Win32, DirectX 등)는 고유 타입을 사용하므로, API 호출 시점에 명시적 변환이 필요합니다.
```cpp
// DirectX 12 API 예시
void DX12SwapChain::Initialize(Core::uint32 width, Core::uint32 height)
{
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    
    // API 경계에서 static_cast<타입>()로 명시적 변환
    desc.Width = static_cast<UINT>(width);
    desc.Height = static_cast<UINT>(height);
    desc.BufferCount = static_cast<UINT>(FRAME_BUFFER_COUNT);
}

// Win32 API 예시
void Window::Create(Core::int32 x, Core::int32 y, Core::uint32 width, Core::uint32 height)
{
    mHwnd = CreateWindowEx(
        0, mClassName, mTitle, WS_OVERLAPPEDWINDOW,
        static_cast<int>(x),
        static_cast<int>(y),
        static_cast<int>(width),
        static_cast<int>(height),
        nullptr, nullptr, mInstance, this
    );
}
```

#### 3. 상수 및 리터럴
```cpp
// 정수 상수
constexpr Core::uint32 MAX_FRAME_COUNT = 3;
constexpr Core::int32 DEFAULT_WIDTH = 1280;

// 부동소수점 상수 (접미사 명시)
constexpr Core::float32 PI = 3.14159f;          // float32는 f
constexpr Core::float64 EPSILON = 1e-10;        // float64는 접미사 없음

// 큰 상수는 밑줄로 구분
constexpr Core::uint64 MAX_MEMORY = 1'073'741'824;  // 1GB
```

#### 4. 루프 카운터
```cpp
// 배열 크기나 인덱스는 size_t
for (Core::size_t i = 0; i < vertices.size(); ++i)
{
    ProcessVertex(vertices[i]);
}

// 명확한 범위가 있는 경우 uint32
for (Core::uint32 frameIndex = 0; frameIndex < MAX_FRAME_COUNT; ++frameIndex)
{
    ResetFrameResources(frameIndex);
}
```

---

### 타입 선택 가이드

| 용도 | 타입 | 예시 |
|------|------|------|
| **일반 카운터/인덱스** | `Core::uint32` | `Core::uint32 triangleCount = 0;` |
| **메모리 크기/배열 인덱스** | `Core::size_t` | `Core::size_t bufferSize = 1024;` |
| **플래그/비트마스크** | `Core::uint32/uint64` | `Core::uint32 flags = FLAG_A \| FLAG_B;` |
| **좌표/위치 (부호 필요)** | `Core::int32` | `Core::int32 mouseX = -100;` |
| **수학 연산** | `Core::float32` | `Core::float32 deltaTime = 0.016f;` |
| **고정밀 계산** | `Core::float64` | `Core::float64 preciseTime = 0.0;` |

---

### 금지 사항
```cpp
// ❌ 표준 타입 직접 사용 금지
int count;           // Core::int32 사용
unsigned int flags;  // Core::uint32 사용
float time;          // Core::float32 사용
long long value;     // Core::int64 사용

// ❌ 플랫폼별 타입 금지 (API 경계 제외)
DWORD frameIndex;    // Core::uint32 + static_cast<타입>() 사용
UINT width;          // Core::uint32 + static_cast<타입>() 사용
```

---

### 종합 예시
```cpp
class DX12VertexBuffer
{
public:
    void Initialize(
        ID3D12Device* device,
        const void* vertexData,
        Core::size_t vertexSize,
        Core::uint32 vertexCount
    )
    {
        // 내부 연산은 Core 타입
        Core::size_t bufferSize = vertexSize * vertexCount;
        
        // API 호출 시 static_cast<타입>()
        D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(
            static_cast<UINT64>(bufferSize)
        );
        
        mVertexBufferView.SizeInBytes = static_cast<UINT>(bufferSize);
        mVertexBufferView.StrideInBytes = static_cast<UINT>(vertexSize);
        
        // 내부 저장은 Core 타입
        mVertexCount = vertexCount;
    }

private:
    Core::uint32 mVertexCount = 0;
};
```

---

### 요약

| 상황 | 사용 방법 | 예시 |
|------|----------|------|
| **엔진 내부** | `Core::` 타입 | `Core::uint32 count` |
| **API 경계** | `static_cast<타입>()` | `static_cast<UINT>(count)` |
| **상수 정의** | `constexpr Core::` | `constexpr Core::uint32 MAX = 3` |
| **표준 타입** | **사용 금지** | ~~`int`, `unsigned`, `float`~~ |

---

**최종 업데이트**: 2025-10-29