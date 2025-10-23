# DevMiniEngine

**[한국어](./README.md)** | **English**

**DirectX 12-based Learning Mini Game Engine**

## Project Overview

DevMiniEngine is a personal learning and portfolio game engine project based on DirectX 12. It was created to learn and experiment with ECS (Entity Component System) architecture and modern C++ design patterns.

### Main Goals

- **ECS Architecture Learning**: Implementation of data-oriented design and entity-component systems
- **Modern Graphics Technology**: Building modern rendering pipelines using DirectX 12
- **Game Engine Fundamentals**: Implementation of core engine systems including physics, AI, and terrain generation
- **Portfolio Creation**: Production-level code quality and documentation

## Implementation Status

### Completed Systems

**Phase 1: Foundation Systems (100% Complete)**

**Core System**
- Memory Management System
  - LinearAllocator: O(1) bump pointer allocation
  - PoolAllocator: O(1) fixed-size object pooling
  - StackAllocator: LIFO allocation using markers
  - Test Coverage: 01_MemoryTest, 02_PoolAllocatorTest, 03_StackAllocatorTest

- Logging System
  - Multi-output Sink architecture (Console, File)
  - Thread-safe logging with Mutex
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
  - Double buffering for frame accuracy
- Test Coverage: 06_WindowTest, 07_InputTest

**Phase 2: DirectX 12 Graphics (25% Complete)**

**DirectX 12 Initialization (Complete)**
- DX12Device: Device and Factory creation, GPU adapter selection
- DX12CommandQueue: Command Queue creation and GPU synchronization
- DX12SwapChain: Double-buffered SwapChain creation
- DX12DescriptorHeap: RTV Descriptor Heap management
- DX12CommandContext: Command Allocator/List management and Resource Barrier
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

### Graphics (Partial Implementation)
- DirectX 12-based rendering system (initialization complete)
  - Device and Command Queue creation
  - SwapChain and Descriptor Heap management
  - Command List recording and execution
  - Resource state transitions via Resource Barrier
- RHI (Render Hardware Interface) abstraction layer (planned)
- PBR (Physically Based Rendering) support (planned)
- High-level rendering system (Material, Mesh, Camera) (planned)

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
│   ├── include/                     # Public headers for all modules
│   │   ├── Core/                    # Core headers
│   │   │   ├── Memory/              # Memory management headers
│   │   │   └── Logging/             # Logging system headers
│   │   ├── Math/                    # Math library headers
│   │   ├── Platform/                # Platform layer headers
│   │   │   ├── Windows/             # Win32 headers
│   │   │   └── Input/               # Input system headers
│   │   ├── Graphics/                # Graphics headers
│   │   │   ├── DX12/                # DirectX 12 headers
│   │   │   ├── RHI/                 # RHI headers (planned)
│   │   │   └── Renderer/            # High-level rendering headers (planned)
│   │   ├── ECS/                     # ECS headers (planned)
│   │   ├── Physics/                 # Physics engine headers (planned)
│   │   ├── AI/                      # Game AI headers (planned)
│   │   └── Scene/                   # Scene management headers (planned)
│   │
│   ├── src/                         # Implementation for all modules
│   │   ├── Core/                    # Core implementation
│   │   │   ├── Memory/              # Memory management implementation
│   │   │   └── Logging/             # Logging system implementation
│   │   ├── Math/                    # Math library implementation
│   │   ├── Platform/                # Platform layer implementation
│   │   │   ├── Windows/             # Win32 implementation
│   │   │   └── Input/               # Input system implementation
│   │   ├── Graphics/                # Graphics implementation
│   │   │   ├── DX12/                # DirectX 12 implementation
│   │   │   ├── RHI/                 # RHI implementation (planned)
│   │   │   └── Renderer/            # High-level rendering implementation (planned)
│   │   ├── ECS/                     # ECS implementation (planned)
│   │   ├── Physics/                 # Physics engine implementation (planned)
│   │   ├── AI/                      # Game AI implementation (planned)
│   │   └── Scene/                   # Scene management implementation (planned)
│   │
│   ├── Core/                        # Core module project
│   │   └── Core.vcxproj
│   ├── Math/                        # Math module project
│   │   └── Math.vcxproj
│   ├── Platform/                    # Platform module project
│   │   └── Platform.vcxproj
│   ├── Graphics/                    # Graphics module project
│   │   └── Graphics.vcxproj
│   │
│   ├── Sandbox/                     # Sandbox game project
│   │   ├── Assets/                  # Game assets
│   │   ├── Source/                  # Game source
│   │   └── Sandbox.vcxproj
│   │
│   └── DemoGame/                    # Demo game project (planned)
│       ├── Assets/
│       ├── Source/
│       └── DemoGame.vcxproj
│
├── Samples/                         # Learning sample projects
│   ├── 01_MemoryTest/               # LinearAllocator test
│   ├── 02_PoolAllocatorTest/        # PoolAllocator test
│   ├── 03_StackAllocatorTest/       # StackAllocator test
│   ├── 04_MathTest/                 # Math library test
│   ├── 05_LoggingTest/              # Logging system test
│   ├── 06_WindowTest/               # Window creation test
│   ├── 07_InputTest/                # Input system test
│   └── 08_DX12Init/                 # DirectX 12 initialization test
│
├── Docs/                            # Documentation
│   ├── Sample
│   │   └── 08_DX12Init.md           # 08_DX12Init Sample Documentation
│   │
│   ├── Architecture.md              # Detailed architecture guide
│   ├── CodingConvention.md          # Coding standards
│   └── DevelopmentLog.md            # Development history
│
├── bin/                             # Build output
│   ├── Debug/
│   └── Release/
│
└── intermediate/                    # Intermediate build files
```

For detailed architecture design and module dependencies, see the [Architecture Document](./Docs/Architecture.md).

## Build and Run

### Requirements

- **OS**: Windows 10/11 (64-bit)
- **IDE**: Visual Studio 2022 or later
- **SDK**: Windows 10 SDK (10.0.19041.0 or later)
- **DirectX**: DirectX 12 capable GPU

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
   - `Ctrl + Shift + B` or from menu `Build > Build Solution`

### Run Samples
```bash
# Example: Run memory allocator test
bin/Debug/01_MemoryTest.exe

