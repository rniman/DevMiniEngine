## Template for Future Entries

```markdown
## YYYY-MM-DD - Brief Description

### Overview

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

## 2026-01-12 - Phase 4.5: Hierarchical Model Loading

### Overview

glTF 노드 계층 구조를 ECS Entity 계층으로 변환하는 기능 구현. 2CylinderEngine.glb(83노드, 43메시) 테스트 완료. 리팩토링으로 MeshAsset 소유권 정리, ECSInspector 트리뷰 추가.

### Tasks

- [x] LoadedNodeData 구조체 추가 (parentIndex, childIndices, meshIndices, localTransform)
- [x] DecomposeMatrix() 행렬→TRS 분해 함수
- [x] CreateModelHierarchy() 5단계 계층 생성 로직
- [x] CreateMeshFromNodeIndices() 노드별 메시 병합
- [x] ModelHierarchyResult 결과 구조체
- [x] ECSInspector 계층 트리뷰 (재귀 TreeNode)
- [x] MeshAsset 소유권 AssetManager 이전
- [x] 상수 버퍼 정렬 확인 (256B, MAX 200)

### Decisions

**계층 생성 5단계**
1. 노드→Entity 매핑 테이블
2. Entity 생성 + Transform + Hierarchy
3. SetParent()로 관계 설정
4. 루트 위치 오프셋
5. 메시/머티리얼 연결

**MeshAsset 소유권**
- AssetManager가 소유 (TextureAsset과 일관성)
- ResourceManager::CreateMeshFromAsset()에서 std::move로 이전
- App은 ResourceId만 보유

**Material 파이프라인**
- 현재 구조 유지 (MaterialResource가 PBR 파라미터 소유)
- baseColor=(1,1,1,1), metallic=0, roughness=0.5

### Issues Encountered

**Dead Code 발견**
- LoadAndCreateMesh() 미사용 (CreateModelHierarchy 전환 후)

### Notes

**수정 파일**: ModelLoader.h/cpp, ModelViewerApp.h/cpp, AssetManager.h/cpp, ResourceManager.h/cpp, ECSInspector.h/cpp

**테스트 결과**

| 모델 | 노드 | 렌더링 Entity |
|------|------|---------------|
| DamagedHelmet | 2 | 1 |
| MultiMaterialCube | 1 | 1 |
| 2CylinderEngine | 83 | 43 |

**아키텍처 원칙**
- Asset(CPU) / Resource(GPU) 분리
- AssetManager: MeshAsset, TextureAsset 소유
- 256B 정렬 상수 버퍼

### Next Steps

- [ ] Phase 5: PBR Pipeline
- [ ] IBL (Image-Based Lighting)
- [ ] 환경 맵 지원

---

## 2026-01-09 - Phase 4.4 Refactoring 02: ECS Component/System API 개선

### Overview

Phase 5 PBR 진입 전 두 번째 리팩토링. MaterialComponent 헬퍼 함수 외부화, TransformSystem 유틸리티 함수 추가, CameraSystem 전면 개선 (CameraUpMode, Dirty 제거).

### Tasks

- [x] MaterialComponent 내부 함수 → MaterialHelpers 네임스페이스로 분리
- [x] TransformSystem에 MoveToward, Lerp, LookAt 등 유틸리티 추가
- [x] CameraComponent에 CameraUpMode 추가 (WorldUp/LocalUp)
- [x] CameraComponent worldUpReference/localUp 벡터 분리
- [x] CameraSystem API 구조 정리 (고수준/저수준 분리)
- [x] CameraComponent Dirty Flag 완전 제거
- [x] ECSInspector Camera Up Mode 콤보박스 추가

### Decisions

**MaterialComponent 헬퍼 외부화**
- 변경 전: MaterialComponent 내부 static 함수
- 변경 후: MaterialHelpers 네임스페이스의 독립 함수
- 이유: ECS 원칙 (Component는 순수 데이터), 재사용성 향상

```cpp
// 변경 전
MaterialComponent::SetSingleMaterial(matComp, materialId);

// 변경 후
MaterialHelpers::SetSingleMaterial(matComp, materialId);
MaterialHelpers::SetMaterials(matComp, {mat1, mat2, mat3});
MaterialHelpers::GetMaterialForSubmesh(matComp, submeshIndex);
```

**CameraUpMode 도입**
- WorldUp: 월드 Y축 고정 (FPS, TPS, RTS)
- LocalUp: 로컬 up 회전 적용 (비행 시뮬, 우주 게임)
- 기본값: WorldUp (대부분의 게임에서 사용)

```cpp
enum class CameraUpMode : uint8
{
    WorldUp,   // worldUpReference 사용
    LocalUp    // localUp을 rotation으로 회전
};
```

**Camera Dirty Flag 제거**
- 선택: 매 프레임 View/Projection 행렬 항상 계산
- 이유: 카메라는 씬에 소수, dirty 관리 복잡도 대비 이득 없음
- 장점: Transform 변경 자동 반영, 코드 단순화, 버그 가능성 감소

```cpp
// 변경 전
void UpdateViewMatrix(const TransformComponent& transform, CameraComponent& camera)
{
    if (!camera.viewDirty) return;
    // ... 계산 ...
    camera.viewDirty = false;
}

