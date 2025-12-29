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

## 2025-12-22 - Phase 3.6 Debug Visualization 구현

### Tasks
- [x] DebugVertex, DebugConstants 타입 정의
- [x] Debug 전용 셰이더 (DebugVS.hlsl, DebugPS.hlsl)
- [x] PrimitiveGenerator, DebugShapes (Header-Only 유틸리티)
- [x] DebugRenderer 클래스 (별도 Root Signature, PSO)
- [x] DX12Renderer 통합, FrameData 확장
- [x] ImGui DebugVisualizationPanel, ECSInspector 선택 콜백 연동
- [x] Display Mode (All / SelectedOnly / None), 선택 색상 강조

### Decisions

- **분리된 파이프라인**: Debug 렌더링은 메인과 독립, Depth On/Off PSO로 X-Ray 모드 지원
- **정적 단위 도형**: Arrow, Sphere를 DX12VertexBuffer로 한 번만 업로드, Transform은 CB로 전달
- **Dynamic Offset CB**: 프레임당 4096개 도형, 256바이트 정렬 슬롯 인덱싱
- **QuaternionFromToRotation**: 직접 행렬 구성 대신 쿼터니언으로 안정적인 방향 회전
- **콜백 패턴**: SelectionChangedCallback으로 Framework-Graphics 모듈 간 느슨한 결합

### Issues Resolved

- **CB 덮어쓰기**: GPU 지연 실행으로 마지막 도형만 렌더링 → Dynamic Offset 패턴으로 해결
- **VB 덮어쓰기**: 동적 버퍼 공유로 Arrow가 Sphere로 덮어씀 → 도형별 정적 버퍼 분리
- **회전 행렬 오류**: 행/열 기준 혼동 → QuaternionFromToRotation + MatrixSRT 사용

### Next Steps
- [ ] TODO: 선택된 Mesh 외곽선 하이라이트 (Stencil Buffer 방식 권장)
- [ ] Phase 4: 모델 로딩 (Assimp/glTF 통합)
- [ ] Phase 5: PBR Lighting

---

## 2025-12-18 - Phase 3.5: Transform 계층 구조 및 Dirty Flag 최적화

### Tasks
- [x] HierarchyComponent (parent, children 저장)
- [x] TransformComponent 확장 (localMatrix, worldMatrix 캐시, dirty 플래그)
- [x] TransformSystem 계층 API (SetParent, GetParent, GetChildren, IsRoot)
- [x] DFS 순회 기반 World Matrix 업데이트
- [x] 지연 전파 방식 (Update에서 일괄 처리)
- [x] ForceUpdateWorldMatrix() 즉시 계산 API
- [x] 10_PhongLighting 샘플에 계층 테스트 통합

### Decisions

**HierarchyComponent 분리**
- TransformComponent와 별도 Component로 설계
- 이유: ECS Single Responsibility, 선택적 계층 참여

**Dirty Flag 분리 (localDirty, worldDirty)**
- Parent 이동 시 worldDirty만 true (local 재계산 불필요)
- 이유: 불필요한 행렬 연산 방지

**지연 전파 방식**
- Transform 변경 시 dirty만 설정, Update에서 일괄 처리
- 이유: 한 프레임 다중 수정 시 중복 전파 방지

**DFS 순회 + dirty 서브트리 스킵**
- dirty 아닌 서브트리는 즉시 return
- 이유: 구현 단순, 계층 변경에 강건, 성능 최적화

### Implementation

**새로 추가된 파일**
```
Engine/include/ECS/Components/HierarchyComponent.h
```

**주요 변경**
- TransformComponent: localMatrix, worldMatrix, localDirty, worldDirty 추가
- TransformSystem: 계층 API, Update() 로직, Root Entity 관리
- Archetype.h: HierarchyTransformArchetype 추가

**World Matrix 계산**
```cpp
// World = Local * ParentWorld
transform->worldMatrix = transform->localMatrix * parentWorldMatrix;
```

### Results

- 부모 회전 시 자식들 자동 공전
- 2단계 계층 정상 동작 (Parent → Child → GrandChild)
- HierarchyComponent 없는 Entity는 기존처럼 동작 (호환성 유지)

### Lessons Learned

- position/rotation/scale은 로컬 좌표, 렌더링은 worldMatrix 사용
- 지연 전파로 다중 수정 시에도 일정한 비용
- Component 직접 수정 시 dirty 누락 위험 → System API 사용 권장

### Next Steps
- [ ] Phase 4: 모델 로딩 (Assimp/glTF 통합)
- [ ] Phase 5: PBR Lighting
- [ ] ECS Inspector 계층 트리 뷰 (선택적)

---

## 2025-12-17 - Phase 3.4: Debug Tools (ImGui Integration)