# Example: Run DirectX 12 initialization test
bin/Debug/08_DX12Init.exe
```

## Sample Projects

| Sample | Status | Description | Learning Content |
|--------|--------|-------------|------------------|
| 01_MemoryTest | Complete | LinearAllocator test | Custom memory allocation |
| 02_PoolAllocatorTest | Complete | PoolAllocator test | Object pooling |
| 03_StackAllocatorTest | Complete | StackAllocator test | Scope allocation |
| 04_MathTest | Complete | Math library test | SIMD vectors, matrices |
| 05_LoggingTest | Complete | Logging system test | Structured logging |
| 06_WindowTest | Complete | Window creation | Win32 API, events |
| 07_InputTest | Complete | Input handling | Keyboard, mouse tracking |
| 08_DX12Init | Complete | DirectX 12 initialization | Device, SwapChain, Clear |
| 09_HelloTriangle | Planned | Basic rendering | Vertex Buffer, Shader, PSO |

## Tech Stack

### Core Technologies
- **C++20**: Modern C++ standard
- **DirectX 12**: Low-level graphics API
- **Win32 API**: Windows platform interface
- **DirectXMath**: SIMD-optimized math library

### Development Tools
- **Visual Studio 2022**: Primary development environment
- **Git/GitHub**: Version control
- **RenderDoc**: Graphics debugging (planned)
- **PIX**: DirectX 12 profiling (planned)

## Documentation

- [Architecture Guide](./Docs/Architecture.md) - Complete engine design and structure
- [Coding Convention](./Docs/CodingConvention.md) - Project coding rules
- [Development Log](./Docs/DevelopmentLog.md) - Detailed development history

## Roadmap

### Phase 1: Foundation (100% Complete)
- [x] Project structure
- [x] Core system
  - [x] Memory allocators (Linear, Pool, Stack)
  - [x] Logging system (Console, File sinks)
  - [x] Assertion macros
- [x] Math library (Vector, Matrix, Quaternion with SIMD)
- [x] Platform layer (Window, Input)

### Phase 2: Graphics (25% Complete)
- [x] DirectX 12 initialization
- [ ] Basic rendering pipeline (Vertex Buffer, Shader, PSO)
- [ ] Mesh and texture loading
- [ ] Camera system

### Phase 3: Advanced ECS (Not Started)
- [ ] ECS core implementation
- [ ] Component queries
- [ ] System dependencies
- [ ] Event system

### Phase 4: Gameplay Systems (Not Started)
- [ ] Physics engine integration
- [ ] Collision detection
- [ ] AI pathfinding (A*)
- [ ] Terrain generation

### Phase 5: Future Plans
- [ ] Threading system (Job system, thread pool)
- [ ] PBR materials
- [ ] Shadow mapping
- [ ] Post-processing

## Code Quality

- Warning Level: 4 (Maximum)
- Active Warnings: 0
- C++ Standard: C++20
- Code Style: Enforced through CodingConvention.md

## Contributing

This project is for personal learning purposes, but feedback and suggestions are always welcome!

### How to Provide Feedback
- **Issues**: Bug reports or feature suggestions
- **Discussions**: Technical questions or idea sharing

## License

This project is open-sourced under the [MIT License](./LICENSE).

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
- [3D Game Programming with DirectX 12 by Frank Luna](http://www.d3dcoder.net/)

---

Last Updated: 2025-10-15

⭐ If this project helps you, please consider giving it a star!