// 변경 후
void UpdateViewMatrix(const TransformComponent& transform, CameraComponent& camera)
{
    // 카메라는 씬에 1-2개이므로 매 프레임 항상 계산
    // ... 계산 ...
}
```

**TransformSystem 유틸리티 함수**
- MoveToward: 목표 지점으로 일정 거리 이동
- Lerp: 선형 보간
- LookAt: 목표 방향으로 회전
- 이유: 게임플레이 코드에서 자주 사용되는 패턴 제공

### Files Modified

| 파일 | 변경 내용 |
|------|-----------|
| MaterialComponent.h | 내부 함수 제거, MaterialHelpers 네임스페이스 추가 |
| TransformSystem.h/cpp | 유틸리티 함수 선언 및 구현 |
| CameraComponent.h | viewDirty/projectionDirty 제거, CameraUpMode/worldUpReference/localUp 추가 |
| CameraSystem.h/cpp | MarkDirty 함수 제거(매 프레임 계산 방식), upMode 분기 처리 |
| ECSInspector.cpp | Camera Up Mode 콤보박스, MarkDirty 호출 제거 |
| ModelViewerApp.cpp | 카메라 초기화 직접 필드 설정 방식 |

### Results

**MaterialComponent**
- Component는 순수 데이터만 포함 (ECS 원칙 준수)
- 헬퍼 함수로 사용성 유지

**TransformSystem**
- 게임플레이에 필요한 유틸리티 제공
- 기존 API와 일관된 패턴 (Entity 기반 + Component 기반)

**CameraSystem**
- Transform 변경 시 View 행렬 자동 업데이트
- Inspector에서 카메라 파라미터 즉시 반영
- API 표면 축소 (MarkDirty 함수 3개 제거)
- Up Mode 변경으로 FPS/비행 스타일 전환 가능

### Notes

- Camera dirty 제거로 매 프레임 행렬 2개 계산 추가되나, 카메라 수가 적어 영향 무시 가능
- Transform dirty는 유지 (Entity 수가 많을 수 있음)
- CameraUpMode는 런타임에 변경 가능 (Inspector 또는 API)

### Next Steps

- [ ] Phase 5.1: PBR Pipeline 기초
- [ ] Phase 5.2: IBL (Image Based Lighting)

---

## 2025-01-07 - Phase 4.4 Refactoring 01: 코드 정리 및 Transform API 개선

### Overview

Phase 4.4 Multi-submesh 완료 후 Phase 5 PBR 진입 전 코드 정리.
ModelViewerApp 중복 제거, TransformSystem API 정리, worldInvTranspose 파이프라인 적용.

### Tasks

- [x] ModelViewerApp 코드 정리 (중복 제거, 버그 수정)
- [x] worldInvTranspose 렌더링 파이프라인 연결
- [x] TransformSystem 저수준 쓰기 API private 전환
- [x] TransformSystem::MarkDirty() public static 노출
- [x] TransformComponent eulerHint 추가 (Unity 방식)
- [x] ECSInspector Rotation 편집 안정화
- [x] 미사용 Camera 소스 솔루션에서 제외

### Decisions

**worldInvTranspose 변환 위치**
- 선택: DX12Renderer에서 일괄 Transpose
- 이유: RenderItem은 수학적 행렬만 저장, GPU 포맷 변환은 Renderer 책임
- 결과: 모든 행렬이 동일한 패턴으로 처리

```cpp
// RenderSystem - 수학적 행렬
renderItem.worldMatrix = worldMatrix;
renderItem.mvpMatrix = worldMatrix * viewProj;
renderItem.worldInvTranspose = GetWorldInvTranspose(*transform);

// DX12Renderer - GPU 포맷 변환
objectData.worldMatrix = MatrixTranspose(item.worldMatrix);
objectData.mvpMatrix = MatrixTranspose(item.mvpMatrix);
objectData.worldInvTranspose = MatrixTranspose(item.worldInvTranspose);
```

**TransformSystem API 접근 수준**
- 저수준 쓰기 API → private (SetRotationEulerInternal 등)
- 저수준 읽기 API → public 유지 (GetWorldMatrix, GetForward 등)
- MarkDirty() → public static (외부 직접 수정 시 호출)
- 이유: ECS 원칙 유지하면서 API 오용 방지

**Euler 캐시 방식 (Unity 방식 채택)**
- 선택: TransformComponent에 eulerHint 필드 추가
- 대안 A: Inspector 캐시 (빠른 구현, 제한적)
- 대안 B: Euler Primary (Unreal 방식, 성능 오버헤드)
- 이유: 산업 표준, 직렬화 지원, 일관된 Euler 접근

```cpp
// 동기화 규칙
SetRotationEuler(euler) → eulerHint = euler, rotation = Quat(euler)
SetRotation(quat)       → rotation = quat, eulerHint = quat.ToEuler()
Inspector 편집          → eulerHint 직접 수정 → rotation 동기화
```

### Issues Encountered

**ECSInspector Transform 수정 반영 안됨**
- 원인: 직접 수정 시 dirty flag 미설정
- 흐름: `transform->position = x` → localDirty = false → Update 스킵
- 해결: MarkDirty() public 노출, Inspector에서 수정 후 호출

**Rotation 드래그 시 값 점프**
- 원인: Quaternion ↔ Euler 변환의 비결정성
- 예시: Y=45 입력 후 Z=45 입력 → X,Y,Z 전체 변경
- 해결: eulerHint 캐시로 Quaternion→Euler 역변환 회피

**OnShutdown Entity 삭제 버그**
- 원인: Entity 삭제 후 다른 Entity 참조 시 무효 핸들
- 해결: Entity 배열에 복사 후 일괄 삭제

```cpp
// 수정 전 (버그)
if (mEntityA.IsValid()) mRegistry->DestroyEntity(mEntityA);
if (mEntityB.IsValid()) mRegistry->DestroyEntity(mEntityB);  // mEntityA 영향받을 수 있음

