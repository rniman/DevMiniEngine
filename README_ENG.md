# DevMiniEngine

**[한국어](./README.md)** | **English**

**DirectX 12-based Mini Game Engine for Learning**

## Project Overview

DevMiniEngine is a personal learning and portfolio project based on DirectX 12. It is designed to learn and experiment with ECS (Entity Component System) architecture and modern C++ design patterns.

### Key Objectives

- **ECS Architecture Learning**: Implementation of data-oriented design and entity-component systems
- **Modern Graphics Technology**: Building modern rendering pipeline using DirectX 12
- **Game Engine Fundamentals**: Implementation of core engine systems including physics, AI, and terrain generation
- **Portfolio Development**: Professional-level code quality and documentation

## Implementation Status

### Completed Systems

**Phase 1: Foundation Systems (100% Complete)**

**Core Systems**
- Memory Management System
  - LinearAllocator: O(1) bump pointer allocation
  - PoolAllocator: O(1) fixed-size object pooling
  - StackAllocator: LIFO allocation using markers
- Timing System
  - High-precision timer (QueryPerformanceCounter)
  - Frame time averaging (50 samples)
  - FPS measurement and profiling tools
- Logging System
  - Multi-output Sink architecture (Console, File)
  - Category-based filtering
  - Color-coded console output

**Math Library**
- SIMD-optimized vector/matrix operations (DirectXMath wrapper)
- Vector2, Vector3, Vector4
- Matrix4x4 (row-major)
- Quaternion operations
- Transform utilities
- Camera matrices (LookAt, Perspective)

**Platform Layer**
- Win32 window creation and management
- Event handling system
- Input system (keyboard, mouse)

**Phase 2: DirectX 12 Graphics & Framework (100% Complete)**

**DirectX 12 Rendering**
- Complete rendering pipeline
  - Vertex/Index Buffer
  - Shader compilation system
  - Pipeline State Object (PSO)
  - Root Signature
  - Constant Buffer
  - Depth-Stencil Buffer
  - DX12Renderer
    - Owns all rendering resources
    - Separation of Scene and rendering responsibilities
- Multi-texture support (7 types)
  - Diffuse, Normal, Specular, Roughness, Metallic, AO, Emissive
- 3D transformation and camera
  - MVP matrix transformation
  - PerspectiveCamera implementation

**Framework Architecture**
- Application lifecycle management
  - Template method pattern
  - Separation of engine/user initialization
- ResourceManager
  - Centralized management of Mesh, Material, Texture
  - Caching and duplicate loading prevention
- Scene/GameObject system
  - Transform hierarchy
  - Rendering data collection (What/How separation)

### Project Statistics

- Total lines of code: ~5,000+
- Implemented modules: 5 (Core, Math, Platform, Graphics, Framework)
- Test coverage: 8 sample projects
- Compiler warnings: 0 (Level 4)

## Project Structure
```
DevMiniEngine/
├── Engine/                          # Engine core
│   ├── include/                     # Public headers for all modules
│   │   ├── Core/                    # Core headers
│   │   │   ├── Memory/              # Memory management
│   │   │   ├── Logging/             # Logging system
│   │   │   └── Timing/              # Timer system
│   │   ├── Math/                    # Math library
│   │   ├── Platform/                # Platform layer
│   │   ├── Graphics/                # Graphics
│   │   │   ├── DX12/                # DirectX 12
│   │   │   ├── Camera/              # Camera system
│   │   │   └── RenderTypes.h        # Rendering types
│   │   └── Framework/               # Framework layer
│   │       ├── Application.h        # Application base
│   │       ├── Resources/           # Resource management
│   │       └── Scene/               # Scene management
│   │
│   ├── src/                         # Implementation for all modules
│   │   └── (same structure)
│   │
│   ├── Core/                        # Core module project
│   ├── Math/                        # Math module project
│   ├── Platform/                    # Platform module project
│   ├── Graphics/                    # Graphics module project
│   └── Framework/                   # Framework module project
│
├── Samples/                         # Sample projects
│   ├── 01_MemoryTest/
│   ├── ...
│   └── 08_TexturedCube/             # Textured cube rendering
│
├── Assets/                          # Assets
│   └── Textures/                    # Texture files
│       └── BrickWall/               # PBR texture set
│
└── Docs/                            # Documentation
    ├── Architecture.md
    ├── CodingConvention.md
    └── DevelopmentLog.md
```