### Tasks
- [x] ImGui 프로젝트 통합 및 ImGuiManager 구현
- [x] Win32 입력 연동 (IsMouseAvailable, IsKeyboardAvailable)
- [x] 렌더링 파이프라인 분리 (BeginFrame/RenderScene/EndFrame/Present)
- [x] PerformancePanel (FPS, Frame Time, 그래프)
- [x] ECSInspector (Entity/Component 조회, 편집, 추가/삭제)
- [x] 키보드 토글 (F1: Performance, F2: Inspector, F3: 전체 UI)

### Decisions

**ImGui 전용 Descriptor Heap 분리**
- Renderer Heap과 별도 관리
- 향후 Bindless 전환 시 유연성 확보

**렌더링 파이프라인 분리**
- ImGui는 CommandList Close 전에 렌더링 필요
- BeginFrame → RenderScene → [ImGui] → EndFrame → Present

**ECSInspector Registry 전달**
- Application이 아닌 Sample에서 OnRenderDebugUI()로 직접 전달
- Framework 모듈의 ECS 의존성 최소화

### Issues Resolved

**DME_DEBUG_UI 매크로 불일치**
- 모듈 간 클래스 크기 불일치로 메모리 손상 발생
- 해결: 조건부 컴파일 제거, 항상 ImGui 포함

**ImGui::NewFrame() 크래시**
- 폰트 텍스처 미업로드 상태에서 호출
- 해결: Initialize()에서 CommandList로 GPU 업로드

### Lessons Learned

- ImGui DX12: 전용 Heap + 폰트 업로드 필수
- 클래스 멤버 #if 조건부 컴파일 → 모듈 간 크기 불일치 주의
- Component 조회는 포인터 반환이 업계 표준 (nullptr로 부재 표현)

### Next Steps
- [ ] Phase 3.5: Transform 계층 구조
- [ ] Phase 4: 모델 로딩 (Assimp 통합, FBX/OBJ 지원)
- [ ] Phase 5: PBR Lighting (Metallic-Roughness Workflow)

---

## 2025-12-08 - Phase 3.3 Lighting System 구현

### Tasks
- [x] DirectionalLightComponent, PointLightComponent 구현
- [x] LightingSystem (CollectDirectionalLights, CollectPointLights)
- [x] Phong Shading Shader (PhongVS.hlsl, PhongPS.hlsl)
- [x] Constant Buffer 확장 (b0, b1, b2)
- [x] Dynamic Constant Buffer Offset 시스템
- [x] StandardVertex에 Tangent 추가
- [x] MeshUtils (Tangent 자동 계산, Header-Only)
- [x] Normal Mapping 적용
- [x] 10_PhongLighting 샘플 애플리케이션

### Decisions

**Component 구조**
- DirectionalLight/PointLight를 LightComponents.h 단일 파일로 통합
- Component는 Vector3, GPU 데이터는 Vector4 (homogeneous coordinates)
- 이유: 관련 Component 그룹화, 4x4 행렬 연산 호환성

**Constant Buffer 설계**
- 3개 분리: b0 (Object), b1 (Material), b2 (Lighting)
- Dynamic Offset 방식으로 1000 오브젝트 지원
- 이유: 업데이트 빈도 분리, 여러 오브젝트 효율적 처리

**Tangent 계산 방식**
- CPU 사전 계산 선택 (GPU ddx/ddy 대신)
- Lengyel's Method + Gram-Schmidt Orthogonalization
- 이유: 품질 보장, Phase 4 모델 로딩과 호환, 업계 표준

**Vertex 구조 통일**
- StandardVertex에 Tangent 포함 (StandardVertexWithTangent 불필요)
- 이유: 구조 단순화, 관리 편의성

**Header-Only MeshUtils**
- Math 모듈을 헤더만으로 구현
- 이유: 인라이닝 최적화, Math 모듈 컨벤션 준수

### Issues Encountered

**이슈 1: 25개 큐브 중 1개만 렌더링**
- 원인: Constant Buffer 덮어쓰기 (같은 주소에 반복 Update)
- 해결: mCurrentObjectCBIndex로 오브젝트별 오프셋 관리, UpdateAtOffset() 추가

**이슈 2: Point Light 미작동**
- 원인: HLSL Constant Buffer 구조체 정렬 불일치 (uint[3] vs uint3)
- 해결: `uint padding[3]` → `uint3 padding` (12바이트 연속 블록)

**이슈 3: Camera 회전 시 isDirty 미설정**
- 원인: SetLookAt()이 Transform만 수정, CameraComponent.isDirty 미설정
- 해결: SetLookAt()에 CameraComponent 매개변수 추가

### Results

**Lighting System**
- Directional Light (태양광, 균일 조명)
- Point Light (거리 감쇠, 색상별 4개)
- Blinn-Phong Model (Ambient + Diffuse + Specular)

