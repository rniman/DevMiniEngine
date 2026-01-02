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

**최종 업데이트**: 2025-12-29