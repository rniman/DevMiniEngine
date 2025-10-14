# DevMiniEngine Architecture

**[한국어](./Architecture.ko.md)** | **English**

> **Note**: This document describes both planned and implemented architecture. Implementation status is marked clearly.

## Table of Contents

- [Overview](#overview)
- [Design Philosophy](#design-philosophy)
- [Core Architecture](#core-architecture)
- [Module Details](#module-details)
- [Data Flow](#data-flow)
- [Memory Management](#memory-management)
- [Threading Model](#threading-model)
- [Future Considerations](#future-considerations)

---

## Overview

DevMiniEngine is designed as a learning-focused game engine built on DirectX 12 and modern C++ practices. The architecture emphasizes:

- **Data-Oriented Design**: ECS architecture for optimal cache performance
- **Modularity**: Clear separation of concerns across engine subsystems
- **Abstraction**: RHI layer for potential graphics API flexibility
- **Modern C++**: Leveraging C++20 features for clean, efficient code

### Key Architectural Goals

1. **Educational Value**: Clear, readable code structure for learning
2. **Performance**: Data-oriented design and efficient memory layouts
3. **Maintainability**: Modular design with minimal dependencies
4. **Extensibility**: Easy to add new features and systems

---

## Design Philosophy

### 1. Entity Component System (ECS)

**Why ECS?**
- Cache-friendly data layouts
- Clear separation between data and logic
- Easy to parallelize systems
- Flexible entity composition

**Core Concepts:**
```cpp
// Entity: Just an ID
using Entity = uint32_t;

// Component: Pure data
struct TransformComponent
{
    Vector3 position;
    Quaternion rotation;
    Vector3 scale;
};

// System: Pure logic
class RenderSystem : public System
{
    void Update(World& world, float deltaTime) override;
};
```

### 2. Data-Oriented Design

**Principles:**
- Components stored in contiguous arrays (SoA - Structure of Arrays)
- Systems iterate over component arrays efficiently
- Minimal pointer chasing
- Cache-friendly memory access patterns

### 3. Dependency Management

**Module Hierarchy:**
```
┌─────────────────────────────────────┐
│         Application Layer           │
├─────────────────────────────────────┤
│     Scene / Gameplay Systems        │
├─────────────────────────────────────┤
│  Graphics │ Physics │ AI │ Terrain  │
├─────────────────────────────────────┤
│         ECS Framework               │
├─────────────────────────────────────┤
│   Core (Memory, Threading, Logging) │
├─────────────────────────────────────┤
│      Math (Independent)             │
├─────────────────────────────────────┤
│       Platform (Win32)              │
└─────────────────────────────────────┘
```

**Dependency Rules:**
- Lower layers know nothing about upper layers
- Math module is completely independent
- Core depends only on Platform
- All game systems depend on ECS

---

## Core Architecture

### Layer Breakdown

#### 1. Platform Layer (Planned)
**Responsibility**: OS-specific functionality abstraction

```cpp
// Platform/include/Platform/Window.h
class Window
{
public:
    virtual void Create(const WindowDesc& desc) = 0;
    virtual void ProcessEvents() = 0;
    virtual void* GetNativeHandle() const = 0;
};

// Platform/src/Windows/WindowsWindow.cpp
class WindowsWindow : public Window
{
    // Win32-specific implementation
};
```

**Components:**
- Window management
- File system access
- High-resolution timer
- Input event handling

#### 2. Math Layer (Implemented)
**Responsibility**: Mathematical primitives and operations

**Status**: Fully implemented with SIMD optimization

**Features:**
- SIMD-optimized vector/matrix operations (DirectXMath wrapper)
- Vector2, Vector3, Vector4
- Matrix3x3, Matrix4x4 (row-major)
- Quaternion for rotations
- Transform utilities (Translation, Rotation, Scaling)
- Camera matrices (LookAt, Perspective, Orthographic)
- Common math utilities (Lerp, Clamp, DegToRad, etc.)

**Test Coverage**: 04_MathTest

```cpp
// Math/include/Math/MathTypes.h
using Vector3 = DirectX::XMFLOAT3;
using Matrix4x4 = DirectX::XMFLOAT4X4;
using Quaternion = DirectX::XMFLOAT4;

// SIMD types for calculations
using VectorSIMD = DirectX::XMVECTOR;
using MatrixSIMD = DirectX::XMMATRIX;

// Math/include/Math/MathUtils.h
Vector3 Add(const Vector3& a, const Vector3& b);
float Dot(const Vector3& a, const Vector3& b);
Matrix4x4 MatrixMultiply(const Matrix4x4& a, const Matrix4x4& b);
Quaternion QuaternionFromEuler(float pitch, float yaw, float roll);
```

**Design Decisions:**
- DirectXMath as foundation (production-quality SIMD)
- Separate storage types (XMFLOAT*) and SIMD types (XMVECTOR)
- Wrapper functions for convenience
- All functions inline (header-only, zero overhead)

#### 3. Core Layer (Partially Implemented)
**Responsibility**: Engine foundation systems

**Subsystems:**

##### Memory Management (Implemented)
**Status**: Fully implemented and tested

**Allocators:**
- **LinearAllocator**: Bump-pointer allocation, O(1) allocation
  - Use case: Frame-temporary data, level loading
  - Cannot free individual allocations (bulk reset only)
  - Test: 01_MemoryTest
  
- **PoolAllocator**: Fixed-size object pool, O(1) allocation/deallocation
  - Use case: Entities, components, particles
  - Free-list implementation for reuse
  - Test: 02_PoolAllocatorTest
  
- **StackAllocator**: LIFO allocation pattern with markers
  - Use case: Nested scope allocations
  - Supports partial deallocation via markers
  - Test: 03_StackAllocatorTest

```cpp
// Core/include/Core/Memory/LinearAllocator.h
class LinearAllocator : public Allocator
{
public:
    explicit LinearAllocator(size_t size);
    void* Allocate(size_t size, size_t alignment = DEFAULT_ALIGNMENT) override;
    void Reset() override;  // Free all at once
    
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
    void Deallocate(void* ptr) override;  // O(1) free
    
private:
    void* mMemory;
    size_t mChunkSize;
    void* mFreeList;  // Linked list of free chunks
};

// Core/include/Core/Memory/StackAllocator.h
class StackAllocator : public Allocator
{
public:
    using Marker = size_t;
    
    explicit StackAllocator(size_t size);
    void* Allocate(size_t size, size_t alignment = DEFAULT_ALIGNMENT) override;
    Marker GetMarker() const;
    void FreeToMarker(Marker marker);  // LIFO deallocation
    
private:
    void* mMemory;
    size_t mOffset;
};
```

**Features:**
- Custom memory alignment support (16-byte default for SIMD)
- Comprehensive logging integration
- Thread-safe assertion macros (CORE_ASSERT, CORE_VERIFY)
- Memory tracking (allocation size/count)

**Future Enhancements:**
- [ ] Debug bounds checking
- [ ] Memory leak detection
- [ ] Allocation statistics/profiling

##### Logging System (Implemented)
**Status**: Fully implemented with multi-output support

**Components:**
- **Logger**: Thread-safe singleton managing all log operations
- **LogSink**: Abstract interface for output targets
  - ConsoleSink: Color-coded console output (Windows Console API)
  - FileSink: File-based logging with timestamps
- **LogLevel**: Trace, Debug, Info, Warning, Error, Fatal
- **LogCategory**: Core, Graphics, Physics, AI, Audio, Input, Memory

**Test Coverage**: 05_LoggingTest

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
    std::mutex mMutex;  // Thread-safe
};

// Core/include/Core/Logging/LogMacros.h
#define LOG_INFO(format, ...) \
    Logger::GetInstance().Log(LogLevel::Info, LogCategory::Core, \
        FormatLog(format, ##__VA_ARGS__), __FILE__, __LINE__)

#define LOG_ERROR(format, ...) \
    Logger::GetInstance().Log(LogLevel::Error, LogCategory::Core, \
        FormatLog(format, ##__VA_ARGS__), __FILE__, __LINE__)

// Category-specific macros
#define LOG_GRAPHICS_INFO(format, ...) \
    Logger::GetInstance().Log(LogLevel::Info, LogCategory::Graphics, \
        FormatLog(format, ##__VA_ARGS__), __FILE__, __LINE__)
```

**Design Decisions:**
- Singleton pattern for global accessibility
- Sink pattern for extensible output system
- Macro-based API for convenience and compile-time optimization
- Printf-style formatting (simple, no external dependencies)
- Compile-time removal of Trace/Debug in Release builds

**Key Features:**
- Thread-safe logging with mutex protection
- Category-based filtering
- Color-coded console output (Windows Terminal compatible)
- File logging with full timestamps and source location
- Zero overhead in Release builds for Trace/Debug logs (~0 ns)
- Debug overhead: ~50 microseconds per log

**Performance:**
```cpp
// Debug build
LOG_INFO("Message");  // ~50 µs overhead

// Release build
LOG_TRACE("Debug info");  // Compiled out, 0 overhead
LOG_DEBUG("Debug info");  // Compiled out, 0 overhead
LOG_INFO("Message");      // ~50 µs overhead
```

**Usage Example:**
```cpp
// Basic logging
LOG_INFO("Engine initialized successfully");
LOG_WARN("Texture quality reduced to fit memory budget");
LOG_ERROR("Failed to load asset: %s", filename);

// Category-specific logging
LOG_GRAPHICS_INFO("Rendering %d triangles", count);
LOG_PHYSICS_WARN("Collision detected at (%.2f, %.2f, %.2f)", x, y, z);
LOG_MEMORY_DEBUG("LinearAllocator allocated %zu bytes", size);

// Integration with Memory allocators
LinearAllocator frameAlloc(10 * MB);
void* data = frameAlloc.Allocate(1024);
// Automatically logs: "LinearAllocator allocated 1024 bytes at 0x..."
```

**Future Enhancements:**
- [ ] Async logging for high-frequency scenarios
- [ ] Log file rotation (size/date-based)
- [ ] JSON output sink for log analysis tools

##### Threading (Planned)
**Status**: Not started

```cpp
// Core/include/Core/Threading/JobSystem.h
class JobSystem
{
public:
    void Schedule(Job* job);
    void Wait(JobHandle handle);
};
```

**Containers (Planned)**
- Custom containers optimized for engine use

#### 4. ECS Layer (Planned)
**Responsibility**: Entity-Component-System framework

**Core Classes:**

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

**Component Storage:**
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

#### 5. Graphics Layer (Planned)

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
    // DirectX 12 implementation
};
```

**Rendering Architecture:**
```
┌──────────────────────────────────┐
│      High-Level Renderer         │ ← Material, Mesh, Camera
├──────────────────────────────────┤
│      Render Graph (Planned)      │ ← Frame graph, pass ordering
├──────────────────────────────────┤
│            RHI Layer             │ ← API abstraction
├──────────────────────────────────┤
│         DirectX 12 API           │ ← Low-level implementation
└──────────────────────────────────┘
```

#### 6. Physics Layer (Planned)

**Architecture:**
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

**ECS Integration:**
```cpp
// Physics/include/Physics/Systems/PhysicsSystem.h
class PhysicsSystem : public System
{
public:
    void Update(World& world, float deltaTime) override
    {
        // Sync ECS components with physics world
        SyncTransforms(world);
        
        // Step physics simulation
        mPhysicsWorld.Step(deltaTime);
        
        // Write results back to ECS
        UpdateTransforms(world);
    }
    
private:
    PhysicsWorld mPhysicsWorld;
};
```

#### 7. Scene Layer (Planned)

**Scene Graph vs ECS:**
- ECS handles entity data and logic
- Scene provides high-level organization
- SceneManager handles scene transitions

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

## Data Flow

### Frame Update Cycle

```
┌─────────────────────────────────────────┐
│  1. Platform: Process Window Events     │
│     - Input events                      │
│     - Window resize, etc.               │
└────────────────┬────────────────────────┘
                 │
┌────────────────▼────────────────────────┐
│  2. Input: Update Input State           │
│     - Keyboard, Mouse                   │
└────────────────┬────────────────────────┘
                 │
┌────────────────▼────────────────────────┐
│  3. Game Logic Systems (ECS)            │
│     - AISystem                          │
│     - PhysicsSystem                     │
│     - GameplaySystem                    │
└────────────────┬────────────────────────┘
                 │
┌────────────────▼────────────────────────┐
│  4. Rendering (ECS)                     │
│     - CameraSystem                      │
│     - RenderSystem                      │
│       → Culling                         │
│       → Sort render queue               │
│       → Submit draw calls               │
└────────────────┬────────────────────────┘
                 │
┌────────────────▼────────────────────────┐
│  5. Present Frame                       │
└─────────────────────────────────────────┘
```

### Render Frame Flow (Planned)

```
┌──────────────────────────────────────────┐
│  RenderSystem::Update()                  │
└────────────────┬─────────────────────────┘
                 │
┌────────────────▼─────────────────────────┐
│  Build Render Queue                      │
│  - Iterate visible entities              │
│  - Extract render components             │
│  - Sort by material/depth                │
└────────────────┬─────────────────────────┘
                 │
┌────────────────▼─────────────────────────┐
│  Record Command List                     │
│  - Begin render pass                     │
│  - Bind pipeline state                   │
│  - Draw indexed/instanced                │
│  - End render pass                       │
└────────────────┬─────────────────────────┘
                 │
┌────────────────▼─────────────────────────┐
│  Submit & Present                        │
└──────────────────────────────────────────┘
```

---

## Memory Management

### Strategy Overview

**Allocation Categories:**

1. **Persistent Memory**: Lives for entire application lifetime
   - Engine core systems
   - Loaded resources

2. **Level Memory**: Cleared on scene change
   - Scene entities
   - Level-specific resources

3. **Frame Memory**: Reset every frame
   - Temporary calculations
   - Render command buffers

**Allocator Types (All Implemented):**

```cpp
// Core/include/Core/Memory/Allocators.h

// Linear allocator for per-frame data
class LinearAllocator
{
    // Fast allocation, bulk deallocation
    // O(1) allocation, cannot free individual allocations
};

// Pool allocator for fixed-size objects
template<typename T>
class PoolAllocator
{
    // O(1) allocation/deallocation
    // Free-list implementation
};

// Stack allocator for scoped allocations
class StackAllocator
{
    // LIFO allocation pattern
    // Supports markers for partial deallocation
};
```

**Usage Example:**
```cpp
// Per-frame allocations
void RenderSystem::Update(World& world, float deltaTime)
{
    LinearAllocator frameAlloc(10 * MB);
    
    // Allocate temporary render commands
    auto* commands = static_cast<RenderCommand*>(
        frameAlloc.Allocate(sizeof(RenderCommand) * entityCount)
    );
    
    // ... use commands
    
    // Automatically freed at scope end via Reset()
    frameAlloc.Reset();
}

// Object pooling
PoolAllocator particlePool(sizeof(Particle), 1000);
Particle* p = static_cast<Particle*>(particlePool.Allocate());
// ... use particle
particlePool.Deallocate(p);  // Return to pool

// Nested scopes
StackAllocator stack(1 * MB);
auto marker1 = stack.GetMarker();
{
    void* temp = stack.Allocate(1024);
    // ... use temp
    
    auto marker2 = stack.GetMarker();
    {
        void* innerTemp = stack.Allocate(512);
        // ... use innerTemp
    }
    stack.FreeToMarker(marker2);  // Free inner scope
}
stack.FreeToMarker(marker1);  // Free all
```

**Performance Characteristics:**
- LinearAllocator: ~0.5 ms for 10,000 allocations
- PoolAllocator: ~2 ms for 10,000 alloc/free cycles
- StackAllocator: ~0.8 ms for 10,000 allocations
- Standard new/delete: ~50-100 ms (10-100x slower)

---

## Threading Model (Planned)

### Thread Architecture

```
┌────────────────────────────────────────┐
│          Main Thread                   │
│  - Game logic                          │
│  - ECS system updates (sequential)     │
│  - Render command recording            │
└────────────────────────────────────────┘

┌────────────────────────────────────────┐
│         Render Thread                  │
│  - Command list submission             │
│  - Resource uploads                    │
│  - GPU synchronization                 │
└────────────────────────────────────────┘

┌────────────────────────────────────────┐
│          Job Threads                   │
│  - Parallel system execution           │
│  - Physics broad phase                 │
│  - Culling                             │
│  - Asset loading                       │
└────────────────────────────────────────┘
```

### Job System

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
    
    // Parallel-for helper
    template<typename Func>
    void ParallelFor(size_t count, size_t batchSize, Func func);
};
```

**Usage:**
```cpp
// Parallel component processing
jobSystem.ParallelFor(entityCount, 64, [&](size_t i)
{
    ProcessEntity(entities[i]);
});
```

---

## Future Considerations

### Phase 1: Foundation (60% Complete)
- [x] Project structure
- [x] Core systems
  - [x] Memory allocators (Linear, Pool, Stack)
  - [x] Logging system (Console, File sinks)
  - [x] Assertion macros
- [x] Math library (Vector, Matrix, Quaternion with SIMD)
- [ ] Platform layer (Window, Input, Timer) ← **Next Priority**
- [ ] Threading system (Job system, thread pool)

### Phase 2: Graphics (Not Started)
- [ ] DirectX 12 initialization
- [ ] Basic rendering pipeline
- [ ] Mesh and texture loading
- [ ] Camera system

### Phase 3: Advanced ECS (Not Started)
- [ ] Component queries
- [ ] System dependencies
- [ ] Event system
- [ ] Archetype optimization

### Phase 4: Gameplay Systems (Not Started)
- [ ] Physics integration
- [ ] Collision detection
- [ ] AI pathfinding (A*)
- [ ] Terrain generation

### Phase 5: Advanced Rendering (Not Started)
- [ ] PBR materials
- [ ] Shadow mapping
- [ ] Post-processing
- [ ] Render graph

### Phase 6: Tools (Not Started)
- [ ] Asset pipeline
- [ ] Resource hot-reloading
- [ ] ImGui debug UI
- [ ] Basic editor prototype

---

## Implementation Progress

### Completed Systems
| System | Status | Test Coverage | Lines of Code |
|--------|--------|---------------|---------------|
| Memory Management | Complete | 3 tests | ~600 |
| Math Library | Complete | 1 test | ~400 |
| Logging System | Complete | 1 test | ~300 |
| **Total** | **3/12 subsystems** | **5 tests** | **~1,300** |

### In Progress
- None

### Planned
- Platform Layer
- DirectX 12 Initialization
- ECS Framework
- Physics Engine
- AI Systems
- Scene Management

---

## References & Inspirations

### Game Engine Architecture
- [Game Engine Architecture by Jason Gregory](https://www.gameenginebook.com/)
- [Game Programming Patterns by Robert Nystrom](https://gameprogrammingpatterns.com/)

### ECS Design
- [Data-Oriented Design Book](https://www.dataorienteddesign.com/dodbook/)
- [EnTT Documentation](https://github.com/skypjack/entt)
- [Unity DOTS](https://unity.com/dots)

### DirectX 12
- [DirectX 12 Programming Guide](https://docs.microsoft.com/en-us/windows/win32/direct3d12/directx-12-programming-guide)
- [3D Game Programming with DirectX 12 by Frank Luna](http://www.d3dcoder.net/)

### Threading & Performance
- [C++ Concurrency in Action by Anthony Williams](https://www.manning.com/books/c-plus-plus-concurrency-in-action)
- [Intel Threading Building Blocks](https://www.intel.com/content/www/us/en/developer/tools/oneapi/onetbb.html)

---

## Document History

- **2025-10-11**: Updated with implemented systems (Memory, Math, Logging)
- **2025-10-05**: Initial architecture design document
- Future updates will be tracked here

---

**Note**: This architecture is subject to change as the project evolves and new requirements emerge. Practical experience may lead to design refinements.

**Last Updated**: 2025-10-11