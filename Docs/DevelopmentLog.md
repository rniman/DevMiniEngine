# Development Log

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