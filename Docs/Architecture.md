# DevMiniEngine Architecture

**[한국어](./Architecture.ko.md)** | **English**

> 📝 **Note**: This document describes the planned architecture. Implementation is in progress.

## 📋 Table of Contents

- [Overview](#overview)
- [Design Philosophy](#design-philosophy)
- [Core Architecture](#core-architecture)
- [Module Details](#module-details)
- [Data Flow](#data-flow)
- [Memory Management](#memory-management)
- [Threading Model](#threading-model)
- [Future Considerations](#future-considerations)

---

## 🎯 Overview

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

## 🏛️ Design Philosophy

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
│      Core (Memory, Threading)       │
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

## 🏗️ Core Architecture

### Layer Breakdown

#### 1. Platform Layer
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

#### 2. Math Layer
**Responsibility**: Mathematical primitives and operations

**Features:**
- SIMD-optimized vector/matrix operations
- Quaternion for rotations
- Transform hierarchy support
- Common math utilities

```cpp
// Math/include/Math/Vector.h
struct Vector3
{
    float x, y, z;
    
    Vector3 operator+(const Vector3& rhs) const;
    float Dot(const Vector3& rhs) const;
    Vector3 Cross(const Vector3& rhs) const;
    // ... SIMD implementations
};
```

#### 3. Core Layer
**Responsibility**: Engine foundation systems

**Subsystems:**
- **Memory Management**: Custom allocators, memory pools
- **Threading**: Job system, thread pool
- **Logging**: Structured logging with levels
- **Containers**: Custom containers optimized for engine use

```cpp
// Core/include/Core/Memory/PoolAllocator.h
template<typename T, size_t PoolSize>
class PoolAllocator
{
public:
    T* Allocate();
    void Deallocate(T* ptr);
};

// Core/include/Core/Threading/JobSystem.h
class JobSystem
{
public:
    void Schedule(Job* job);
    void Wait(JobHandle handle);
};
```

#### 4. ECS Layer
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

#### 5. Graphics Layer

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

#### 7. Scene Layer

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

## 🔄 Data Flow

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

### Render Frame Flow

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

## 💾 Memory Management

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

**Allocator Types:**

```cpp
// Core/include/Core/Memory/Allocators.h

// Linear allocator for per-frame data
class LinearAllocator
{
    // Fast allocation, bulk deallocation
};

// Pool allocator for fixed-size objects
template<typename T>
class PoolAllocator
{
    // O(1) allocation/deallocation
};

// Stack allocator for scoped allocations
class StackAllocator
{
    // LIFO allocation pattern
};
```

**Usage Example:**
```cpp
// Per-frame allocations
void RenderSystem::Update(World& world, float deltaTime)
{
    FrameScope scope(gFrameAllocator);
    
    // Allocate temporary render commands
    auto* commands = scope.AllocArray<RenderCommand>(entityCount);
    
    // ... use commands
    
    // Automatically freed at scope end
}
```

---

## 🧵 Threading Model

### Thread Architecture (Planned)

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

## 🔮 Future Considerations

### Phase 1: Foundation (Current)
- [x] Project structure
- [ ] Core systems (Memory, Threading, Logging)
- [ ] Math library
- [ ] Basic ECS framework

### Phase 2: Graphics
- [ ] DirectX 12 initialization
- [ ] Basic rendering pipeline
- [ ] Mesh and texture loading
- [ ] Camera system

### Phase 3: Advanced ECS
- [ ] Component queries
- [ ] System dependencies
- [ ] Event system
- [ ] Archetype optimization

### Phase 4: Gameplay Systems
- [ ] Physics integration
- [ ] Collision detection
- [ ] AI pathfinding (A*)
- [ ] Terrain generation

### Phase 5: Advanced Rendering
- [ ] PBR materials
- [ ] Shadow mapping
- [ ] Post-processing
- [ ] Render graph

### Phase 6: Tools
- [ ] Asset pipeline
- [ ] Resource hot-reloading
- [ ] ImGui debug UI
- [ ] Basic editor prototype

---

## 📚 References & Inspirations

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

## 🔄 Document History

- **2025-10-05**: Initial architecture design document
- Future updates will be tracked here

---

**Note**: This architecture is subject to change as the project evolves and new requirements emerge. Practical experience may lead to design refinements.