// 수정 후
Entity toDestroy[] = { mEntityA, mEntityB, ... };
for (auto& e : toDestroy) {
    if (e.IsValid()) mRegistry->DestroyEntity(e);
}
```

### Files Modified

| 파일 | 변경 내용 |
|------|-----------|
| TransformComponent.h | eulerHint 필드 추가 (12바이트) |
| TransformSystem.h | 저수준 쓰기 API private, MarkDirty public static |
| TransformSystem.cpp | eulerHint 동기화, 함수명 Internal 접미사 |
| ECSInspector.cpp | eulerHint 사용, MarkDirty 호출 |
| RenderTypes.h | RenderItem에 worldInvTranspose 추가 |
| RenderSystem.cpp | worldInvTranspose 계산 및 전달 |
| DX12Renderer.h | ObjectConstants에 worldInvTranspose 추가 |
| DX12Renderer.cpp | 행렬 Transpose 일괄 처리 |
| ModelViewerApp.h | 멤버 변수 정리 |
| ModelViewerApp.cpp | 중복 제거, OnShutdown 버그 수정 |

### Results

**Transform API 일관성**
- 고수준 API: Entity 기반, dirty 자동 설정
- 직접 수정: 멤버 수정 후 MarkDirty() 호출
- 읽기 전용: static 함수로 어디서든 접근

**Inspector Rotation 안정성**
- 값 입력 시 점프 현상 제거
- 드래그 시 부드러운 변경
- Gimbal Lock 근처에서도 안정적

**렌더링 파이프라인**
- 비균등 스케일 시 노멀/탄젠트 정확한 변환
- worldMatrix, mvpMatrix, worldInvTranspose 일관된 처리

### Notes

- eulerHint는 직렬화 시 저장하여 씬 파일 가독성 향상
- worldInvTranspose는 균등 스케일 시 worldMatrix 반환 (최적화)
- Camera 소스는 삭제하지 않고 솔루션에서만 제외 (향후 재사용)

### Next Steps

- [ ] Phase 5.1: PBR Pipeline 기초
- [ ] Phase 5.2: IBL (Image Based Lighting)

---

## 2026-01-06 - Phase 4.4: Multi-Submesh Support (Complete)

### Overview

하나의 메시가 여러 Material을 사용하는 경우(캐릭터의 피부, 옷, 장비 등) 서브메시별로 개별 렌더링할 수 있는 기능 구현. Phase 5 PBR Pipeline의 선행 요구사항.

### Tasks

**4.4.1 Graphics 레이어 확장**
- [x] Graphics/SubmeshInfo.h 신규 생성 (의존성 방향 정리)
- [x] MeshResource.DrawSubmesh() 구현
- [x] MeshResource.SetSubmeshes(), GetSubmeshCount() 추가
- [x] ResourceManager 서브메시 복사 로직

**4.4.2 ECS 레이어 확장**
- [x] MaterialComponent 배열 구조 변경 (MAX_MATERIALS = 8)
- [x] RenderItem.submeshIndex 추가
- [x] RenderSystem 서브메시 순회 로직
- [x] MeshResource Initialize에서 기본 서브메시 생성
- [x] ResourceManager 중복 SetSubmeshes 제거
- [x] ECSInspector 다중 Material 표시

**4.4.3 DX12Renderer 수정**
- [x] DrawRenderItems()에서 DrawSubmesh() 호출

**4.4.4 검증 및 테스트**
- [x] 멀티 서브메시 모델 테스트 (MultiMaterialCube - 3개 서브메시)
- [x] 서브메시별 다른 Material 적용 확인
- [x] 기존 Sphere, Helmet 정상 동작 확인
- [x] ModelViewerApp 4개 Entity 렌더링

### Decisions

**MaterialComponent 구조**
```cpp
struct MaterialComponent
{
    static constexpr uint32 MAX_MATERIALS = 8;
    ResourceId materialIds[MAX_MATERIALS];  // 64 bytes
    uint32 count = 1;                       // 4 bytes
    uint32 _padding = 0;                    // 4 bytes
};  // 총 72 bytes
```
- POD 유지, 고정 크기, 캐시 친화적
- 서브메시 > Material 수일 때 마지막 Material 반복
- 최대 8개 Material (대부분 게임 모델 커버)

**기본 서브메시 자동 생성**
- 모든 Initialize 함수에서 `CreateDefaultSubmesh()` 호출
- ProceduralSphere 등 직접 Initialize하는 메시 대응
- `GetSubmeshCount() >= 1` 항상 보장

**glTF 멀티 메시 → 서브메시 병합**
- Assimp가 멀티 머티리얼을 별도 메시(Primitive)로 파싱
- ModelViewerApp에서 모든 메시를 하나로 병합하고 서브메시 정보 생성

### Files Modified

| 파일 | 변경 내용 |
|------|----------|
| Graphics/SubmeshInfo.h | 신규 생성 |
| Graphics/MeshResource.h | 서브메시 멤버, DrawSubmesh, Getter/Setter |
| Graphics/MeshResource.cpp | DrawSubmesh 구현, Initialize에서 기본 서브메시 생성 |
| Graphics/RenderTypes.h | RenderItem.submeshIndex 추가 |
| Graphics/DX12/DX12Renderer.cpp | Draw → DrawSubmesh |
| Framework/Assets/MeshAsset.h | Graphics::SubmeshInfo include |
| Framework/Resources/ResourceManager.cpp | 멀티 서브메시만 SetSubmeshes |
| ECS/Components/MaterialComponent.h | 배열 구조, GetMaterial, SetMaterial |
| ECS/Systems/RenderSystem.cpp | 서브메시 순회 로직 |
| Framework/DebugUI/ECSInspector.cpp | 다중 Material 표시 |
| Samples/ModelViewerApp.h | Cube 관련 멤버, 함수 선언 추가 |
| Samples/ModelViewerApp.cpp | CreateCubeEntity, SetupCubeMesh, SetupCubeMaterial 구현 |

### Implementation Details

**RenderSystem 서브메시 순회**
```cpp
uint32 submeshCount = mesh->GetSubmeshCount();

