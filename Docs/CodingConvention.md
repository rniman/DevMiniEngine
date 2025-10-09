# DevMiniEngine Coding Convention

**[한국어](./CodingConvention.ko.md)** | **English**

## 📁 1. File and Directory Structure

- Use `.h`, `.cpp` extensions
- `pch.h`, `pch.cpp` → Precompiled headers
- Organize directories by system/module (e.g., `Renderer/`, `Core/`, `Resource/`)

---

## 🧱 2. Naming Conventions

### ✅ Classes
- `PascalCase`

### ✅ Functions
- `PascalCase`
- **Always start with a verb**
- `bool` returning functions use meaningful prefixes: `Is`, `Has`, `Can`, `Should`

### ✅ Variables
- Local variables: `camelCase`
- Member variables: `m` prefix + `PascalCase`
  - Example: `mFrameIndex`, `mDevice`
- Static member variables: `s` prefix + `PascalCase`

### ✅ Global Variables
- `g` prefix + `PascalCase`
  - Example: `gRenderer`, `gInputSystem`

### ✅ Struct Members
- `camelCase`, no prefix

### ✅ Enumerations
```cpp
enum class ResourceState
{
    Common,
    RenderTarget,
    CopyDest
};
```
- Enum class name: `PascalCase`
- Enum members: `PascalCase`
- Do not repeat enum name (type safety is guaranteed)

---

## 🎨 3. Code Style

### ✅ Basic Style
- Always use braces `{}` (even for single-line if statements)
- Attach pointer to type (`ID3D12Device* device`)
- Use `nullptr`, never `NULL`
- Use `auto` only when type is obvious
- Avoid C-style arrays, prefer STL containers
  - Fixed size: `std::array`
  - Dynamic size: `std::vector`

### ✅ Variable Initialization Rules (Simplified)

**Basic Principle: Prefer `=` initialization**

```cpp
// ✅ Default style
int count = 0;
float pi = 3.14f;
auto result = Calculate();

// ✅ Container initialization
std::vector<int> values = {1, 2, 3};
std::array<float, 3> pos = {0.0f, 1.0f, 0.0f};

// ✅ Member variables (in class definition)
class Renderer
{
private:
    UINT mFrameIndex = 0;
    float mDeltaTime = 0.0f;
};
```

**Exception: Use `{}` directly only for:**
1. Aggregate initialization for structs
2. Explicit constructor calls

```cpp
// Struct aggregate initialization
Vertex vertex{position, normal, uv};

// Explicit constructor
std::vector<int> sizes(10);  // 10 elements, use parentheses
```

> 📌 Narrowing prevention is handled by compiler warnings (`/W4` or `/Wall`)

### ✅ Complex Object Initialization Style

- Line breaks recommended for multiple or complex items

```cpp
// ✅ Multiple items
std::array<CD3DX12_DESCRIPTOR_RANGE, 3> descriptorRanges = {
    CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0),
    CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0),
    CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0),
};
```

> 📌 Trailing comma (,) recommended after last item (for version control convenience)

---

## 🔐 4. Memory Management

### ✅ Basic Principles
- Minimize raw pointers, use references (`&`) actively
- Read-only parameters default to `const T&`
- Pass smart pointers as `const&` (avoid copy cost)
- Use pointers only for null possibility or explicit ownership transfer

### ✅ Recommended Smart Pointers
- `std::unique_ptr` (default choice)
- `std::shared_ptr` (when sharing needed)
- `Microsoft::WRL::ComPtr` (for DirectX COM objects)

```cpp
using Microsoft::WRL::ComPtr;
ComPtr<ID3D12Device> mDevice;
```

### ✅ Smart Pointer Parameter Passing

| Intent | Signature | Use Case |
|--------|-----------|----------|
| Observe only (no ownership) | `T*` or `T&` | **Most common**, when object existence is guaranteed |
| Store internally (share ownership) | `shared_ptr<T>` | When reference count increase needed |
| Transfer ownership | `unique_ptr<T>` | Transfer exclusive ownership |
| Optional observation | `T*` | When null is possible |

