# DevMiniEngine

**[한국어](./README.md)** | **English**

**A Learning-Focused Mini Game Engine Based on DirectX 12**

## Project Overview

DevMiniEngine is a personal learning and portfolio project built on DirectX 12. It aims to explore and implement ECS (Entity Component System) architecture and modern C++ design patterns.

### Main Goals

- **ECS Architecture Learning**: Implementing data-oriented design and entity-component systems
- **Modern Graphics Technology**: Building contemporary rendering pipelines with DirectX 12
- **Game Engine Fundamentals**: Implementing core engine systems including physics, AI, and terrain generation
- **Portfolio Development**: Maintaining production-quality code and comprehensive documentation

## Implementation Status

### Completed Systems

**Phase 1: Foundation Systems (100% Complete)**

**Core Systems**
- Memory Management System
  - LinearAllocator: O(1) bump-pointer allocation
  - PoolAllocator: O(1) fixed-size object pooling
  - StackAllocator: LIFO allocation with markers
  - Test Coverage: 01_MemoryTest, 02_PoolAllocatorTest, 03_StackAllocatorTest

- Logging System
  - Multi-output sink architecture (Console, File)
  - Thread-safe logging with mutex protection
  - Category-based filtering (Core, Graphics, Physics, AI, Audio, Input, Memory)
  - Color-coded console output
  - Test Coverage: 05_LoggingTest

**Math Library**
- SIMD-optimized vector/matrix operations (DirectXMath wrapper)
- Vector2, Vector3, Vector4
- Matrix3x3, Matrix4x4 (row-major)
- Quaternion operations
- Transform utilities (Translation, Rotation, Scaling)
- Camera matrices (LookAt, Perspective, Orthographic)
- Test Coverage: 04_MathTest

**Platform Layer**
- Win32 window creation and management
- Event handling system
- Input system
  - Keyboard state tracking (pressed/held/released)
  - Mouse state tracking (buttons, position, wheel)
  - Double buffering for frame-accurate input
- Test Coverage: 06_WindowTest, 07_InputTest

**Phase 2: DirectX 12 Graphics (25% Complete)**

**DirectX 12 Initialization (Complete)**
- DX12Device: Device and Factory creation, GPU adapter selection
- DX12CommandQueue: Command Queue creation and GPU synchronization
- DX12SwapChain: Double-buffered SwapChain creation
- DX12DescriptorHeap: RTV Descriptor Heap management
- DX12CommandContext: Command Allocator/List management and Resource Barriers
- First rendering success (Clear screen)
- Test Coverage: 08_DX12Init

### In Progress

**Phase 2: DirectX 12 Graphics**
- Next Priority: Triangle rendering (Vertex Buffer, Shader compilation, PSO)

### Project Statistics

- Total Lines of Code: ~3,000
- Implemented Modules: 5/12 subsystems
- Test Coverage: 8 sample projects
- Compiler Warnings: 0 (Level 4)

## Key Features

### Graphics (Partially Implemented)
- DirectX 12-based rendering system (initialization complete)
  - Device and Command Queue creation
  - SwapChain and Descriptor Heap management
  - Command List recording and execution
  - Resource state transitions via Resource Barriers
- RHI (Render Hardware Interface) abstraction layer (Planned)
- PBR (Physically Based Rendering) support (Planned)
- High-level rendering systems (Material, Mesh, Camera) (Planned)

### ECS Architecture (Planned)
- Entity-based game object management
- Component-based data structures
- System-based logic processing
- Efficient memory layout

### Physics Engine (Planned)
- Basic collision detection
- Rigid body dynamics
- Physics World simulation

### Game AI (Planned)
- A* pathfinding algorithm
- NavMesh-based navigation

## Project Structure
```
DevMiniEngine/
├── Engine/                          # Engine Core
│   ├── Core/                        # Base Systems (Complete)
│   │   ├── Memory/                  # Custom Allocators
│   │   └── Logging/                 # Multi-output Logging
│   ├── Math/                        # Math Library (Complete)
│   ├── Platform/                    # Platform Layer (Complete)
│   │   ├── Windows/                 # Win32 Implementation
│   │   └── Input/                   # Input System
│   ├── Graphics/                    # Rendering System (Partial)
│   │   ├── RHI/                     # Rendering Abstraction Layer (Planned)
│   │   ├── DX12/                    # DirectX 12 Implementation (Init Complete)
│   │   └── Renderer/                # High-level Rendering (Planned)
│   ├── ECS/                         # Entity Component System (Planned)
│   ├── Physics/                     # Physics Engine (Planned)
│   ├── AI/                          # Game AI (Planned)
│   └── Scene/                       # Scene Management (Planned)
│
├── Samples/                         # Learning Sample Projects
│   ├── 01_MemoryTest/              # LinearAllocator Test
│   ├── 02_PoolAllocatorTest/       # PoolAllocator Test
│   ├── 03_StackAllocatorTest/      # StackAllocator Test
│   ├── 04_MathTest/                # Math Library Test
│   ├── 05_LoggingTest/             # Logging System Test
│   ├── 06_WindowTest/              # Window Creation Test
│   ├── 07_InputTest/               # Input System Test
│   └── 08_DX12Init/                # DirectX 12 Initialization Test
│
├── Docs/                            # Documentation
│   ├── Architecture.md              # Detailed Architecture Guide
│   ├── CodingConvention.md          # Coding Standards
│   └── DevelopmentLog.md            # Development History
│
├── bin/                             # Build Output
│   ├── Debug/
│   └── Release/
│
└── intermediate/                    # Intermediate Build Files
```