for (uint32 i = 0; i < submeshCount; ++i)
{
    ResourceId matId = materialComp->GetMaterial(i);
    MaterialResource* material = GetMaterial(matId);

    RenderItem item;
    item.mesh = mesh;
    item.material = material;
    item.submeshIndex = i;
    mFrameData.opaqueItems.push_back(item);
}
```

**DX12Renderer 변경**
```cpp
// 변경 전
item.mesh->Draw(cmdList);

// 변경 후
item.mesh->DrawSubmesh(cmdList, item.submeshIndex);
```

**glTF 멀티 메시 병합 (ModelViewerApp)**
```cpp
for (size_t i = 0; i < modelData.meshes.size(); ++i)
{
    auto& meshData = modelData.meshes[i];

    Graphics::SubmeshInfo submesh;
    submesh.startIndex = baseIndex;
    submesh.indexCount = static_cast<uint32>(meshData.indices.size());
    submesh.baseVertex = baseVertex;
    submesh.materialIndex = static_cast<uint32>(i);
    submeshes.push_back(submesh);

    allVertices.insert(allVertices.end(), meshData.vertices.begin(), meshData.vertices.end());
    allIndices.insert(allIndices.end(), meshData.indices.begin(), meshData.indices.end());

    baseVertex += static_cast<uint32>(meshData.vertices.size());
    baseIndex += static_cast<uint32>(meshData.indices.size());
}
```

### Results

**ModelViewerApp 4개 Entity 렌더링**

| 위치 | Entity | 서브메시 |
|------|--------|----------|
| (-3.5, 0, 0) | Procedural Sphere | 1 |
| (0, 0, 0) | Loaded Sphere | 1 |
| (3.5, 0, 0) | DamagedHelmet | 1 |
| (0, 2.5, 0) | MultiMaterialCube | 3 |

- 단일 서브메시 모델 정상 렌더링
- 멀티 서브메시 모델 (MultiMaterialCube) 3개 Material 각각 적용
- 하위 호환성 유지 (기존 코드 변경 최소화)

### Notes

- ProceduralSphere는 MeshAsset 경유 없이 직접 Initialize하므로 기본 서브메시 필수
- Assimp는 빈 기본 머티리얼을 자동 추가함 (무시해도 무방)
- MaterialComponent.GetMaterial()에서 부족한 Material은 마지막 Material 반복

### Next Steps

- [ ] Phase 5: PBR Pipeline

---

## 2026-01-06 - Phase 4.4.1: Graphics 레이어 서브메시 지원

### Overview

PBR 모델의 서브메시별 Material 적용을 위한 Graphics 레이어 기반 구축. 기존 MeshResource는 전체 메시를 한 번에 렌더링하는 `Draw()`만 제공했으나, 서브메시별 개별 렌더링이 가능하도록 확장.

### Tasks

- [x] SubmeshInfo를 Framework에서 Graphics 모듈로 이동 (의존성 방향 정리)
- [x] MeshResource 서브메시 지원 (DrawSubmesh, SetSubmeshes, GetSubmeshCount)
- [x] ResourceManager에서 MeshAsset → MeshResource 서브메시 복사

### 구현 상세

**SubmeshInfo 위치 이동**

```
변경 전: Framework::SubmeshInfo (MeshAsset.h)
         → Graphics에서 사용 불가 (순환 의존성)

변경 후: Graphics::SubmeshInfo (SubmeshInfo.h)
         → Framework가 Graphics를 include (정상적인 의존성 방향)
```

**MeshResource 확장**

| 추가 항목 | 설명 |
|----------|------|
| `std::vector<SubmeshInfo> mSubmeshes` | 서브메시 정보 저장 |
| `DrawSubmesh(cmdList, index)` | 특정 서브메시만 렌더링 |
| `SetSubmeshes(submeshes)` | 서브메시 정보 설정 |
| `GetSubmeshCount()` | 서브메시 개수 반환 (최소 1 보장) |
| `CreateDefaultSubmesh()` | 서브메시 없을 때 전체를 단일 서브메시로 등록 |

**DrawSubmesh 동작**

```cpp
void MeshResource::DrawSubmesh(ID3D12GraphicsCommandList* cmdList, uint32 submeshIndex) const
{
    const SubmeshInfo& submesh = mSubmeshes[submeshIndex];
    
    cmdList->IASetVertexBuffers(0, 1, &vbView);
    cmdList->IASetIndexBuffer(&ibView);
    cmdList->DrawIndexedInstanced(
        submesh.indexCount,   // 이 서브메시의 인덱스 개수
        1,
        submesh.startIndex,   // 인덱스 버퍼 내 시작 위치
        submesh.baseVertex,   // 정점 버퍼 오프셋
        0
    );
}
```

### Decisions

**서브메시 없는 메시도 GetSubmeshCount() == 1 보장**

- RenderSystem에서 분기 없이 일관된 처리 가능
- `CreateDefaultSubmesh()`가 전체 인덱스 범위를 단일 서브메시로 등록

**의존성 방향 정리**

- 모듈 의존성은 `Framework → Graphics` 방향이어야 함
- Graphics가 Framework에 의존하면 순환 참조 위험

### Files Modified

| 파일 | 변경 내용 |
|------|----------|
| Graphics/SubmeshInfo.h | 신규 생성 |
| Graphics/MeshResource.h | 서브메시 멤버, DrawSubmesh, Getter/Setter |
| Graphics/MeshResource.cpp | DrawSubmesh 구현, CreateDefaultSubmesh |
| Framework/Assets/MeshAsset.h | `#include "Graphics/SubmeshInfo.h"`, 기존 SubmeshInfo 제거 |
| Framework/Assets/MeshAsset.cpp | `Graphics::SubmeshInfo` 타입 사용 |
| Framework/Assets/ModelLoader.h | `Graphics::SubmeshInfo` 타입 사용 |
| Framework/Resources/ResourceManager.cpp | 서브메시 복사 로직 추가 |

