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

### 4.3 Texture Pipeline (완료)
- [x] sRGB / Linear 색공간 처리 (WICTextureLoader Extended)
- [x] Asset/Resource 클래스 이름 정리 (Mesh/Texture/Material → *Resource)
- [x] TextureAsset 메타데이터 확장 (width, height, format, sRGB)
- [x] Asset/Resource 소유권 분리 (AssetManager ↔ ResourceManager)
- [x] ImGui 텍스처 정보 패널 (Loaded Textures 테이블)

### 4.3+ Texture Pipeline 확장 (보류)
- [ ] DirectXTex 통합 (필요 시 별도 Phase)
- [ ] DDS 로더, BCn 압축, Mipmap 자동 생성

> 진행 타이밍: PBR 구현 시 VRAM 절약 필요하거나 대규모 씬 작업 시

### 4.4 멀티 서브메시 + ECS 통합 (완료)

> Phase 5 직전 필수. PBR 모델의 서브메시별 Material 적용 기반 구축

#### 4.4.1 Graphics 레이어 확장 (완료)
- [x] Graphics/SubmeshInfo.h 신규 생성
- [x] MeshAsset에서 기존 SubmeshInfo 제거, Graphics 버전 include
- [x] MeshResource에 서브메시 배열 추가
- [x] MeshResource::DrawSubmesh(commandList, submeshIndex) 구현
- [x] MeshResource::SetSubmeshes() 구현
- [x] ResourceManager::CreateMeshFromAsset()에서 서브메시 복사
- [x] 서브메시 없는 경우 자동 단일 등록 로직

#### 4.4.2 ECS 레이어 확장 (완료)
- [x] MaterialComponent 배열 구조로 변경 (MAX_MATERIALS = 8)
- [x] RenderItem.submeshIndex 추가
- [x] RenderSystem::Update() 서브메시 순회 로직
- [x] MeshResource Initialize에서 기본 서브메시 생성
- [x] ResourceManager 중복 SetSubmeshes 제거
- [x] ECSInspector MaterialComponent 표시 수정

#### 4.4.3 DX12Renderer 수정 (완료)
- [x] DX12Renderer::DrawRenderItems()에서 DrawSubmesh() 호출

#### 4.4.4 검증 및 테스트 (완료)
- [x] 멀티 서브메시 모델 테스트 (MultiMaterialCube - 3개 서브메시)
- [x] 서브메시별 다른 Material 적용 확인
- [x] 기존 Sphere, Helmet 모델 정상 동작 확인
- [x] ModelViewerApp 4개 Entity 렌더링 (Sphere x2, Helmet, Cube)

#### 4.4+ 멀티 서브메시 확장 (선택적)
- [ ] 동일 Material 서브메시 배칭
- [ ] Material 기준 정렬 (State Change 최소화)
- [ ] 런타임 Material 교체 API
- [ ] 노드 계층 → HierarchyComponent 변환

### 4.5 Descriptor 관리 고도화 (선택적)

> 진행 타이밍: 씬 전환 기능 구현 시, 에디터 개발 시, Descriptor 부족 경고 발생 시

- [ ] SRV Descriptor 풀 및 슬롯 재활용
- [ ] Frame-based 할당 (Ring Buffer 방식)
- [ ] 기존 렌더러에 통합

> 현재 상태: 2048개 고정 할당, 순차 할당만 동작, FreeBlock은 리스트 추가만 (재사용 미구현)

### 4.6 리소스 업로드 최적화 (선택적)

> 진행 타이밍: 대규모 씬 로딩 시 스터터링 발생, 스트리밍 로딩 필요, 런타임 리소스 생성 빈번

- [ ] Upload Ring Buffer 구현
- [ ] 프레임 펜스 기반 버퍼 재사용
- [ ] Async Copy Queue (별도 복사 큐)

> 현재 상태: 리소스당 개별 Upload Buffer 생성 후 즉시 해제

### 4.7 Shader 시스템 확장 (선택적)

> 진행 타이밍: SM 6.0+ 기능 필요 시, 셰이더 수정 사이클이 개발 병목 시

- [ ] DXC 컴파일러 통합 (SM 6.0+)
- [ ] Shader Reflection (자동 Root Signature 생성)
- [ ] Hot Reload (런타임 셰이더 수정)

> 현재 상태: D3DCompiler (SM 5.1), 수동 Root Signature

---

### Phase 5: PBR Rendering (예정)

#### 5.1 HDR Pipeline 구축
- [ ] HDR 렌더 타겟 (R16G16B16A16_FLOAT)
- [ ] 톤매핑 셰이더 (ACES)
- [ ] 렌더 패스 분리 (Scene → Post → UI)
- [ ] 기존 pow(1/2.2) 제거

#### 5.2 PBR Material
- [ ] Metallic-Roughness Workflow
- [ ] Cook-Torrance BRDF
- [ ] PBR 텍스처 슬롯 활용 (4.3에서 준비된 sRGB/Linear)

#### 5.3 IBL (Image-Based Lighting)
- [ ] Skybox
- [ ] Irradiance Map
- [ ] PMREM (Prefiltered Mipmaped Radiance Environment Map)

#### 5.4 Post-Process (선택적)
- [ ] Bloom
- [ ] Exposure Control

### Phase 5.b: Physics Integration (포트폴리오 마일스톤)
- [ ] 물리 엔진 통합
- [ ] 충돌 감지
- [ ] Rigidbody Component

---

## 진행 순서 (권장)

```
Phase 4.3 완료 ✅
    ↓
Phase 4.4 멀티 서브메시 + ECS 통합 완료 ✅
    ↓
Phase 5 PBR Rendering ← 현재
    ↓
Phase 4.5~4.7 (필요 시점에 삽입)
    ↓
Phase 5.b Physics
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
| 2026-01-06 | Phase 4.4 완료 (멀티 서브메시 + ECS 통합, MultiMaterialCube 테스트) |
| 2025-01-05 | Phase 4.4~4.7 재구성 (멀티 서브메시를 4.4로 이동, 기존 4.4~4.6을 4.5~4.7로) |
| 2025-01-04 | Phase 4.3 완료 (sRGB, Asset/Resource 분리, ImGui 패널) |
| 2025-12-31 | Phase 4.1, 4.2 완료 표시, 멀티 서브메시 4.7로 이동 |
| 2025-12-30 | Phase 4.2 Step 1~3 완료 |
| 2025-12-26 | Phase 4.1 완료 |