## Build and Run

### Requirements

- **OS**: Windows 10/11 (64-bit)
- **IDE**: Visual Studio 2022 or later
- **SDK**: Windows 10 SDK (10.0.19041.0 or later)
- **DirectX**: DirectX 12-capable GPU
- **C++ Standard**: C++20

### Build Instructions

1. **Clone Repository**
```bash
git clone https://github.com/rniman/DevMiniEngine.git
cd DevMiniEngine
```

2. **Open Solution**
```bash
DevMiniEngine.sln
```

3. **Build Configuration**
   - `Debug` - For debugging
   - `Release` - Optimized build

4. **Build**
   - `Ctrl + Shift + B`

### Running Examples
```bash
# Run textured cube sample
bin/Debug/08_TexturedCube.exe
```

## Roadmap

### Phase 1-2: Foundation Systems (100% Complete)
- [x] Project structure
- [x] Core systems (memory, logging, timer)
- [x] Math library (SIMD)
- [x] Platform layer (Window, Input)
- [x] DirectX 12 initialization and rendering pipeline
- [x] Mesh and texture system
- [x] Camera system
- [x] Framework architecture (Application, ResourceManager, Scene)

**Current State:** Complete PBR texture set loading and shader binding pipeline (Diffuse, Normal, Metallic, Roughness, etc. 7 types). Current rendering uses Diffuse map as baseline; shader implementation utilizing loaded PBR maps is next step.

---

### Phase 3: ECS Architecture & Debug Tools
**Goal:** Core implementation of data-oriented design

- [ ] **ECS Core**
  - [ ] Entity Manager (create/destroy/recycle)
  - [ ] Component Storage (Archetype-based)
  - [ ] System Framework (execution order management)
- [ ] **Core Components & Systems**
  - [ ] TransformComponent (hierarchy, World Matrix caching)
  - [ ] TransformSystem (Dirty Flag propagation)
- [ ] **Query System**
  - [ ] Component composition queries
  - [ ] Query caching and optimization
- [ ] **Debug Tools (early adoption)**
  - [ ] ImGui integration
  - [ ] ECS Inspector (Entity/Component editing)
  - [ ] Performance monitoring (FPS, Draw Calls)

**Upon Completion:** Manage 1000 entities, real-time editing with ImGui

---

### Phase 3.5: Job System (Optional)
**Goal:** Basic multithreading infrastructure

- [ ] Worker thread pool
- [ ] Job dispatcher
- [ ] TransformSystem parallelization
- [ ] Performance benchmark (Single vs Multi-thread)

**Note:** Can be postponed to Phase 9 (after sufficient parallelizable work)

---

### Phase 4: DX12 Infrastructure Expansion & Asset Pipeline
**Goal:** Asset pipeline for productivity improvement

- [ ] **Descriptor Management Enhancement**
  - [ ] SRV/UAV Descriptor Heap integration
  - [ ] Descriptor pool and recycling
  - [ ] Frame-based allocation
- [ ] **Resource Upload Optimization**
  - [ ] Upload buffer ring
  - [ ] Async Copy Queue (optional)
- [ ] **Asset Pipeline**
  - [ ] glTF 2.0 loader (mesh, PBR materials, hierarchy)
  - [ ] DDS/BCn texture pipeline
  - [ ] Automatic mipmap generation
- [ ] **Shader System Expansion**
  - [ ] DXC compiler (Shader Model 6.6+)
  - [ ] Shader Reflection (automatic Root Signature)
  - [ ] Hot Reload (optional)
- [ ] **ECS Rendering Integration**
  - [ ] MeshRendererComponent
  - [ ] RenderSystem (ECS query-based)

**Upon Completion:** Blender model loading, ECS-based rendering

---

### Phase 4.5: Frame Graph & Culling
**Goal:** Efficient rendering pipeline

- [ ] **Frame Graph**
  - [ ] Pass/Resource dependency graph
  - [ ] Automatic Barrier insertion
  - [ ] Resource Lifetime management