### Data Flow

```
[glTF 파일]
    ↓ ModelLoader::LoadModel()
LoadedMeshData.submeshes (파싱)
    ↓ MeshAsset::SetSubmeshes()
MeshAsset.mSubmeshes (CPU)
    ↓ ResourceManager::CreateMeshFromAsset()
MeshResource.mSubmeshes (GPU 측 메타데이터)
    ↓ RenderSystem (Phase 4.4.2에서 구현)
DrawSubmesh() 호출
```

### Results

- 서브메시별 Material 적용 기반 완성
- 기존 `Draw()` 함수 하위 호환성 유지
- 서브메시 유무와 관계없이 동일한 렌더링 API

### Next Steps

- [ ] Phase 4.4.2: ECS 레이어 확장
- [ ] Phase 4.4.3: DX12Renderer 수정
- [ ] Phase 4.4.4: 검증 및 테스트
- [ ] Phase 5: PBR Pipeline

---

## 2025-01-04 - Phase 4.3: Texture Pipeline (Step 3~5 완료)

### Tasks

- [x] Step 3: TextureAsset 메타데이터 확장
- [x] Step 4: Asset/Resource 소유권 분리
- [x] Step 5: ImGui 텍스처 정보 패널

### 구현 상세

**Step 3: TextureAsset 메타데이터**
```cpp
// 추가 멤버
Graphics::TextureType mTextureType;
uint32 mWidth, mHeight;
DXGI_FORMAT mFormat;
bool mIsSRGB;

// ResourceManager만 설정 가능 (friend)
void SetMetadata(uint32 w, uint32 h, DXGI_FORMAT fmt, bool srgb);
void SetTextureType(Graphics::TextureType type);
```

**Step 4: 소유권 분리**
```
AssetManager (CPU)          ResourceManager (GPU)
├─ TextureAsset       ←──── RegisterTextureAsset()
├─ MeshAsset                ├─ TextureResource
└─ MaterialAsset            └─ MeshResource
```

초기화 순서:
```cpp
mResourceManager = make_unique<ResourceManager>(device, renderer);
mAssetManager = make_unique<AssetManager>(mResourceManager.get());
mAssetManager->Initialize();
mResourceManager->SetAssetManager(mAssetManager.get());  // 양방향 연결
```

**Step 5: ImGui 패널**
- `DXGIFormatToString()`: 포맷 문자열 변환
- `TextTextureStatus()`: 메타데이터 인라인 표시
- `RenderModelInfoPanelTextures()`: 테이블 뷰 (Type, Size, Format, Color Space)

### Decisions

- **메타데이터만 저장**: WICTextureLoader가 ID3D12Device 필수, CPU 전용 로딩 불가
- **friend class**: SetMetadata()는 ResourceManager만 호출 가능하도록 제한
- **폴백 텍스처**: AssetManager에 미등록 (특수 내부 리소스)

### Files Modified

| 파일 | 변경 |
|------|------|
| TextureAsset.h/cpp | 메타데이터 멤버, Getter/Setter, friend 선언 |
| AssetManager.h/cpp | Register/Unregister/GetTextureAsset |
| ResourceManager.h/cpp | mAssetManager 연결, GetTextureAsset 위임 |
| ModelViewerApp.cpp | DXGIFormatToString, RenderModelInfoPanelTextures |

### Results

```
▶ Loaded Textures
  Type      Size       Format      Space
  ────────────────────────────────────────
  Albedo    2048x2048  R8G8B8A8    sRGB
  Normal    2048x2048  R8G8B8A8    Linear
  Total: 5 textures
```

### Next Steps

- [ ] Phase 5: PBR Pipeline
- [ ] Phase 6: Shader System (DXC, Hot Reload)

---

## 2026-01-03 - Phase 4.3.2: Asset/Resource 네이밍 변경

### Tasks

- [x] Graphics 모듈 클래스 네이밍 변경
- [x] 참조 파일 수정 및 빌드 확인

### Changes

**파일명 및 클래스명 변경 (Graphics 모듈)**

| 변경 전 | 변경 후 |
|---------|---------|
| Mesh.h/cpp | MeshResource.h/cpp |
| Texture.h/cpp | TextureResource.h/cpp |
| Material.h/cpp | MaterialResource.h/cpp |

**유지 (Framework 모듈)**

| 클래스 | 비고 |
|--------|------|
| MeshAsset | CPU 데이터 |
| TextureAsset | CPU 데이터 (Step 3에서 확장 예정) |
| MaterialAsset | CPU 데이터 |

### Decisions

**Asset/Resource 역할 분리 명확화**

| 구분 | Asset | Resource |
|------|-------|----------|
| 역할 | 원본 자산 (파일 로드) | GPU 리소스 (렌더링) |
| 위치 | CPU 메모리 | GPU 메모리 |
| 모듈 | Framework | Graphics |

### Files Modified

| 주요 파일 | 변경 내용 |
|------|----------|
| MeshResource.h/cpp | 클래스명 Mesh → MeshResource |
| TextureResource.h/cpp | 클래스명 Texture → TextureResource |
| MaterialResource.h/cpp | 클래스명 Material → MaterialResource |
| Graphics.vcxproj | 파일 경로 수정 |
| Graphics.vcxproj.filters | 파일 경로 수정 |
| ResourceManager.h/cpp | 전방선언 및 참조 수정 |
| DX12PipelineStateCache.h/cpp | 전방선언 및 참조 수정 |
| RenderSystem.h/cpp | 전방선언 및 참조 수정 |

