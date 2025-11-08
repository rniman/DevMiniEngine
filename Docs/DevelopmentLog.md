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

## 2025-11-08 - Phase 3.1: ECS Foundation & ResourceId 시스템

### Tasks
- [x] ECS 핵심 구조 (Entity, Component, Registry)
- [x] TransformComponent/System, MeshComponent, MaterialComponent
- [x] ResourceId 기반 리소스 참조 시스템 (64비트 해시)
- [x] UTF-8 경로 통일 및 Hash.h 구현
- [x] ResourceManager 및 Material을 ResourceId로 전환
- [x] 09_ECSRotatingCube 샘플

### Decisions

**ECS 아키텍처 채택**
- Entity: ID(uint32) + Version(uint32) - 재활용 안전성
- Component: 순수 데이터(POD), 로직 없음
- System: 정적 함수로 데이터 처리
- 이유: 메모리 효율성, 캐시 친화성, 데이터 지향 설계

**ResourceId 시스템 (64비트 해시)**
```cpp
struct ResourceId { uint64 id; };
struct MeshComponent { ResourceId meshId; };  // 8 bytes
```
- 32비트: 77K 리소스에서 충돌 50%
- 64비트: 5B 리소스에서 충돌 50% (실질적으로 불가능)
- UTF-8 경로 통일로 일관된 해시 보장

**Component 설계 원칙**
```cpp
// 데이터만 [O]
struct TransformComponent {
    Vector3 position;
    Quaternion rotation;
    Vector3 scale;
};

// 로직 금지 [X]
void Translate(...);  // System으로!
```

### Implementation

**ECS 구조**
```
Registry
├── Entities: vector<Entity>
├── Versions: vector<uint32>
├── FreeList: vector<uint32>
└── ComponentPools: map<TypeId, ComponentPool<T>*>
```

**리소스 흐름**
```
Component (ResourceId 8B) 
  → ResourceManager (실제 리소스 소유)
  → Renderer (GPU 제출)
```

### Results

**메모리 효율성**
- GameObject: ~100 bytes
- ECS Entity: ~56 bytes
- 1000개 벽에 같은 Material 공유: 8KB vs 100KB

**아키텍처**
- 순환 의존성 제거
- Component 단위 테스트 가능
- 캐시 친화적 순회

### Issues Resolved

**Material 순환 참조**
- shared_ptr → ResourceId 전환

**UTF-8/UTF-16 해시 불일치**
- Hash64(wstring) 내부에서 UTF-8 변환

### Lessons Learned

- Entity Version으로 dangling reference 방지
- Component는 절대 로직 포함하지 말 것
- ResourceId(8B) >> shared_ptr(24B), 직렬화 가능
- UTF-8 통일 필수 (해시 일관성)

### Next Steps
- [ ] Phase 3.2: ISystem + SystemManager + Query/View 패턴
- [ ] Phase 3.3: Lighting (Phong, Directional/Point Light)
- [ ] Phase 3.4: ImGui + ECS Inspector

---

**최종 업데이트**: 2025-11-09