```cpp
// ✅ Read-only access
void Draw(const Mesh& mesh);

// ✅ Share ownership
void SetTexture(std::shared_ptr<Texture> texture)
{
    mTexture = std::move(texture);
}

// ✅ Transfer ownership
std::unique_ptr<Mesh> CreateMesh()
{
    return std::make_unique<Mesh>();
}
```

---

## 📌 5. Constant Definitions

### ✅ Basic Principles
- Prohibit `#define`
- Compile-time constants: `constexpr` (globals use **ALL_CAPS**)
- Runtime constants: `const`

```cpp
constexpr int MAX_FRAME_COUNT = 3;
constexpr float PI = 3.14159f;
```

### ✅ Exceptions
- External API macros allowed (`MAX_PATH`, `SUCCEEDED(hr)`, etc.)
- **Do not use `enum` for constant definitions**

---

## 🔍 6. const Usage Principles

- Read-only parameters: `const T&`
- Read-only member functions: `...() const`
- Immutable local variables: `const` or `constexpr`
- Clearly distinguish pointer target/self immutability

---

## 🧭 7. Namespaces

- `using namespace std;` allowed
- `using Microsoft::WRL::ComPtr;` declaration allowed

---

## 🧩 8. Class Member Access

### ✅ Basic Principles
- No direct access to `private` members
- Use `SetX()`, `GetX()` accessors/mutators

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

### ✅ Smart Pointer Get() Allowed
```cpp
ID3D12CommandQueue* GetCmdQueue() const { return mCommandQueue.Get(); }
```

---

## 🧾 9. Function Parameter Alignment

### ✅ Line Break Criteria
- Recommended for 4+ parameters
- Even with ≤3 parameters if complex types or comments needed

```cpp
auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
    backBuffer.Get(),
    D3D12_RESOURCE_STATE_RENDER_TARGET,
    D3D12_RESOURCE_STATE_PRESENT
);
```

### ✅ Function Definition Line Breaks
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

## 🧱 10. Class Organization

### ✅ Access Specifier Order
```cpp
public:
protected:
private:
```

### ✅ Order Within Each Block

**public / protected:**
1. Constructors, destructors
2. Static creation functions (`Create()`, `GetInstance()`)
3. Main API functions (`Init()`, `Render()`, `Update()`)
4. Getters / Setters

**private:**
1. Internal utility functions
2. Member variables (at the bottom)

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

## 🛡️ 11. Function Access Levels

### ✅ Basic Principles
- External interface: `public`
- Internal implementation only: `private`
- For derived classes: `protected`

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

## 🧩 12. Header File Includes

### ✅ Basic Principles
- Prefer forward declarations in headers
- Include only when implementation needed

```cpp
// ✅ Forward declaration
class Device;
class CommandQueue;

// Include only when necessary
#include <d3d12.h>
```

### ✅ When Include is Necessary
- Class size/layout information needed
- Inheritance relationship definition
- Template usage
- Inline function implementation

---

## 🧭 13. Dependencies and Singleton Access

| Layer | Global Access | Description |
|-------|--------------|-------------|
| **Engine Root / App** | ✅ Allowed | Application initialization layer |
| **Manager** | ⚠️ Limited | Explicit passing ideal, global access temporary |
| **Component / Object** | ❌ Avoid | Pass explicitly via parameters |
| **Utility / Math** | 🚫 Forbidden | Maintain complete independence |

```cpp
// ❌ Component directly accessing singleton
Renderer::GetInstance().Draw();

// ✅ Explicit passing
component->Render(renderer);
```

### ✅ Consider Testability
```cpp
// ✅ Make testable with dependency injection
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

## 🛡️ 14. Exception Handling and Error Management

### ✅ HRESULT Handling
```cpp
inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw std::runtime_error("DirectX API call failed");
    }
}

// Usage
ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&mQueue)));
```

### ✅ Exception Usage Principles
- **Initialization failure**: Throw exception
- **Runtime error**: Logging + recovery or safe termination
- **Game logic**: Avoid exceptions, use return values

```cpp
// ✅ Initialization
void Renderer::Init()
{
    ThrowIfFailed(CreateDevice());
}

// ✅ Runtime
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