### Results

- Asset(CPU)과 Resource(GPU) 역할 네이밍으로 명확히 구분
- 기존 기능 동작 확인

### Next Steps

- [ ] Phase 4.3.3: TextureAsset 역할 확장 (로딩 로직 이동)
- [ ] Phase 4.3.4: TextureResource 정리 (GPU 생성만 담당)
- [ ] Phase 4.3.5: 검증 및 ImGui 텍스처 정보 표시

---

## 2026-01-02 - Phase 4.3.1: sRGB/Linear 색공간 처리

### Overview

텍스처 로딩 시 용도(TextureType)에 따라 sRGB/Linear 색공간을 자동으로 처리하는 기능 구현. 물리 기반 렌더링(PBR)의 기초가 되는 올바른 색공간 파이프라인 확립.

### Tasks

- [x] TextureType.h에 `IsSRGBTexture()` 헬퍼 함수 추가
- [x] Texture 클래스 WICTextureLoaderEx API 적용
- [x] Texture 클래스 `mIsSRGB` 멤버 및 Getter 추가
- [x] ResourceManager TextureType 필수 오버로드 구현
- [x] ModelViewerApp 호출부 TextureType 명시
- [x] 기존 TextureType 미지정 API 삭제 (강제화)

### Decisions

**색공간 분류 기준**
- sRGB (감마 보정 필요): Albedo, Emissive
- Linear (데이터 텍스처): Normal, Roughness, Metallic, AmbientOcclusion, Specular

```cpp
inline constexpr bool IsSRGBTexture(TextureType type)
{
    switch (type)
    {
    case TextureType::Albedo:
    case TextureType::Emissive:
        return true;
    default:
        return false;
    }
}
```

**WIC Loader 플래그 전략**
- sRGB 텍스처: `WIC_LOADER_FORCE_SRGB` → GPU가 샘플링 시 자동 Linear 변환
- Linear 텍스처: `WIC_LOADER_IGNORE_SRGB` → 변환 없이 원본 데이터 유지

**API 강제화 결정**
- TextureType 미지정 API 완전 삭제 (권장→강제)
- 이유: 개인 프로젝트, 호출부 이미 수정 완료, 색공간 실수 원천 차단
- 삭제 대상:
  - `LoadTexture(path)` → `LoadTexture(path, TextureType)`
  - `LoadTextureW(path)` → `LoadTextureW(path, TextureType)`
  - `LoadTextureFromMemory(name, data, size)` → `LoadTextureFromMemory(name, data, size, TextureType)`
  - `CreateTextureFromMemory(name, data, w, h, DXGI_FORMAT)` → `CreateTextureFromMemory(name, data, w, h, TextureType)`

**출력 감마 보정 방식**
- 현재: `pow(finalColor, 1/2.2)` 유지 (임시)
- 이유: sRGB RTV 적용 시 ImGui/DebugRenderer 전체 수정 필요
- 계획: Phase 5.1 HDR Pipeline에서 톤매핑과 함께 처리

### Issues Encountered

**DDS 파일의 sRGB 처리**
- 문제: DDS는 별도 sRGB 플래그 필요 없음
- 이유: DDS 메타데이터에 포맷 정보 포함 (BC7_UNORM_SRGB 등)
- 해결: DDSTextureLoader가 포맷에서 자동 감지, LoadFromDDS에서 mIsSRGB 설정

**RGBA 원시 데이터 색공간**
- 문제: `CreateTextureFromMemory`는 DXGI_FORMAT만 받아 색공간 불명확
- 해결: TextureType 오버로드 추가, 내부에서 포맷 자동 결정
  - Albedo → `DXGI_FORMAT_R8G8B8A8_UNORM_SRGB`
  - Normal → `DXGI_FORMAT_R8G8B8A8_UNORM`

**폴백 텍스처 예외 처리**
- 문제: 폴백 텍스처는 TextureType 없이 생성
- 해결: `Texture::CreateFromMemory(..DXGI_FORMAT)` 유지 (내부용)
- ResourceManager에서 직접 호출하므로 public API 영향 없음

### Files Modified

| 파일 | 변경 내용 |
|------|----------|
| TextureType.h | `IsSRGBTexture()` 헬퍼 함수 추가 |
| Texture.h | TextureType 파라미터 오버로드, `mIsSRGB` 멤버, `IsSRGB()` Getter |
| Texture.cpp | `LoadWICTextureFromFileEx`, `LoadWICTextureFromMemoryEx` 적용, sRGB 플래그 처리 |
| ResourceManager.h | TextureType 필수 오버로드만 유지, 기존 API 삭제 |
| ResourceManager.cpp | 오버로드 구현, DXGI_FORMAT 버전 통합, 로그에 sRGB 정보 포함 |
| ModelViewerApp.cpp | `SetupSharedMaterial()`, `SetupHelmetMaterial()` TextureType 명시 |

### Results

**로그 출력 확인**
```
[Texture] Loading WIC texture: BaseColor.png (Type: Albedo, sRGB: Yes)
[Texture] WIC texture loaded (1024x1024, Format: 91, sRGB: Yes)
[ResourceManager] Loaded texture: BaseColor.png (Type: Albedo, sRGB: Yes)

[Texture] Loading WIC texture: Normal.png (Type: Normal, sRGB: No)
[Texture] WIC texture loaded (1024x1024, Format: 87, sRGB: No)
[ResourceManager] Loaded texture: Normal.png (Type: Normal, sRGB: No)
```

