# Development Log

---

## 2025-10-13 - Input System Implementation

### Tasks
- [x] Design Input class architecture
- [x] Implement keyboard/mouse state tracking
  - Double buffering system (current + previous frame)
  - IsKeyPressed/Down/Released distinction
- [x] Integrate Input with Window (window-owned instance)
- [x] Create 07_InputTest sample
- [x] Update documentation

### Decisions
- **Window-owned instance**: Each window has independent input state (not singleton)
- **Double buffering**: Track current + previous frame state to distinguish "pressed" vs "held"
- **Math::Vector2 integration**: Mouse position/delta use engine's vector type
- **Frame cycle pattern**: `Update()` → `ProcessEvents()` → Game Logic → `Reset()`
- **Mouse wheel normalization**: Win32 delta (±120) normalized to ±1.0

### Lessons Learned
- **Win32 Coordinate Sign Extension**: `LOWORD(lParam)` doesn't sign-extend for negative coordinates. Must cast to `short` first for multi-monitor support.
- **System Key Handling**: Alt key combinations require handling both `WM_KEYDOWN` and `WM_SYSKEYDOWN` messages separately.
- **State vs Event Distinction**: Keys/buttons are persistent state, but mouse wheel is a per-frame event requiring explicit `Reset()`.

### Future Improvements
- **Input Mapping System**: Rebindable keys with command pattern for user customization.
- **Text Input Support**: WM_CHAR handling for text fields and UI input.
- **Gamepad Support**: XInput integration for Xbox controllers.
- **Input Recording**: Record/playback functionality for replays and automated testing.

### Notes
- ~400 lines of code, 0 warnings (Level 4)
- Window-owned design supports future multi-window scenarios
- Double buffering pattern applicable to future input devices (gamepad, touch)
- Clean integration: Input state ↔ Window events ↔ Math types

### Next Steps (Phase 4: DirectX 12)
- [ ] **Graphics module structure** - Create DX12 wrapper project
- [ ] **Device initialization** - DX12Device, factory, adapter
- [ ] **SwapChain creation** - Double/triple buffering setup
- [ ] **Command Queue** - Graphics command submission
- [ ] **08_DX12Init sample** - First clear screen render

---

## 2025-10-12 - Platform Layer Implementation

### Tasks
- [x] Design Platform Layer architecture
- [x] Implement Window abstraction (interface + Win32 implementation)
- [x] Create 06_WindowTest sample
- [x] Update documentation

### Decisions
- **Header organization**: All headers in `include/` for consistency with Core/Math
  - Chose consistency over encapsulation for learning project
- **Window architecture**: Abstract interface + Factory pattern
  - `Window` interface → `Win32Window` implementation → `CreatePlatformWindow()` factory
- **Win32 message handling**: Static WindowProc + instance HandleMessage pattern
- **Event system**: Simple callback pattern (full event queue deferred)
- **Message loop**: Non-blocking `PeekMessage` for real-time rendering

### Issues Encountered
- **Windows.h pollution**: Forward-declared `HWND__`/`HINSTANCE__` in headers, include only in .cpp
- **Unicode conversion**: `MultiByteToWideChar` for UTF-8 → UTF-16 conversion
- **Window sizing**: Used `AdjustWindowRectEx` to account for borders

### Performance
- Window creation: ~5 ms
- ProcessEvents: ~0.01-0.05 ms per frame
- Memory overhead: ~200 bytes per window
- Platform.lib: ~45 KB (Debug)

### Code Statistics
- **New files**: 6 (3 headers, 3 implementations)
- **Lines of code**: ~650
- **Build time**: < 2 seconds

### Notes
- Clean separation between interface and implementation
- Logging integration from day 1 simplifies debugging
- Ready for DirectX 12 integration

### Next Steps (Recommended Priority)
- [ ] **Input System** - Keyboard/mouse state tracking (Phase 3)
- [ ] **DirectX 12 initialization** - Device, SwapChain, first clear screen (Phase 4)
- [ ] High-DPI awareness (low priority)
- [ ] Multi-monitor support (low priority)

---

## 2025-10-11 - Logging System Implementation

### Tasks
- [x] Design and implement logging system
  - Logger singleton (thread-safe)
  - Sink pattern (Console + File)
  - Macro-based API (LOG_INFO, LOG_ERROR, etc.)
- [x] Create 05_LoggingTest sample
- [x] Apply logging to Memory allocators
- [x] Update documentation

### Decisions
- **Singleton pattern**: Global access with thread safety
- **Sink pattern**: Extensible output system
- **Printf-style formatting**: Simple, no dependencies
- **Macro API**: Compile-time optimization (Trace/Debug removed in Release)
- **Console colors**: Visual log level distinction (Windows Console API)

### Issues Encountered
- **localtime() warning**: Replaced with localtime_s()
- **Macro variadic args**: Used ##__VA_ARGS__ for optional arguments
- **File path in macros**: Strip path with GetFileName() helper

