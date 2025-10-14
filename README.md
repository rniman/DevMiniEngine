# DevMiniEngine

**[한국어](./README.ko.md)** | **English**

**A Learning-Focused Mini Game Engine Based on DirectX 12**

## Project Overview

DevMiniEngine is a personal learning and portfolio project built on DirectX 12. It aims to explore and implement ECS (Entity Component System) architecture and modern C++ design patterns.

### Main Goals

- **ECS Architecture Learning**: Implementing data-oriented design and entity-component systems
- **Modern Graphics Technology**: Building contemporary rendering pipelines with DirectX 12
- **Game Engine Fundamentals**: Implementing core engine systems including physics, AI, and terrain generation
- **Portfolio Development**: Maintaining production-quality code and comprehensive documentation

## Key Features

### Graphics
- DirectX 12-based rendering system
- RHI (Render Hardware Interface) abstraction layer
- PBR (Physically Based Rendering) support planned
- High-level rendering systems (Material, Mesh, Camera)

### ECS Architecture
- Entity-based game object management
- Component-based data structures
- System-based logic processing
- Efficient memory layout

### Terrain System
- Procedural terrain generation
- Height map-based rendering
- LOD (Level of Detail) support planned

### Physics Engine
- Basic collision detection
- Rigid body dynamics
- Physics world simulation

### Game AI
- A* pathfinding algorithm
- NavMesh-based navigation
- Behavior tree support planned

## Project Structure

```
DevMiniEngine/
├── Engine/                          # Engine Core
│   ├── Core/                        # Base Systems (Memory, Threading, Logging)
│   ├── Math/                        # Math Library
│   ├── ECS/                         # Entity Component System
│   ├── Graphics/                    # Rendering System
│   │   ├── RHI/                     # Rendering Abstraction Layer
│   │   ├── DX12/                    # DirectX 12 Implementation
│   │   └── Renderer/                # High-Level Rendering
│   ├── Physics/                     # Physics Engine
│   ├── Terrain/                     # Terrain System
│   ├── AI/                          # Game AI
│   ├── Scene/                       # Scene Management
│   └── Input/                       # Input Handling
│
├── Platform/                        # Platform Layer (Windows)
├── Samples/                         # Learning Sample Projects
├── Tools/                           # Development Tools
├── Assets/                          # Resources (Shaders, Textures, Models)
├── Tests/                           # Unit Tests
├── Docs/                            # Documentation
│
├── bin/                             # Build Outputs (exe, lib, dll)
│   ├── Debug/
│   └── Release/
│
└── intermediate/                    # Intermediate Build Files (obj, pch)
   ├── Debug/
   └── Release/
```

For detailed **architecture design and module dependencies**, see [Architecture Documentation](./Docs/Architecture.md).

## Build & Run (Planned)

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
   - Press `Ctrl + Shift + B` or select `Build > Build Solution` from menu

### Running Samples

```bash
# Set sample project as startup project and run
bin/Debug/01_HelloTriangle.exe
```

## Learning Samples (Planned)

| Sample | Description | Learning Topics |
|--------|-------------|-----------------|
| **01_MemoryTest** | LinearAllocator test | Memory allocator basics |
| **02_PoolAllocatorTest** | PoolAllocator test | Fixed-size object pool |
| **03_StackAllocatorTest** | StackAllocator test | Stack-based memory management |
| **04_MathTest** | Math library test | Vectors, matrices, quaternions |
| **05_LoggingTest** | Logging system test | Structured logging, multiple outputs |
| **06_WindowTest** | Window creation | Win32 API, event handling |
| **07_DX12Init** | DirectX 12 initialization | Device, Command Queue creation |
...
| **XX_HelloTriangle** | Basic triangle rendering | DirectX 12 basics, rendering pipeline |
| **XX_BasicLighting** | Lighting system | Phong shading, normal mapping |
| **XX_TextureMapping** | Texture application | Texture sampling, UV mapping |
| **XX_ECS_Basics** | ECS system demo | Entity, Component, System concepts |
| **XX_Physics_Demo** | Physics simulation | Collision detection, rigid body dynamics |
| **XX_Terrain_Demo** | Terrain generation & rendering | Procedural generation, height maps |
| **XX_AI_Pathfinding** | AI pathfinding | A* algorithm, NavMesh |

## Tech Stack

### Core Technologies
- **C++20**: Modern C++ standard
- **DirectX 12**: Low-level graphics API
- **Win32 API**: Windows platform interface

### External Libraries
- **DirectXTK12**: DirectX utility library
- **ImGui**: Debug UI
- **spdlog**: High-performance logging library

### Development Tools
- **Visual Studio 2022**: Primary development environment
- **Git/GitHub**: Version control
- **RenderDoc**: Graphics debugging

## Documentation

- [Architecture Guide](./Docs/Architecture.md) - Engine design philosophy and structure
- [Coding Conventions](./Docs/CodingConvention.md) - Project coding rules
- [Development Log](./Docs/DevelopmentLog.md) - Development progress tracking
- [Technical Notes](./Docs/TechnicalNotes/) - Detailed implementation notes

## Learning Resources (Planned)

### ECS Architecture
- [ECS Basics](./Docs/TechnicalNotes/ECS_Basics.md)
- [Component Design Patterns](./Docs/TechnicalNotes/Component_Design.md)

### Graphics
- [Getting Started with DirectX 12](./Docs/TechnicalNotes/DX12_Basics.md)
- [Rendering Pipeline](./Docs/TechnicalNotes/Render_Pipeline.md)

### Physics Simulation
- [Collision Detection Algorithms](./Docs/TechnicalNotes/Collision_Detection.md)
- [Rigid Body Physics Basics](./Docs/TechnicalNotes/Rigid_Body_Physics.md)

## Development Scripts (Planned)

```bash
# Clean project
Scripts/clean.bat

# Full build
Scripts/build.bat

# Regenerate project files (if needed)
Scripts/generate_project.bat
```

## Testing (Planned)

```bash
# Run unit tests
bin/Debug/CoreTests.exe
bin/Debug/ECSTests.exe
bin/Debug/MathTests.exe
```

## Core Coding Conventions

### Naming
- **Classes/Functions**: `PascalCase`
- **Local Variables**: `camelCase`
- **Member Variables**: `m` prefix + `PascalCase` (e.g., `mFrameIndex`)
- **Global Variables**: `g` prefix + `PascalCase`

### Style
- Pointer attached to type: `ID3D12Device* device`
- Use `nullptr` (~~`NULL`~~ prohibited)
- Always use braces `{}`
- Prefer `=` for initialization

### Memory Management
- Prefer smart pointers: `std::unique_ptr`, `std::shared_ptr`
- DirectX COM objects: `Microsoft::WRL::ComPtr`
- Read-only parameters: `const T&`

For details, see [Coding Conventions Documentation](./Docs/CodingConvention.md).

## Roadmap

### Phase 0: Project Setup (Current)

### Phase 1: Foundation Systems
- [ ] DirectX 12 initialization
- [ ] Basic rendering pipeline
- [ ] ECS core system implementation
- [ ] Math library completion

### Phase 2: Rendering Extensions
- [ ] PBR material system
- [ ] Shadow mapping
- [ ] Post-processing

### Phase 3: Gameplay Systems
- [ ] Physics engine integration
- [ ] AI system implementation
- [ ] Terrain generation system

### Phase 4: Optimization & Tools
- [ ] Multi-threaded rendering
- [ ] Resource management optimization
- [ ] Editor prototype

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
- [Game Programming Patterns](https://gameprogrammingpatterns.com/) - Design pattern references

---

⭐ If this project helps you, please consider giving it a star!