**DXGI_FORMAT 확인**
- Format 87: `DXGI_FORMAT_B8G8R8A8_UNORM` (Linear)
- Format 91: `DXGI_FORMAT_B8G8R8A8_UNORM_SRGB` (sRGB)

**색공간 파이프라인**
```
[입력]                      [처리]                     [출력]
sRGB 텍스처 ──→ GPU Linear 변환 ──→ 조명 계산 ──→ pow(1/2.2) ──→ 모니터
      ↑         (SRV 자동)                         ↑
  Phase 4.3.1                                 Phase 5.1에서
  WIC_LOADER_FORCE_SRGB                      HDR + 톤매핑으로 대체
```

### Notes

- WICTextureLoader Extended 함수: `LoadWICTextureFromFileEx`, `LoadWICTextureFromMemoryEx`
- DDS 파일은 이미 색공간 정보 포함하므로 TextureType 불필요
- 폴백 텍스처(1x1 Magenta)는 UNORM으로 생성 (sRGB 불필요)
- `pow(1/2.2)`는 근사치, 정확한 sRGB 변환은 하드웨어(sRGB RTV) 사용 권장

### Next Steps

- [ ] Phase 4.3.2: TextureAsset 메타데이터 확장 (선택적)
- [ ] Phase 4.3.3: ImGui 텍스처 정보 표시 (선택적)
- [ ] Phase 4.3.4: texconv 활용 가이드 문서 (선택적)
- [ ] Phase 4.4: Descriptor 관리 고도화
- [ ] Phase 5.1: HDR Pipeline 구축 (pow 제거, 톤매핑)

---

## 2025-12-31 - Phase 4.2: Model Loading (Step 4~5 완료)

### Tasks

- [x] Step 4: glTF 텍스처 자동 로딩
  - [x] LoadedMaterialData → GPU Material 텍스처 바인딩
  - [x] 폴백 텍스처 시스템 (1x1 Magenta)
  - [x] 임베디드 텍스처 지원 (glb 파일)
  - [x] 다중 머티리얼 Constant Buffer 지원
- [x] Step 5: Normal Map 검증
  - [x] MikkTSpace 탄젠트 + Normal Map 조명 테스트
  - [x] 조명 회전 시 음영 변화 확인

### 구현 상세

**폴백 텍스처 시스템**
- `Texture::CreateFromMemory()`: 원시 RGBA 데이터로 텍스처 생성
- `ResourceManager::CreateFallbackTexture()`: 생성자에서 1x1 Magenta 자동 생성
- `ResourceManager::GetFallbackTexture()`: 폴백 텍스처 ID 반환
- 텍스처 로드 실패 시 Magenta로 시각적 식별 가능

**임베디드 텍스처 지원**
- `LoadedTextureInfo` 확장:
  ```cpp
  std::vector<Core::uint8> embeddedData;  // PNG/JPG 바이너리 또는 RGBA 원시
  Core::uint32 width, height;              // 원시 데이터일 때만 유효
  bool isEmbedded = false;
  bool isCompressed = true;                // true: PNG/JPG, false: RGBA
  ```
- `ModelLoader::ExtractEmbeddedTexture()`: aiScene에서 텍스처 바이너리 추출
- `Texture::LoadFromMemory()`: WIC로 압축 포맷(PNG/JPG) 디코딩
- `ResourceManager::LoadTextureFromMemory()`: 압축 데이터 로드 API

**다중 머티리얼 Constant Buffer**
- `mCurrentMaterialCBIndex`: 프레임 내 머티리얼 슬롯 인덱스
- `ALIGNED_MATERIAL_SIZE = 256`: D3D12 정렬 요구사항 준수
- Object CB와 동일한 패턴으로 일관성 유지
- `BeginFrame()`에서 인덱스 리셋

### Decisions

**폴백 텍스처 생성 위치**
- ResourceManager 생성자에서 자동 생성
- 외부 파일 의존성 없음 (절차적 생성)
- 삭제 방지 로직 추가 (`RemoveTexture()`)

**임베디드 텍스처 데이터 흐름**
```
glb 파일
    ↓ Assimp
aiScene->mTextures[index]
    ↓ ExtractEmbeddedTexture()
LoadedTextureInfo.embeddedData
    ↓ ResourceManager::LoadTextureFromMemory()
Texture::LoadFromMemory() (WIC 디코딩)
    ↓
GPU Texture
```

**Constant Buffer 인덱스 리셋 위치**
- `BeginFrame()`에서 리셋 유지 (프레임 리소스 의미론)
- `RenderScene()`에 assert 추가 권장 (방어적 검증)

### Issues Encountered

**WIC 메모리 로드**
- 문제: `LoadWICTextureFromFile()`은 파일 경로만 지원
- 해결: `LoadWICTextureFromMemory()` 사용 (DirectXTK12 제공)

**aiTexture 데이터 형식**
- `mHeight == 0`: 압축 포맷 (PNG/JPG), `mWidth`가 바이트 크기
- `mHeight > 0`: 원시 ARGB 데이터, RGBA로 변환 필요
- aiTexel은 ARGB 순서이므로 복사 시 채널 재배치

### Files Modified

| 파일 | 변경 내용 |
|------|----------|
| Texture.h/cpp | `CreateFromMemory()`, `LoadFromMemory()` 추가 |
| ResourceManager.h/cpp | 폴백 텍스처, `LoadTextureFromMemory()`, `CreateTextureFromMemory()` |
| ModelLoader.h/cpp | `LoadedTextureInfo` 확장, `ExtractEmbeddedTexture()` |
| DX12Renderer.h/cpp | Material CB 인덱싱, `ALIGNED_MATERIAL_SIZE` 상수화 |
| ModelViewerApp.cpp | `SetupHelmetMaterial()` 임베디드/외부/폴백 분기 처리 |

### Results