**Normal Mapping**
- 표면 디테일 표현 (벽돌 틈새, 돌출)
- Tangent Space → World Space 변환 (TBN 행렬)
- 자동 Tangent 계산 (MeshUtils)

**샘플 애플리케이션**
- 5×5 그리드 큐브 (25개)
- 1 Directional Light + 4 Point Lights
- 큐브 회전 + 카메라 회전

### Notes

**HLSL 정렬 규칙**
- 구조체 배열 요소는 16바이트 배수로 정렬
- `uint3` (12 bytes 연속) vs `uint[3]` (각 요소 개별 정렬)
  - HLSL cbuffer 내 배열(type name[N])의 각 요소는 강제로 16바이트 정렬됨 (Stride 16)
  - C++ uint32[3](12 bytes)과 매칭하려면 HLSL에서는 uint3(12 bytes) 벡터를 사용해야 함
- Vector4 사용으로 자동 정렬 보장

**Constant Buffer 패턴**
- Triple Buffering (프레임별 독립 메모리)
- Dynamic Offset (여러 오브젝트 효율적 처리)
- BeginFrame()에서 mCurrentObjectCBIndex = 0 리셋

**Tangent Space 수학**
- Lengyel's Method: Edge/UV delta로 Tangent 계산
- Gram-Schmidt: Tangent - (Tangent·Normal) * Normal
- TBN 행렬: [Tangent | Bitangent | Normal]

**ECS 패턴**
- Component: 순수 데이터 (isDirty 플래그 포함)
- System: 정적 함수, 상태 없음
- View 패턴: CreateView<Transform, PointLight>()

### Lessons Learned

- DirectX 12 Constant Buffer는 256바이트 정렬 필수
- HLSL 구조체 정렬은 C++과 다름 (배열, 벡터 주의)
- CPU 사전 계산이 GPU 실시간 계산보다 품질/성능 우수
- Dynamic Offset으로 많은 오브젝트 효율적 처리
- Header-Only는 Math 유틸리티에 적합 (인라이닝 최적화)

### Next Steps
- [ ] Phase 3.4: ImGui 통합 및 실시간 파라미터 조정, ECS Inspector
- [ ] Phase 3.5: Advanced (선택)
- [ ] Phase 4: 모델 로딩 (Assimp 통합, FBX/OBJ 지원)
- [ ] Phase 5: PBR Lighting (Metallic-Roughness Workflow)

---

## 2025-11-24 - Archetype 시스템 구현

### Tasks
- [x] Archetype<Components...> 템플릿 구조 구현
- [x] 공통 Archetype 정의 (Renderable, Camera, TransformOnly)
- [x] 컴파일 타임 유틸리티 (HasComponent, IsSameArchetype)
- [x] 기존 코드를 Archetype 방식으로 전환

### Decisions

**Archetype 구조**
- Component 조합을 타입으로 정의
- CreateView()로 Registry::CreateView<T...>() 래핑
- 컴파일 타임 메타프로그래밍 (HasComponent, GetComponentType)
- 이유: Component 조합 재사용, 타입 안전성, 코드 간결화

**공통 Archetype 정의**
```cpp
using RenderableArchetype = Archetype<Transform, Mesh, Material>;
using CameraArchetype = Archetype<Transform, Camera>;
```
- 자주 사용되는 조합을 미리 정의
- 의도가 명확한 네이밍
- 이유: 긴 템플릿 인자 제거, 일관성 유지

**사용 방식**
```cpp
// 기존: registry.CreateView<Transform, Mesh, Material>();
// 변경: RenderableArchetype::CreateView(registry);
```

### Implementation

**새로 추가된 파일**
```
Engine/include/ECS/
├── Archetype.h
└── Archetype.inl
```

**주요 변경**
- RenderSystem, CameraSystem에서 Archetype 방식 사용
- CreateView 호출을 Archetype::CreateView로 래핑

### Results

**가독성 및 유지보수성**
- 긴 템플릿 인자 목록 제거
- Component 조합 변경 시 한 곳만 수정
- 타입 이름으로 의도 전달 (RenderableArchetype)

**타입 안전성**
- 컴파일 타임에 Component 조합 검증
- 잘못된 조합 시 컴파일 에러

**확장성**
- 추후 Archetype 기반 Storage 최적화 가능
- Query DSL 확장 (With, Without 조건)

### Lessons Learned

- using 선언으로 복잡한 템플릿을 도메인 언어로 표현
- 컴파일 타임 검증으로 런타임 오버헤드 제거
- 기존 시스템과 공존 가능한 점진적 개선
- Fold Expression, std::tuple로 메타프로그래밍 구현

### Next Steps
- [ ] Phase 3.3: Lighting System 구현
- [ ] Phase 3.4: ImGui 통합 및 ECS Inspector
- [ ] Phase 3.5: Transform 계층 구조

---

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

**최종 업데이트**: 2025-12-17