### ✅ noexcept Usage
- Destructors: Always `noexcept`
- Move operators: `noexcept` if possible
- Performance-critical functions: Consider `noexcept`

---

## 📝 15. Comment Guidelines

### ✅ Basic Principles
- Explain **Why** over What
- Self-documenting code first
- Use TODO/FIXME tags

### ✅ Use Objective Expressions
- Avoid absolute terms (Perfect, Always, Never, Best)
- Use specific and measurable expressions
- Clearly state advantages, disadvantages, or conditions

**Examples:**
```cpp
// ❌ Perfect for all use cases
// ✅ Well-suited for per-frame allocations due to O(1) allocation time

// ❌ Always use this method
// ✅ Recommended for single-threaded scenarios

// ❌ The best approach
// ✅ More efficient than alternative X when condition Y is met

### ✅ File Header
```cpp
/**
 * @file Renderer.h
 * @brief Core DirectX 12 rendering system class
 * @author YourName
 * @date 2025-10-04
 */
```

### ✅ Function Documentation
```cpp
/**
 * @brief Renders triangles to the screen
 * @param vertices Vertex array
 * @param count Number of vertices (multiple of 3)
 * @param transform Transformation matrix
 * @return true on success
 * @note Requires DirectX 12 pipeline state
 */
bool DrawTriangles(const Vertex* vertices, size_t count, const Matrix& transform);
```

### 📝 Function Documentation Criteria

**Must document:**
- Public API functions
- Functions with usage constraints
- Functions with unclear parameter meanings
- Functions that can throw exceptions

**Optional documentation:**
- Protected functions (case by case)
- Complex algorithm implementations

**No documentation needed:**
- Self-explanatory Getters/Setters
- Private internal implementation functions
- When meaning is clear from function name and parameters alone

**Principle:**
> "If a developer seeing this function for the first time can't understand it in 5 seconds, document it"

### ✅ TODO Tags
```cpp
// TODO: Add multithreaded rendering support
// FIXME: Crash on window resize (Issue #42)
// NOTE: Optimization needed - currently takes 100ms
// HACK: Temporary solution, needs future revision
```

---

## 🔧 16. Template Usage

### ✅ Template Naming
```cpp
// ✅ Simple case
template<typename T>
class Array { };

// ✅ When meaning is important
template<typename VertexType, typename IndexType>
class Mesh { };

// ✅ C++20 Concepts
template<std::integral T>
T Add(T a, T b);
```

### ✅ Definition Location
- Full definition in header (default)
- Separate to cpp only when explicit instantiation needed

---

## ⚡ 17. Performance Optimization

### ✅ Minimize Copying
```cpp
// ✅ Return const reference
const std::vector<Vertex>& GetVertices() const
{
    return mVertices;
}

// ✅ Move semantics
std::vector<Vertex> ExtractVertices()
{
    return std::move(mVertices);
}
```

### ✅ Reserve and Emplace
```cpp
std::vector<Vertex> vertices;
vertices.reserve(1000);  // Pre-allocate
vertices.emplace_back(pos, normal, uv);  // Construct in-place
```

### ✅ Inline Functions
- Simple Getter/Setter inline in header
- Only for simple logic ≤3 lines

---

## 🔀 18. Multithreading

### ✅ Synchronization Primitives
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

### ✅ Thread-safe Singleton
```cpp
class Engine
{
public:
    static Engine& GetInstance()
    {
        static Engine instance;  // Thread-safe since C++11
        return instance;
    }
    
private:
    Engine() = default;
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
};
```

---

## 🚫 19. Anti-patterns

### ❌ Macro Abuse
```cpp
// ❌ Global access macro
#define DEVICE Engine::GetInstance().GetDevice()

// ✅ Explicit passing
void Init(Device* device);
```

### ❌ Unnecessary Smart Pointers
```cpp
// ❌ shared_ptr when ownership not needed
void Draw(const std::shared_ptr<Mesh>& mesh);

// ✅ Reference is sufficient
void Draw(const Mesh& mesh);
```

### ❌ Meaningless Comments
```cpp
// ❌ Stating the obvious
i++;  // Increment i by 1

// ✅ Explain why
i++;  // Move to next frame index
```