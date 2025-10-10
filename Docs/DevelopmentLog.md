# Development Log

---

## 2025-10-10 - Error Handling & Memory Safety

### Tasks
- [x] Implement Assert.h for error handling
  - CORE_ASSERT (debug only, uses assert)
  - CORE_VERIFY (always checks, throws exception)
- [x] Prevent memory leaks in LinearAllocator
  - Delete copy constructor and copy assignment
  - Replace assert with new macros

### Decisions
- **CORE_ASSERT vs CORE_VERIFY**: ASSERT for debug checks (compiled out in release), VERIFY for critical checks (always executed)
- **No copy semantics**: LinearAllocator owns raw memory - copying would cause double-free
- **Fail-fast on OOM**: Out-of-memory throws exception rather than returning nullptr

### Issues Encountered
- **ASSERT vs VERIFY usage**: Initially unclear when to use which
  - Solution: ASSERT for "should never happen in correct code", VERIFY for "might fail due to external factors"
- **OOM handling**: Debated between return nullptr vs throw exception
  - Decision: Fail fast - OOM is unrecoverable in our use case

### Notes
- Assert.h pattern will be reused across all Core modules
- Copy prevention pattern applies to all future allocators
- All existing MemoryTest cases still pass
- No performance impact in release builds

### Next Steps
- [ ] Implement PoolAllocator for fixed-size object pools
- [ ] Implement StackAllocator for scoped allocations
- [ ] Document assertion guidelines in CodingConvention.md
- [ ] Add memory profiling/statistics (peak usage, leak detection)
- [ ] Create Logging system (console and file logger)
- [ ] Begin Math project (Vector, Matrix, Quaternion)

---

## 2025-10-09 - Memory System Implementation & Testing

### Tasks
- [x] Design and implement Memory subsystem
  - MemoryConfig.h (alignment constants, helper functions)
  - Allocator.h (base interface with tracking)
  - LinearAllocator (header + implementation)
- [x] Create MemoryTest sample project
  - Configure project settings (executable, links Core.lib)
  - Write test suite for LinearAllocator
  - Build and run successful

### Decisions
- **LinearAllocator first**: Simpler than PoolAllocator, most commonly needed for frame allocations
- **16-byte default alignment**: For SIMD optimization and cache performance
- **No individual deallocation**: LinearAllocator design - bulk reset only for simplicity and speed
- **Sample project pattern**: Created Samples/MemoryTest for testing static library code

### Issues Encountered
- **Static library testing**: Initially unclear how to test Core.lib
  - Solution: Created separate executable sample project
- **Linker dependencies**: Required careful configuration of library paths and dependencies
- **Build order**: Set project dependencies so Core builds before MemoryTest

### Notes
- LinearAllocator: O(1) allocation, 10-100x faster than new/delete
- Test results: All alignment and allocation tests passed
- Memory tracking works correctly (size and count)
- Sample project pattern will be reused for future features
- ~300 lines of code (Memory system + tests), 0 warnings

### Next Steps
- [ ] Implement PoolAllocator for fixed-size object pools
- [ ] Implement StackAllocator for scoped allocations
- [ ] Add memory profiling/statistics (peak usage, leak detection)
- [ ] Create Logging system (console and file logger)
- [ ] Begin Math project (Vector, Matrix, Quaternion)

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