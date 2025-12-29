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