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

**최종 업데이트**: 2025-12-29