- DamagedHelmet.gltf 외부 텍스처 정상 로드
- glb 파일 임베디드 텍스처 정상 로드
- 텍스처 누락 시 Magenta 폴백 표시
- Normal Map + Tangent 조명 정상 동작
- 다중 오브젝트 렌더링 시 머티리얼 독립 적용

### Notes

- `LoadWICTextureFromMemory()`는 DirectXTK12의 WICTextureLoader.h에 포함
- 임베디드 텍스처 이름은 "ModelName_*index" 형식으로 해시 충돌 방지
- Phase 4.3에서 DDS/BCn 압축 포맷, sRGB 처리 예정

### Next Steps

- [ ] Phase 4.2 완료 문서화 (Architecture.md 업데이트)
- [ ] Phase 4.3: Texture Pipeline (DDS, BCn, sRGB)
- [ ] 서브메시 지원 (Phase 4.7 또는 별도 Phase)

---

## 2025-12-30 - Phase 4.2: Model Loading (Step 1~3 완료)

### Tasks

- [x] Assimp 라이브러리 통합 (vcpkg, 의존성 격리)
- [x] ModelLoader 클래스 구현 (LoadMesh, LoadModel)
- [x] LoadedMeshData, LoadedMaterialData, LoadedTextureInfo 구조체 정의
- [x] StandardVertex.tangent Vector3→Vector4 확장 (w = bitangent 부호)
- [x] MikkTSpaceCalculator 구현 (업계 표준 탄젠트 계산)
- [x] 좌표계 변환 파이프라인 확립 (Blender→glTF→DirectX)
- [x] PrimitiveGenerator 와인딩 순서 CCW→CW 수정
- [x] SubmeshInfo 구조체 정의 (MeshAsset.h 단일 정의)
- [x] MeshAsset 확장 (Bounding Box, MeshDataPolicy, ReleaseSourceData)
- [x] ResourceManager::CreateMeshFromAsset() 구현
- [x] Mesh::InitializeStandard32() 추가 (32비트 인덱스)
- [x] 11_ModelViewer 3개 오브젝트 렌더링 (Procedural, Loaded, Helmet)

### Decisions

**Assimp Import Flags**
- `aiProcess_ConvertToLeftHanded` 사용으로 수동 좌표 변환 제거
- glTF(오른손)→DirectX(왼손) 자동 변환

**탄젠트 계산**
- MikkTSpace 사용 (Unreal, Unity 표준)
- bitangent는 tangent.w 부호로 셰이더에서 재구성

**SubmeshInfo 단일 정의**
- ModelLoader.h와 MeshAsset.h 중복 정의로 타입 불일치 발생
- MeshAsset.h에 통합, ModelLoader에서 include

**인덱스 포맷 자동 선택**
- vertexCount ≤ 65535 → 16비트 (메모리 절약)
- vertexCount > 65535 → 32비트

### Issues Encountered

**와인딩 순서 불일치**
- 문제: Procedural Sphere 정상, Loaded Sphere 노말 반전
- 원인: Procedural은 CCW, Loaded는 CW (Assimp 변환)
- 해결: PrimitiveGenerator를 CW로 통일

### Notes

- DX12IndexBuffer는 이미 R32_UINT 지원 (수정 불필요)
- Asset Pipeline: LoadedMeshData→MeshAsset→GPU Mesh 흐름 확립
- std::move 활용으로 대용량 메시 복사 방지

### Next Steps

- [ ] Step 4: glTF 텍스처 자동 로딩
- [ ] LoadedMaterialData → GPU Material 변환
- [ ] DamagedHelmet 실제 텍스처 적용
- [ ] Phase 4.3: Texture Pipeline (DDS, BCn)

---

## 2025-12-26 - Phase 4.1: Asset Loading Foundation

### Tasks

- [x] AssetTypes.h, IAsset.h (열거형, 인터페이스)
- [x] Asset 클래스 5개 (MeshAsset, TextureAsset, MaterialAsset, ModelAsset, AnimationAsset)
- [x] AssetManager (ResourceId 기반 관리, 기본 Asset 시스템)
- [x] 11_ModelViewer 샘플 (프로시저럴 UV Sphere, ImGui 패널)

### Decisions

**ResourceId vs AssetHandle**
- ResourceId 채택: 기존 ResourceManager 패턴 일관성, ECS POD 원칙 (8B vs 16B)
- AssetHandle은 필요 시 선택적 래퍼로 추가 가능

**TextureFormat**
- DXGI_FORMAT 직접 사용 (자체 열거형 제거)
- 모든 DirectX 포맷 지원, 변환 로직 불필요

**Asset vs Resource 분리**
- Asset: CPU 데이터 (파일 로딩)
- Resource: GPU 리소스 (렌더링)
- Phase 4.2+에서 Asset → Resource 변환 구현 예정

### Issues Resolved

**템플릿 특수화 링크 에러 (LNK2005)**
- 문제: `GetDefaultAssetId<T>()` 특수화 중복 정의
- 해결: 헤더 끝에 특수화 선언 추가

```cpp
template<> ResourceId AssetManager::GetDefaultAssetId<MeshAsset>() const;
template<> ResourceId AssetManager::GetDefaultAssetId<TextureAsset>() const;
// ...
```

### Results

- 회색 UV Sphere Y축 회전, Directional Light 조명
- AssetManager 패널: 기본 Asset 4개 표시
- Sphere Control 패널: Segments/Rings 실시간 변경

### Next Steps

- [ ] Phase 4.2: Model Loading (glTF 2.0, Assimp)
- [ ] Phase 4.3: Texture Pipeline (DDS, BCn 압축)
- [ ] Phase 4.4: Asset → Resource 변환 통합

---

**최종 업데이트**: 2025-01-07