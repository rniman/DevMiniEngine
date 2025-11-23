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

## 2025-11-23 - Phase 3.2.1: CameraComponent 통합

### Tasks
- [x] CameraComponent 순수 데이터 구조 구현
- [x] CameraSystem 정적 함수 구현
- [x] RenderSystem에서 PerspectiveCamera 의존성 제거
- [x] Camera Entity 통합 및 Main Camera 플래그 시스템
- [x] Dirty Flag 패턴 적용 (viewDirty, projectionDirty)

### Decisions

**CameraComponent 설계**
- 순수 데이터: ProjectionType, FOV, Aspect Ratio, Clip Planes
- 행렬 캐싱: viewMatrix, projectionMatrix를 Component에 저장
- Dirty Flag: 변경 시에만 재계산하여 최적화
- Main Camera: isMainCamera 플래그로 렌더링 대상 식별
- 이유: Component는 데이터만, 계산 결과 캐싱으로 재계산 방지

**CameraSystem 정적 함수**
- UpdateViewMatrix/ProjectionMatrix: Dirty Flag 체크 후 재계산
- FindMainCamera: Registry에서 Main Camera Entity 자동 검색
- Helper 함수: SetFovYDegrees, SetAspectRatio, SetLookAt 등
- 이유: 상태 없는 정적 함수로 ECS 원칙 준수

**RenderSystem 변경**
- PerspectiveCamera 포인터 의존성 제거
- FindMainCamera()로 Camera Entity 자동 검색
- 이유: Camera 타입 독립적 설계

### Implementation

**새로 추가된 파일**
```
Engine/include/ECS/
├── Components/CameraComponent.h
└── Systems/CameraSystem.h

Engine/src/ECS/Systems/CameraSystem.cpp
```

**주요 변경**
- ECSRotatingCubeApp: PerspectiveCamera → Camera Entity
- Transform + Camera Component 조합으로 카메라 구성
- SetMainCamera()로 하나의 Main Camera만 보장

### Results

**아키텍처 개선**
- PerspectiveCamera 클래스 완전 제거
- Camera도 ECS 프레임워크에 통합
- TransformComponent 재사용으로 코드 중복 제거
- CreateView<Transform, Camera>()로 쿼리 가능

**기능 유지**
- 09_ECSRotatingCube 샘플 정상 동작
- View/Projection 행렬 계산 결과 동일

### Lessons Learned

- Component에 계산 결과 캐싱하여 System 간 데이터 공유
- 모든 게임 오브젝트를 Entity로 일관되게 관리 (Camera 예외 제거)
- Helper 함수로 사용자 친화적 API 제공 (도 단위, 너비/높이 등)
- Component 크기 권장(64B)은 개체 수가 적은 경우 예외 허용

### Next Steps
- [ ] Phase 3.3: Lighting System (DirectionalLight, PointLight)
- [ ] Phase 3.4: ImGui 통합 및 ECS Inspector
- [ ] Phase 3.5: Transform 계층 구조

---

## 2025-11-15 - Phase 3.2: System Framework 구축

### Tasks
- [x] ISystem 인터페이스 설계 및 구현
- [x] RegistryView<T...> Query 패턴 구현
- [x] SystemManager 구현 (등록, 생명주기, 실행 순서 관리)
- [x] RenderSystem 구현 (FrameData 수집)
- [x] Registry에 CreateView<T...>() 추가
- [x] 09_ECSRotatingCube 샘플 SystemManager로 전환
- [x] 빌드 및 동작 검증

### Decisions

**ISystem 인터페이스**
- 생명주기: OnInitialize → OnUpdate (필수) → OnShutdown
- 활성화 제어: SetActive/IsActive로 System 활성화 관리
- 이유: 명확한 생명주기 + 선택적 초기화/종료

**RegistryView<T...> Query 패턴**
- Fold Expression으로 여러 Component 조합 쿼리
- 즉시 평가: View 생성 시 Entity 수집
- Range-based for 지원
- 이유: 직관적인 API, 효율적인 Entity 순회

**SystemManager**
- 등록 순서 = 실행 순서 (명시적)
- LIFO 종료 (역순 Shutdown)
- std::type_index로 중복 등록 방지
- 이유: 간단하면서도 안전한 실행 순서 보장

**RenderSystem 역할 (과도기)**
- 현재: 카메라 업데이트 + 데이터 수집 (혼재)
- Phase 3.2.1: CameraSystem 추가 후 수집만 담당
- 이유: System Framework 검증 우선, 올바른 책임 분리는 다음 단계

### Implementation

**새로 추가된 파일**
```
Engine/include/ECS/
├── ISystem.h
├── RegistryView.h
├── SystemManager.h
└── Systems/RenderSystem.h

Engine/src/ECS/
├── SystemManager.cpp
└── Systems/RenderSystem.cpp
```

**Registry.h 수정**
- RegistryView<T...> 전방 선언
- CreateView<T...>() 함수 선언
- 파일 끝에 RegistryView.h include

**RenderTypes.h 통일**
- RenderObject → RenderItem
- renderObjects → opaqueItems
- FrameData::Clear() 추가

### Results

**코드 구조 개선**
- Application 코드 간결화 (CollectRenderData 제거)
- SystemManager가 자동 실행
- 새 System 추가 용이 (RegisterSystem 한 줄)

**아키텍처 검증**
- System 등록 및 실행 순서 제어
- View를 통한 Component 조합 쿼리
- System 간 독립성
- 기존 기능 유지 (큐브 회전)

**메모리 및 성능**
- SystemManager: ~100 bytes
- RegistryView: 즉시 평가 (Phase 3.5 캐싱 예정)
- RenderSystem: FrameData 재사용

### Issues Resolved

**View 타입명 모호성**
- 문제: View<T...> 너무 일반적
- 해결: RegistryView<T...>로 명확화

### Lessons Learned

**System 책임 분리**
- RenderSystem이 카메라 업데이트는 임시 방편
- 각 System은 한 가지 책임 (Single Responsibility)

**Component에 계산 결과 캐싱**
- System이 계산한 결과를 Component에 저장
- 다른 System은 이미 계산된 값 사용 (복사만)
- Dirty Flag 패턴으로 최적화 가능

**등록 순서 = 실행 순서**
- 의존성 있는 System은 순서 중요
- 명시적이지만 실수 가능 (Phase 3.3+ Priority 고려)

**ECS 핵심 패턴**
- Entity = 핸들 (Id + Version)
- Component = 데이터 (POD)
- System = 로직
- Registry = 저장소
- View = 쿼리

### Next Steps
- [ ] Phase 3.2.1: CameraComponent 추가 (강력 권장)
- [ ] Phase 3.3: Lighting System (Phong Shading)
- [ ] Phase 3.4: ImGui + ECS Inspector
- [ ] Phase 3.5: Transform 계층 구조 + Query 최적화

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

**최종 업데이트**: 2025-11-23