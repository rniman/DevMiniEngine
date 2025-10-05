# Development Log

---

## 2025-10-05 - Core Project Setup & Basic Types

### Tasks
- [x] Create Core project (Static Library)
- [x] Configure project settings
  - Output: `bin\$(Configuration)\`, Intermediate: `intermediate\$(Configuration)\$(ProjectName)\`
  - C++20, Additional includes: `$(ProjectDir)include\`
  - Warning Level 4, Runtime Library: /MDd (Debug), /MD (Release)
- [x] Implement Types.h
  - Basic type aliases (uint8, uint16, uint32, int8, int16, int32, float32, float64)
  - Smart pointer aliases (UniquePtr, SharedPtr, WeakPtr)
  - Container aliases (Vector, Array, String)
  - Helper functions (MakeUnique, MakeShared)
- [x] Add Core.cpp for module initialization
- [x] Build verification successful

### Decisions
- **Simplified output paths**: No platform suffix (x64 only target)
- **Core namespace**: All type aliases in `Core::` to avoid std conflicts
- **Smart pointers included**: WeakPtr added for circular reference handling

### Issues Encountered
- C/C++ property tab not visible until .cpp file added
- Initially added redundant `_DEBUG`/`NDEBUG` preprocessor definitions (auto-defined by VS)

### Notes
- First successful build: `bin/Debug/Core.lib`
- ~50 lines of code, 0 warnings, < 1s build time
- Types.h provides foundation for all future modules

### Next Steps
- [ ] Implement basic Logging system (Logger.h/cpp)
- [ ] Add Macros.h (Assert, platform detection)
- [ ] Create Math project structure
- [ ] Design Memory allocator interfaces

---

## 2025-10-05 - Project Initialization

### Tasks
- [x] Set up project directory structure
- [x] Write initial documentation
  - README.md (English)
  - README.ko.md (Korean)
  - Architecture.md (English)
  - Architecture.ko.md (Korean)
  - CodingConvention.md (Korean)
  - DevelopmentLog.md (English)
- [x] Add MIT License
- [x] Configure Git repository
- [x] Create `.gitignore` for Visual Studio and DirectX projects
- [x] Set up Visual Studio 2022 solution file (empty projects)

### Decisions
- **Architecture**: Chose ECS (Entity Component System) as core architecture
- **Graphics API**: DirectX 12 as primary rendering API
- **Language**: C++20 for modern features and performance
- **Platform**: Windows-first, potential cross-platform later
- **Documentation**: Bilingual (English/Korean) for accessibility

### Notes
- Project structure designed with modularity in mind
- Planned 6 development phases from foundation to tools
- Solution created with empty project structure ready for implementation
- All core documentation completed before first commit

### Next Steps
- [ ] Create Core module project (Types, Memory, Logging)
- [ ] Implement basic Math library (Vector, Matrix)
- [ ] Set up unit testing framework
- [ ] Begin ECS framework implementation

---

## Template for Future Entries

```markdown
## YYYY-MM-DD - Brief Description

### Tasks
- [ ] Task 1
- [ ] Task 2
- [x] Completed task

### Decisions
- Decision point and rationale

### Issues Encountered
- Problem and solution

### Notes
- Additional observations or learnings

### Next Steps
- [ ] Upcoming task 1
- [ ] Upcoming task 2
```

---

## Development Phases

### Phase 0: Project Setup ✅
- **Status**: Complete
- **Duration**: 2025-10-05
- **Key Deliverables**: Documentation, project structure

### Phase 1: Foundation Systems 🔄
- **Status**: Not Started
- **Target**: TBD
- **Key Goals**: Core systems, Math library, Basic ECS

### Phase 2: Graphics
- **Status**: Not Started
- **Key Goals**: DirectX 12 initialization, Basic rendering pipeline

### Phase 3: Advanced ECS
- **Status**: Not Started
- **Key Goals**: Component queries, System dependencies

### Phase 4: Gameplay Systems
- **Status**: Not Started
- **Key Goals**: Physics, AI pathfinding, Terrain generation

### Phase 5: Advanced Rendering
- **Status**: Not Started
- **Key Goals**: PBR materials, Shadow mapping, Post-processing

### Phase 6: Tools
- **Status**: Not Started
- **Key Goals**: Asset pipeline, Editor prototype

---

## Statistics

- **Project Start Date**: 2025-10-05
- **Total Commits**: 0 (Initial commit pending)
- **Lines of Code**: 0
- **Modules Completed**: 0/8

---

## Resources & Learning

### Books
- Game Engine Architecture by Jason Gregory
- 3D Game Programming with DirectX 12 by Frank Luna

### Documentation
- [DirectX 12 Official Docs](https://docs.microsoft.com/en-us/windows/win32/direct3d12/)
- [Learn OpenGL](https://learnopengl.com/)

### References
- [EnTT - ECS Library](https://github.com/skypjack/entt)
- [Game Programming Patterns](https://gameprogrammingpatterns.com/)

---

**Last Updated**: 2025-10-05