### Performance
- Debug overhead: ~50 microseconds/log
- Release overhead: 0 (Trace/Debug compiled out)
- Memory allocators now have comprehensive logging

### Notes
- ~300 lines of code, 0 warnings (Level 4)
- Async logging deferred until rendering performance profiling
- Log rotation deferred until long-running applications needed
- JSON sink deferred until profiling tools required

### Next Steps (Recommended Priority)
- [ ] **Platform layer** - Window creation (Win32 API)
- [ ] **DirectX 12 initialization** - Device, SwapChain, basic render loop
- [ ] ECS TransformComponent (using Math library)

---

## 2025-10-11 - Math Library with DirectXMath

### Tasks
- [x] Create Math project (Static Library)
- [x] Wrap DirectXMath for convenient usage
- [x] Implement MathTypes.h
  - Type aliases (Vector2/3/4, Matrix3x3/4x4, Quaternion)
  - SIMD types (VectorSIMD, MatrixSIMD)
  - Common constants (PI, DEG_TO_RAD, etc.)
  - Load/Store helpers
- [x] Implement MathUtils.h
  - Vector operations (Add, Dot, Cross, Normalize, etc.)
  - Matrix operations (Multiply, Inverse, Transpose, etc.)
  - Transform matrices (Translation, Rotation, Scaling)
  - Camera matrices (LookAt, Perspective, Orthographic)
  - Quaternion operations (FromEuler, Slerp, RotateVector, etc.)
  - Utility functions (Clamp, DegToRad, Lerp)
- [x] Create 04_MathTest sample
  - Vector operations test
  - Matrix operations test
  - Transform composition test
  - Camera matrices test
  - Quaternion operations test

### Decisions
- **DirectXMath over custom**: Save time, production-quality SIMD optimization
  - Trade-off: Less learning opportunity, but more practical
- **Storage vs SIMD types**: Separate types for memory and calculation
  - Storage (Vector3): 12 bytes, array-friendly
  - SIMD (VectorSIMD): 16 bytes, fast calculation
- **Wrapper pattern**: Convenience functions around DirectXMath
  - Hide complex Load/Store operations
  - Cleaner API for common operations
- **Transform.h deferred**: Will implement as ECS TransformComponent later
  - Better fit with entity-component architecture

### Issues Encountered
- **Type confusion**: Initially unclear when to use storage vs SIMD types
  - Solution: Storage for data structures, SIMD for calculations
  - Load/Store helpers make conversion easy
- **Quaternion representation**: DirectXMath uses XMVECTOR for quaternions
  - Solution: XMFLOAT4 for storage, XMVECTOR for calculation

### Notes
- DirectXMath already included with Windows SDK (no external dependencies)
- All functions are inline (header-only, no .cpp needed)
- Math.lib is interface library (no actual binary output)
- Test demonstrates all common operations successfully
- ~400 lines total (headers + test), 0 warnings

### Next Steps
- [ ] Create Logging system (console logger)
- [ ] Begin Platform layer (Window creation)
- [ ] Implement ECS TransformComponent (using Math library)
- [ ] Create unified memory manager facade
- [ ] Add memory statistics/profiling

---

## 2025-10-11 - Pool & Stack Allocator Implementation

### Tasks
- [x] Implement PoolAllocator
  - Fixed-size chunk allocation with free-list
  - O(1) allocation and deallocation
- [x] Implement StackAllocator  
  - LIFO allocation with marker system
  - Scoped allocation support
- [x] Create test samples (02_PoolAllocatorTest, 03_StackAllocatorTest)
- [x] Apply comment conventions to all Memory headers

### Decisions
- **PoolAllocator alignment**: Limited to 8 bytes for now (free-list requires pointer storage)
  - 16-byte SIMD alignment deferred until optimization phase
- **StackAllocator overhead**: 8 bytes per allocation for padding metadata
- **Comment policy**: Self-documenting code prioritized, minimal comments in headers

### Issues Encountered
- **Alignment assertion failure**: `DEFAULT_ALIGNMENT` (16) vs `sizeof(void*)` (8) mismatch
  - Solution: Explicitly specify 8-byte alignment in test code
  - Deferred: Upgrade to 16-byte support with `_aligned_malloc` (premature optimization)

### Notes
- Three allocator patterns complete: Linear (fastest), Pool (individual free), Stack (scoped)
- All avoid OS calls after init (10-100x faster than new/delete)
- ~600 lines total (allocators + tests), 0 warnings
- Performance: PoolAllocator ~2ms, LinearAllocator ~0.5ms, StackAllocator ~0.8ms (10,000 allocs)

### Next Steps
- [ ] Create unified memory manager facade
- [ ] Add memory statistics/profiling
- [ ] Document allocator usage guidelines
- [ ] Begin Math library (Vector, Matrix, Quaternion)
- [ ] Create Logging system

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