- [ ] **Culling System**
  - [ ] Frustum Culling (CPU)
  - [ ] AABB Bounding Box
- [ ] **Asset Pipeline Expansion (optional)**
  - [ ] Hot Reload (textures, shaders)

**Upon Completion:** Efficient rendering of 1000 objects

---

### Phase 5: PBR & Post-Processing
**Goal:** Realistic rendering

- [ ] **PBR Material**
  - [ ] Metallic-Roughness workflow
  - [ ] Cook-Torrance BRDF
  - [ ] IBL (Skybox, PMREM, Irradiance Map)
  - [ ] Uber Shader & Permutations
- [ ] **Post-Processing**
  - [ ] Tone mapping (Reinhard, ACES)
  - [ ] Auto Exposure
  - [ ] Bloom
  - [ ] TAA (Temporal Anti-Aliasing)

**Upon Completion:** Various objects with PBR materials, IBL lighting

---

### 🎯 Phase 5.5: Physics Integration (Portfolio Milestone)
**Goal:** Create playable prototype

> **Important:** Portfolio video production possible from this point!

- [ ] **Physics Engine Integration**
  - [ ] Choose Jolt Physics or Bullet
  - [ ] Physics World initialization
- [ ] **Physics Components (ECS)**
  - [ ] RigidBodyComponent (Mass, Velocity)
  - [ ] ColliderComponent (Box, Sphere, Capsule)
  - [ ] Physics Material (Friction, Restitution)
- [ ] **PhysicsSystem**
  - [ ] Transform ↔ Physics Engine synchronization
  - [ ] Fixed Timestep
  - [ ] Collision events (OnCollisionEnter/Exit)
- [ ] **Sandbox Demo**
  - [ ] Cubes falling with gravity
  - [ ] Player pushing objects (Mouse Ray)
  - [ ] Edit RigidBody properties with ImGui
- [ ] **Physics Debug Rendering**
  - [ ] Collider wireframe
  - [ ] Contact Point display

**Upon Completion:** PBR materials + Physics interaction demo (portfolio material secured)

---

### Phase 6: Shadows & Ambient
**Goal:** Enhanced visual fidelity

- [ ] **Shadow Mapping**
  - [ ] Basic Shadow Map (Depth, PCF)
  - [ ] Cascaded Shadow Maps (CSM)
  - [ ] Cascade boundary blending
- [ ] **Ambient Occlusion**
  - [ ] SSAO (Screen Space Ambient Occlusion)
  - [ ] Bilateral Blur
- [ ] **Additional Effects (optional)**
  - [ ] SSR (Screen Space Reflections)
  - [ ] DOF (Depth of Field)

**Upon Completion:** Physics sandbox + dynamic shadows + SSAO

---

### Phase 7: Compute Shaders & GPU Acceleration
**Goal:** Utilize GPU computation

- [ ] **Compute Pipeline**
  - [ ] Dispatch framework
  - [ ] UAV (Unordered Access View) management
- [ ] **GPU Particle System**
  - [ ] Compute Shader-based simulation
  - [ ] Indirect Drawing
  - [ ] Interaction with Physics objects
- [ ] **GPU Culling (optional)**
  - [ ] Frustum Culling (Compute)
  - [ ] Hi-Z Occlusion Culling

**Upon Completion:** 100K particles + Physics interaction

---

### Phase 8: Bindless & GPU Driven
**Goal:** Modern rendering techniques

- [ ] **Bindless Resources**
  - [ ] Descriptor Indexing (SM 6.6+)
  - [ ] Unbounded Descriptor Arrays
  - [ ] Material ID → Descriptor Index mapping
- [ ] **GPU Driven Rendering**
  - [ ] ExecuteIndirect / DrawIndirect
  - [ ] GPU-generated Draw Calls
  - [ ] Mesh Shaders (optional, DX12 Ultimate)
- [ ] **Resource Management**
  - [ ] GPU memory allocator
  - [ ] Async Upload tuning

**Upon Completion:** 10K unique meshes + Physics rendering

---

### Phase 9: Job System Expansion (if Phase 3.5 was skipped)
**Goal:** CPU parallelization

- [ ] Job System implementation (see Phase 3.5)
- [ ] ECS System parallelization
- [ ] PhysicsSystem parallelization
- [ ] Render thread separation

