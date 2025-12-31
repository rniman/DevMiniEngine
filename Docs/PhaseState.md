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

### Phase 3: ECS 아키텍처 (완료)
- [x] Entity, Registry, Component Storage
- [x] ISystem 인터페이스, SystemManager, Query/View 패턴
- [x] Lighting (Directional, Point, Phong Shading, Normal Mapping)
- [x] Transform 계층 구조 (Parent-Child, Dirty Flag 최적화)
- [x] Debug Tools (ImGui, ECS Inspector, Performance Panel)
- [x] Debug Visualization (Light Gizmo, DebugRenderer)

---

## Phase 4: DX12 인프라 확장 & Asset Pipeline

### 4.1 Asset Loading Foundation (완료)
- [x] AssetManager 클래스 설계
- [x] Asset 타입 열거 (Mesh, Texture, Material)
- [x] Asset 캐싱 및 참조 카운팅
- [x] 파일 경로 → ResourceId 매핑 확장
- [x] 11_ModelViewer 샘플 생성 (기본 구조)

### 4.2 Model Loading (완료)
- [x] Assimp 라이브러리 통합 (vcpkg)
- [x] glTF 2.0 메쉬 로딩
- [x] 버텍스 포맷 변환 (Position, Normal, UV, Tangent)
- [x] MikkTSpace 탄젠트 계산
- [x] 인덱스 버퍼 처리 (16/32비트 자동 선택)
- [x] 좌표계 변환 (glTF → DirectX 왼손)
- [x] 텍스처 자동 로딩 (외부 + 임베디드)
- [x] 폴백 텍스처 시스템 (1x1 Magenta)
- [x] 다중 머티리얼 Constant Buffer
- [x] 11_ModelViewer glTF 렌더링

### 4.3 Texture Pipeline (예정)
- [ ] sRGB / Linear 색공간 처리
- [ ] DDS 로더 구현 (DirectXTex)
- [ ] BCn 압축 텍스처 지원
- [ ] Mipmap 자동 생성

### 4.4 Descriptor 관리 고도화 (예정)
- [ ] SRV Descriptor 풀 및 슬롯 재활용
- [ ] Frame-based 할당
- [ ] 기존 렌더러에 통합

### 4.5 리소스 업로드 최적화 (선택적)
- [ ] Upload Ring Buffer 구현
- [ ] 프레임 펜스 기반 버퍼 재사용
- [ ] Async Copy Queue

### 4.6 Shader 시스템 확장 (선택적)
- [ ] DXC 컴파일러 통합 (SM 6.0+)
- [ ] Shader Reflection
- [ ] Hot Reload (개발 편의)

### 4.7 멀티 서브메시 + ECS 통합 (Phase 5 직전)
- [ ] Mesh::DrawSubmesh() 추가
- [ ] RenderItem.submeshIndex 추가
- [ ] MeshComponent Material 배열 지원
- [ ] 모델 계층구조 → HierarchyComponent 변환 (선택적)
- [ ] 11_ModelViewer 멀티 서브메시 모델 테스트

---

### Phase 5: PBR Lighting (예정)
- [ ] Metallic-Roughness Workflow
- [ ] Cook-Torrance BRDF
- [ ] IBL (Skybox, PMREM, Irradiance Map)

### Phase 5.5: Physics Integration (포트폴리오 마일스톤)
- [ ] 물리 엔진 통합
- [ ] 충돌 감지
- [ ] Rigidbody Component

---

## 진행 순서 (권장)

```
Phase 4.2 완료
    ↓
Phase 4.3 Texture Pipeline (sRGB, DDS)
    ↓
Phase 4.7 멀티 서브메시 + ECS 통합
    ↓
Phase 5 PBR Lighting
    ↓
Phase 5.5 Physics
```

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
- [ ] 계층 트리 뷰 (Phase 4+)
- [ ] Parent/Child 관계 시각화

---

## 수정 이력

| 날짜 | 변경 내용 |
|------|----------|
| 2025-12-31 | Phase 4.1, 4.2 완료 표시, 멀티 서브메시 4.7로 이동 |
| 2025-12-30 | Phase 4.2 Step 1~3 완료 |
| 2025-12-26 | Phase 4.1 완료 |