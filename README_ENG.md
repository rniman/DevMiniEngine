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

### Completed Systems (Phase 1: Foundation - 75% Complete)

**Core Systems**
- Memory Management System (Fully Implemented)
  - LinearAllocator: O(1) bump-pointer allocation
  - PoolAllocator: O(1) fixed-size object pooling
  - StackAllocator: LIFO allocation with markers
  - Test Coverage: 01_MemoryTest, 02_PoolAllocatorTest, 03_StackAllocatorTest

- Logging System (Fully Implemented)
  - Multi-output sink architecture (Console, File)
  - Thread-safe logging with mutex protection
  - Category-based filtering (Core, Graphics, Physics, AI, Audio, Input, Memory)
  - Color-coded console output
  - Test Coverage: 05_LoggingTest

**Math Library (Fully Implemented)**
- SIMD-optimized vector/matrix operations (DirectXMath wrapper)
- Vector2, Vector3, Vector4
- Matrix3x3, Matrix4x4 (row-major)
- Quaternion operations
- Transform utilities (Translation, Rotation, Scaling)
- Camera matrices (LookAt, Perspective, Orthographic)
- Test Coverage: 04_MathTest

**Platform Layer (Fully Implemented)**
- Win32 window creation and management
- Event handling system
- Input system
  - Keyboard state tracking (pressed/held/released)
  - Mouse state tracking (buttons, position, wheel)
  - Double buffering for frame-accurate input
- Test Coverage: 06_WindowTest, 07_InputTest

### In Progress

**Phase 4: DirectX 12 Graphics**
- Next Priority: Device initialization and basic rendering

### Project Statistics

- Total Lines of Code: ~2,200
- Implemented Modules: 4/12 subsystems
- Test Coverage: 7 sample projects
- Compiler Warnings: 0 (Level 4)

## Key Features

### Graphics (Planned)
- DirectX 12-based rendering system
- RHI (Render Hardware Interface) abstraction layer
- PBR (Physically Based Rendering) support
- High-level rendering systems (Material, Mesh, Camera)

### ECS Architecture (Planned)
- Entity-based game object management
- Component-based data structures
- System-based logic processing
- Efficient memory layout

### Physics Engine (Planned)
- Basic collision detection
- Rigid body dynamics
- Physics world simulation

### Game AI (Planned)
- A* pathfinding algorithm
- NavMesh-based navigation

## Project Structure

```
DevMiniEngine/
├── Engine/                          # Engine Core
│   ├── Core/                        # Base Systems (Implemented)
│   │   ├── Memory/                  # Custom allocators
│   │   └── Logging/                 # Multi-output logging
│   ├── Math/                        # Math Library (Implemented)
│   ├── Platform/                    # Platform Layer (Implemented)
│   │   ├── Windows/                 # Win32 implementation
│   │   └── Input/                   # Input system
│   ├── ECS/                         # Entity Component System (Planned)
│   ├── Graphics/                    # Rendering System (Planned)
│   │   ├── RHI/                     # Rendering Abstraction Layer
│   │   ├── DX12/                    # DirectX 12 Implementation
│   │   └── Renderer/                # High-Level Rendering
│   ├── Physics/                     # Physics Engine (Planned)
│   ├── AI/                          # Game AI (Planned)
│   └── Scene/                       # Scene Management (Planned)
│
├── Samples/                         # Learning Sample Projects
│   ├── 01_MemoryTest/              # LinearAllocator test
│   ├── 02_PoolAllocatorTest/       # PoolAllocator test
│   ├── 03_StackAllocatorTest/      # StackAllocator test
│   ├── 04_MathTest/                # Math library test
│   ├── 05_LoggingTest/             # Logging system test
│   ├── 06_WindowTest/              # Window creation test
│   └── 07_InputTest/               # Input system test
│
├── Docs/                            # Documentation
│   ├── Architecture.md              # Detailed architecture guide
│   ├── CodingConvention.md          # Coding standards
│   └── DevelopmentLog.md            # Development progress
│
├── bin/                             # Build Outputs
│   ├── Debug/
│   └── Release/
│
└── intermediate/                    # Intermediate Build Files
```

For detailed architecture design and module dependencies, see [Architecture Documentation](./Docs/Architecture.md).

## Build & Run

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
   - Press `Ctrl + Shift + B` or select `Build > Build Solution`

### Running Samples

```bash
# Example: Run memory allocator test
bin/Debug/01_MemoryTest.exe

# Example: Run input system test
bin/Debug/07_InputTest.exe
```

## Sample Projects

| Sample | Status | Description | Learning Topics |
|--------|--------|-------------|-----------------|
| 01_MemoryTest | Complete | LinearAllocator test | Custom memory allocation |
| 02_PoolAllocatorTest | Complete | PoolAllocator test | Object pooling |
| 03_StackAllocatorTest | Complete | StackAllocator test | Scoped allocations |
| 04_MathTest | Complete | Math library test | SIMD vectors, matrices |
| 05_LoggingTest | Complete | Logging system test | Structured logging |
| 06_WindowTest | Complete | Window creation | Win32 API, events |
| 07_InputTest | Complete | Input handling | Keyboard, mouse tracking |
| 08_DX12Init | Planned | DirectX 12 init | Device, command queue |
| 09_HelloTriangle | Planned | Basic rendering | DirectX 12 pipeline |

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

## Documentation

- [Architecture Guide](./Docs/Architecture.md) - Complete engine design and structure
- [Coding Conventions](./Docs/CodingConvention.md) - Project coding standards
- [Development Log](./Docs/DevelopmentLog.md) - Detailed development history

## Roadmap

### Phase 1: Foundation Systems (75% Complete)
- [x] Project structure
- [x] Core systems
  - [x] Memory allocators (Linear, Pool, Stack)
  - [x] Logging system (Console, File sinks)
  - [x] Assertion macros
- [x] Math library (Vector, Matrix, Quaternion with SIMD)
- [x] Platform layer (Window, Input)
- [ ] Threading system (Job system, thread pool)

### Phase 2: Graphics (Not Started)
- [ ] DirectX 12 initialization
- [ ] Basic rendering pipeline
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

## Code Quality

- Warning Level: 4 (highest)
- Active Warnings: 0
- C++ Standard: C++20
- Code Style: Enforced via CodingConvention.md

## Contributing

This is a personal learning project, but feedback and suggestions are welcome!

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

Last Updated: 2025-10-14

⭐ If this project helps you, please consider giving it a star!