---

### Phase 10: AI & Gameplay Systems
**Goal:** Game logic framework

- [ ] **Navigation**
  - [ ] NavMesh generation (Recast)
  - [ ] A* Pathfinding
  - [ ] Path Following
- [ ] **Behavior Tree**
  - [ ] Executor (Selector, Sequence, Action)
  - [ ] Blackboard (shared data)
- [ ] **Gameplay Framework**
  - [ ] Event System (type-safe)
  - [ ] Game State management

**Upon Completion:** AI agents chase player, avoid Physics objects

---

### Phase 11: Audio & Tools
**Goal:** Enhanced completeness

- [ ] **Audio System**
  - [ ] XAudio2 integration
  - [ ] 3D Spatialization
  - [ ] Sound effects on collision
- [ ] **Debug Tools Enhancement**
  - [ ] ECS Inspector (add/remove components)
  - [ ] Profiler (CPU/GPU Timeline)
  - [ ] Console (command system)

**Upon Completion:** Complete game engine experience

---

### Phase 12: Demo Game & Polish
**Goal:** Portfolio completion

- [ ] **Demo Game (Simple TPS)**
  - [ ] Physics Character Controller
  - [ ] Shooting system (Raycast + Force)
  - [ ] Enemy AI (NavMesh + Physics)
  - [ ] UI (health, ammo)
- [ ] **Optimization**
  - [ ] CPU/GPU Profiling
  - [ ] Physics performance optimization
  - [ ] LOD (Level of Detail)
- [ ] **Portfolio Documentation**
  - [ ] Technical blog (ECS + Physics integration)
  - [ ] Video recording (gameplay + technical explanation)
  - [ ] GitHub README update

**Final Result:** Playable TPS demo, ready for portfolio submission

---

## Backlog (On-Demand)

Features to be added selectively as needed:

- [ ] **Deferred Rendering** - Compare/switch with Forward+
- [ ] **Tessellation & Geometry Shaders** - Adaptive terrain
- [ ] **Standalone Editor** - Independent editor beyond ImGui tools
- [ ] **Scripting** - Lua/Python bindings
- [ ] **Procedural Terrain** - Heightmap/Voxel-based
- [ ] **Raytracing (DXR)** - DX12 Ultimate-based (requires RTX GPU)
- [ ] **Advanced Animation** - Blend trees, IK
- [ ] **Volumetric Effects** - Fog, Clouds
- [ ] **Network Replication** - ECS component replication, multiplayer

---

## Key Features

### Architecture
- **Modular Design**: Clear separation of concerns and dependency management
- **ECS Architecture**: Data-oriented design (planned)
- **Framework Pattern**: Automatic Application lifecycle management
- **What/How Separation**: Scene (logic) and Renderer (implementation) separation

### Rendering
- **Multi-texture**: PBR workflow ready
- **Efficient Resource Management**: Centralized ResourceManager
- **Frame Buffering**: Triple buffering for CPU-GPU parallelism

### Code Quality
- **Modern C++20**: Smart pointers, lambdas, constexpr utilization
- **Consistent Coding Convention**: Documented style guide
- **Comprehensive Logging**: Category-based debugging support

## Documentation

- [Architecture Guide](./Docs/Architecture.md) - Engine design and structure
- [Coding Convention](./Docs/CodingConvention.md) - Coding rules
- [Development Log](./Docs/DevelopmentLog.md) - Development history

## License

This project is released under the [MIT License](./LICENSE).

## Developer

**rniman**
- GitHub: [@rniman](https://github.com/rniman)
- Email: shinmg00@gmail.com

## Acknowledgments

- [DirectX 12 Official Documentation](https://docs.microsoft.com/en-us/windows/win32/direct3d12/)
- [Learn OpenGL](https://learnopengl.com/) - Graphics concept learning
- [Game Engine Architecture by Jason Gregory](https://www.gameenginebook.com/)
- [3D Game Programming with DirectX 12 by Frank Luna](http://www.d3dcoder.net/)

---

**Last Updated:** 2025-11-06  
**Roadmap Version:** v3.2 (Portfolio-focused)

⭐ If this project helped you, please give it a star!