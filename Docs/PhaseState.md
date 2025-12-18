## Phase State

### Phase 1: 기초 시스템 (완료)
- [x] 프로젝트 구조 설정
- [x] Core 시스템 (메모리, 로깅, 타이머)
- [x] Math 라이브러리 (SIMD)

### Phase 2: DirectX 12 기초 (완료)
- [x] 윈도우 생성 및 입력 처리
- [x] DX12 초기화
- [x] 기본 렌더링 파이프라인
- [x] 메시 및 텍스처 시스템
- [x] 카메라 시스템
- [x] Framework 아키텍처 (Application, ResourceManager, Scene)

---

### Phase 3: ECS 아키텍처 (진행 중)

#### 3.1 ECS Foundation (완료)
- [x] Entity (ID + Version)
- [x] Registry (생성/삭제/재활용)
- [x] Component Storage (unordered_map)
- [x] TransformComponent (순수 데이터)
- [x] MeshComponent, MaterialComponent
- [x] TransformSystem (정적 함수)
- [x] ResourceId 시스템 (64비트 해시)
- [x] UTF-8 경로 통일
- [x] 09_ECSRotatingCube 샘플

#### 3.2 System Framework (완료)
- [x] ISystem 인터페이스
- [x] SystemManager
- [x] RenderSystem (자동 수집)
- [x] Query/View 패턴
- [x] Registry::View<T...>() 구현

#### 3.3 Lighting (완료)
- [x] DirectionalLightComponent
- [x] PointLightComponent
- [x] LightingSystem
- [x] Phong Shading
- [x] Normal Mapping

#### 3.3.5 리팩토링 (완료)
- [x] Shutdown 리소스 해제 순서 개선
- [x] ISystem 인터페이스 통일
- [x] MathTypes 클래스화 및 리펙토링

#### 3.4 Debug Tools (완료)
- [x] ImGui 통합
- [x] ECS Inspector
- [x] 성능 모니터링

#### 3.5 Advanced (완료)
- [x] HierarchyComponent 설계 및 구현
- [x] Transform 계층 구조 (parent-child)
- [x] Dirty Flag 최적화 (localDirty, worldDirty)
- [x] Root Entity 관리
- [x] DFS 순회 기반 World Matrix 업데이트
- [x] 지연 전파 방식 (Update 일괄 처리)
- [x] Hierarchy 없는 Entity 호환성 유지
- [x] ForceUpdateWorldMatrix() 즉시 계산 API
- [x] 10_PhongLighting 샘플에 계층 테스트 통합

---

### Phase 4: Asset Pipeline (다음 단계)
- [ ] glTF 2.0 / Assimp 로더
- [ ] DDS/BCn 텍스처 파이프라인
- [ ] Mipmap 자동 생성
- [ ] Descriptor 관리 고도화

### Phase 5: PBR Lighting
- [ ] Metallic-Roughness Workflow
- [ ] Cook-Torrance BRDF
- [ ] IBL (Skybox, PMREM, Irradiance Map)

### Phase 5.5: Physics Integration (포트폴리오 마일스톤)
- [ ] 물리 엔진 통합
- [ ] 충돌 감지
- [ ] Rigidbody Component

---

## 향후 최적화 예정 (Phase 4+)

### Transform 계층 구조
| 영역 | 현재 | 최적화 | 조건 |
|------|------|--------|------|
| Children | std::vector | firstChild + nextSibling | 메모리 제약 시 |
| 순회 | DFS 재귀 | 명시적 스택 | 깊이 100+ 시 |
| 순회 | DFS 재귀 | Topological Sort 캐싱 | Entity 10,000+ 시 |
| 캐시 | local + world | + worldInvTranspose | 조명 병목 시 |
| 병렬화 | 단일 스레드 | 서브트리 병렬 처리 | Job System 도입 후 |

### ECS Inspector
- [ ] 계층 트리 뷰 (Phase 4)
- [ ] Parent/Child 관계 시각화