For detailed information on architecture design and module dependencies, see [Architecture Documentation](./Docs/Architecture.md).

## Build and Run

### Requirements

- **OS**: Windows 10/11 (64-bit)
- **IDE**: Visual Studio 2022 or later
- **SDK**: Windows 10 SDK (10.0.19041.0 or later)
- **DirectX**: DirectX 12 compatible GPU

### Build Instructions

1. **Clone Repository**
```bash
git clone https://github.com/rniman/DevMiniEngine.git
cd DevMiniEngine
```

2. **Open Solution**
```bash
# Open with Visual Studio 2022
DevMiniEngine.sln
```

3. **Select Build Configuration**
   - `Debug` - For debugging (optimizations disabled)
   - `Release` - For release (optimizations enabled)

4. **Build**
   - `Ctrl + Shift + B` or `Build > Build Solution` from menu

### Running Samples
```bash
# Example: Run memory allocator test
bin/Debug/01_MemoryTest.exe

# Example: Run DirectX 12 initialization test
bin/Debug/08_DX12Init.exe
```

## Sample Projects

| Sample | Status | Description | Learning Topics |
|--------|--------|-------------|-----------------|
| 01_MemoryTest | Complete | LinearAllocator Test | Custom Memory Allocation |
| 02_PoolAllocatorTest | Complete | PoolAllocator Test | Object Pooling |
| 03_StackAllocatorTest | Complete | StackAllocator Test | Scope Allocation |
| 04_MathTest | Complete | Math Library Test | SIMD Vectors, Matrices |
| 05_LoggingTest | Complete | Logging System Test | Structured Logging |
| 06_WindowTest | Complete | Window Creation | Win32 API, Events |
| 07_InputTest | Complete | Input Handling | Keyboard, Mouse Tracking |
| 08_DX12Init | Complete | DirectX 12 Init | Device, SwapChain, Clear |
| 09_HelloTriangle | Planned | Basic Rendering | Vertex Buffer, Shader, PSO |

## Tech Stack

### Core Technologies
- **C++20**: Latest C++ standard
- **DirectX 12**: Low-level graphics API
- **Win32 API**: Windows platform interface
- **DirectXMath**: SIMD-optimized math library

### Development Tools
- **Visual Studio 2022**: Primary development environment
- **Git/GitHub**: Version control
- **RenderDoc**: Graphics debugging (Planned)
- **PIX**: DirectX 12 profiling (Planned)

## Documentation

- [Architecture Guide](./Docs/Architecture.md) - Complete engine design and structure
- [Coding Convention](./Docs/CodingConvention.md) - Project coding standards
- [Development Log](./Docs/DevelopmentLog.md) - Detailed development history

## Roadmap

### Phase 1: Foundation Systems (100% Complete)
- [x] Project Structure
- [x] Core Systems
  - [x] Memory Allocators (Linear, Pool, Stack)
  - [x] Logging System (Console, File sinks)
  - [x] Assertion Macros
- [x] Math Library (Vector, Matrix, Quaternion with SIMD)
- [x] Platform Layer (Window, Input)

### Phase 2: Graphics (25% Complete)
- [x] DirectX 12 Initialization
- [ ] Basic Rendering Pipeline (Vertex Buffer, Shader, PSO)
- [ ] Mesh and Texture Loading
- [ ] Camera System

### Phase 3: Advanced ECS (Not Started)
- [ ] ECS Core Implementation
- [ ] Component Queries
- [ ] System Dependencies
- [ ] Event System

### Phase 4: Gameplay Systems (Not Started)
- [ ] Physics Engine Integration
- [ ] Collision Detection
- [ ] AI Pathfinding (A*)
- [ ] Terrain Generation

### Phase 5: Future Plans
- [ ] Threading System (Job system, thread pool)
- [ ] PBR Materials
- [ ] Shadow Mapping
- [ ] Post-processing

## Code Quality

- Warning Level: 4 (Highest)
- Active Warnings: 0
- C++ Standard: C++20
- Code Style: Enforced via CodingConvention.md

## Contributing

This is a personal learning project, but feedback and suggestions are always welcome!

### Feedback Methods
- **Issues**: Bug reports or feature suggestions
- **Discussions**: Technical questions or idea sharing

## License

This project is licensed under the [MIT License](./LICENSE).

## Developer

**rniman**
- GitHub: [@rniman](https://github.com/rniman)
- Email: shinmg00@gmail.com
- Blog: [Tech Blog](https://rniman.github.io/)

## Acknowledgments

- [DirectX 12 Official Documentation](https://docs.microsoft.com/en-us/windows/win32/direct3d12/)
- [Learn OpenGL](https://learnopengl.com/) - Graphics concepts
- [Game Programming Patterns](https://gameprogrammingpatterns.com/) - Design patterns
- [Game Engine Architecture by Jason Gregory](https://www.gameenginebook.com/)

---

Last Updated: 2025-10-15

⭐ If this project helps you, please consider giving it a star!