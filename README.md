# DevMiniEngine

**한국어** | **[English](./README_ENG.md)**

**DirectX 12 기반 학습용 미니 게임 엔진**

## 프로젝트 개요

DevMiniEngine은 DirectX 12를 기반으로 하는 개인 학습 및 포트폴리오 목적의 게임 엔진 프로젝트입니다. ECS(Entity Component System) 아키텍처와 현대적인 C++ 디자인 패턴을 학습하고 실험하기 위해 제작되었습니다.

### 주요 목표

- **ECS 아키텍처 학습**: 데이터 지향 설계와 엔티티-컴포넌트 시스템 구현
- **최신 그래픽스 기술**: DirectX 12를 활용한 현대적 렌더링 파이프라인 구축
- **게임 엔진 기초**: 물리, AI, 지형 생성 등 핵심 엔진 시스템 구현
- **포트폴리오 제작**: 실무 수준의 코드 품질과 문서화

## 구현 현황

### 완료된 시스템

**Phase 1: 기반 시스템 (100% 완료)**

**Core 시스템**
- 메모리 관리 시스템
  - LinearAllocator: O(1) 범프 포인터 할당
  - PoolAllocator: O(1) 고정 크기 객체 풀링
  - StackAllocator: 마커를 사용한 LIFO 할당
- 타이밍 시스템
  - 고정밀 타이머 (QueryPerformanceCounter)
  - 프레임 시간 평균화 (50개 샘플)
  - FPS 측정 및 프로파일링 도구
- 로깅 시스템
  - 다중 출력 Sink 아키텍처 (Console, File)
  - 카테고리 기반 필터링
  - 컬러 코딩된 콘솔 출력

**Math 라이브러리**
- SIMD 최적화 벡터/행렬 연산 (DirectXMath 래퍼)
- Vector2, Vector3, Vector4
- Matrix4x4 (행 우선)
- Quaternion 연산
- Transform 유틸리티
- 카메라 행렬 (LookAt, Perspective)

**Platform 레이어**
- Win32 윈도우 생성 및 관리
- 이벤트 처리 시스템
- 입력 시스템 (키보드, 마우스)

**Phase 2: DirectX 12 그래픽스 & Framework (100% 완료)**

**DirectX 12 렌더링**
- 완전한 렌더링 파이프라인
  - Vertex/Index Buffer
  - Shader 컴파일 시스템
  - Pipeline State Object (PSO)
  - Root Signature
  - Constant Buffer
  - Depth-Stencil Buffer
  - DX12Renderer
    - 모든 렌더링 리소스 소유
    - Scene과 렌더링 책임 분리
- 멀티 텍스처 지원 (7개 타입)
  - Diffuse, Normal, Specular, Roughness, Metallic, AO, Emissive
- 3D 변환 및 카메라
  - MVP 행렬 변환
  - PerspectiveCamera 구현

**Framework 아키텍처**
- Application 라이프사이클 관리
  - 템플릿 메서드 패턴
  - 엔진/사용자 초기화 분리
- ResourceManager
  - Mesh, Material, Texture 중앙 관리
  - 캐싱 및 중복 로딩 방지
- Scene/GameObject 시스템
  - Transform 계층 구조
  - 렌더링 데이터 수집 (What/How 분리)

### 프로젝트 통계

- 총 코드 라인 수: ~5,000+
- 구현된 모듈: 5개 (Core, Math, Platform, Graphics, Framework)
- 테스트 커버리지: 8개 샘플 프로젝트
- 컴파일러 경고: 0개 (Level 4)

## 프로젝트 구조
```
DevMiniEngine/
├── Engine/                          # 엔진 코어
│   ├── include/                     # 모든 모듈의 public 헤더
│   │   ├── Core/                    # Core 헤더
│   │   │   ├── Memory/              # 메모리 관리
│   │   │   ├── Logging/             # 로깅 시스템
│   │   │   └── Timing/              # 타이머 시스템
│   │   ├── Math/                    # Math 라이브러리
│   │   ├── Platform/                # Platform 레이어
│   │   ├── Graphics/                # 그래픽스
│   │   │   ├── DX12/                # DirectX 12
│   │   │   ├── Camera/              # 카메라 시스템
│   │   │   └── RenderTypes.h        # 렌더링 타입
│   │   └── Framework/               # Framework 레이어
│   │       ├── Application.h        # 애플리케이션 베이스
│   │       ├── Resources/           # 리소스 관리
│   │       └── Scene/               # 씬 관리
│   │
│   ├── src/                         # 모든 모듈의 구현
│   │   └── (동일한 구조)
│   │
│   ├── Core/                        # Core 모듈 프로젝트
│   ├── Math/                        # Math 모듈 프로젝트
│   ├── Platform/                    # Platform 모듈 프로젝트
│   ├── Graphics/                    # Graphics 모듈 프로젝트
│   └── Framework/                   # Framework 모듈 프로젝트
│
├── Samples/                         # 샘플 프로젝트
│   ├── 01_MemoryTest/
│   ├── ...
│   └── 08_TexturedCube/             # 텍스처 큐브 렌더링
│
├── Assets/                          # 에셋
│   └── Textures/                    # 텍스처 파일
│       └── BrickWall/               # PBR 텍스처 세트
│
└── Docs/                            # 문서
    ├── Architecture.md
    ├── CodingConvention.md
    └── DevelopmentLog.md
```

## 빌드 및 실행

### 요구 사항

- **OS**: Windows 10/11 (64-bit)
- **IDE**: Visual Studio 2022 이상
- **SDK**: Windows 10 SDK (10.0.19041.0 이상)
- **DirectX**: DirectX 12 지원 GPU
- **C++ 표준**: C++20

### 빌드 방법

1. **저장소 클론**
```bash
git clone https://github.com/rniman/DevMiniEngine.git
cd DevMiniEngine
```

2. **솔루션 열기**
```bash
DevMiniEngine.sln
```

3. **빌드 구성**
   - `Debug` - 디버깅용
   - `Release` - 최적화 빌드

4. **빌드 실행**
   - `Ctrl + Shift + B`

### 실행 예시
```bash
# 텍스처 큐브 샘플 실행
bin/Debug/08_TexturedCube.exe
```

## 로드맵

### Phase 1-2: 기반 시스템 (100% 완료)
- [x] 프로젝트 구조
- [x] Core 시스템 (메모리, 로깅, 타이머)
- [x] Math 라이브러리 (SIMD)
- [x] Platform 레이어 (Window, Input)
- [x] DirectX 12 초기화 및 렌더링 파이프라인
- [x] 메시 및 텍스처 시스템
- [x] 카메라 시스템
- [x] Framework 아키텍처 (Application, ResourceManager, Scene)

**현재 상태:** PBR 텍스처 셋(Diffuse, Normal, Metallic, Roughness 등 7종)의 완벽한 로딩 및 셰이더 바인딩 파이프라인 구축 완료. (현재 렌더링은 Diffuse 맵을 기준으로 하며, 로드된 PBR 맵들을 활용하는 셰이더 구현은 다음 단계임)

---

### Phase 3: ECS 아키텍처 & 디버그 툴
**목표:** 데이터 지향 설계의 핵심 구현

- [ ] **ECS Core**
  - [ ] Entity Manager (생성/삭제/재활용)
  - [ ] Component Storage (Archetype 기반)
  - [ ] System Framework (실행 순서 관리)

- [ ] **Core Components & Systems**
  - [ ] TransformComponent (계층 구조, World Matrix 캐싱)
  - [ ] TransformSystem (Dirty Flag 전파)
  - [ ] MeshComponent & MaterialComponent
  - [ ] RenderSystem (ECS 기반 렌더링)

- [ ] **기초 조명 시스템 (Phong)**
  - [ ] DirectionalLightComponent
  - [ ] PointLightComponent  
  - [ ] LightingSystem (Phong Shading)
  - [ ] Normal Map 지원 (TBN 행렬)

- [ ] **Query System**
  - [ ] 컴포넌트 조합 쿼리
  - [ ] Query 캐싱 및 최적화

- [ ] **디버그 툴 (조기 도입)**
  - [ ] ImGui 통합
  - [ ] ECS Inspector (Entity/Component 편집)
  - [ ] 조명 파라미터 실시간 조정
  - [ ] 성능 모니터링 (FPS, Draw Call)

**완료 시:** 1000개 Entity 관리, Phong 조명, ImGui로 실시간 편집

---

### Phase 3.5: Job System (선택적)
**목표:** 기본 멀티스레딩 인프라

- [ ] 워커 스레드 풀
- [ ] Job 디스패처
- [ ] TransformSystem 병렬화
- [ ] 성능 벤치마크 (Single vs Multi-thread)

**참고:** Phase 9로 연기 가능 (병렬화할 작업이 충분해진 후)

---

### Phase 4: DX12 인프라 확장 & Asset Pipeline
**목표:** 생산성 향상을 위한 에셋 파이프라인

- [ ] **Descriptor 관리 고도화**
  - [ ] SRV/UAV Descriptor Heap 통합
  - [ ] Descriptor 풀 및 재활용
  - [ ] Frame-based 할당
- [ ] **리소스 업로드 최적화**
  - [ ] Upload 버퍼 링
  - [ ] Async Copy Queue (선택적)
- [ ] **Asset Pipeline**
  - [ ] glTF 2.0 로더 (메쉬, PBR 재질, 계층구조)
  - [ ] DDS/BCn 텍스처 파이프라인
  - [ ] Mipmap 생성 자동화
- [ ] **Shader 시스템 확장**
  - [ ] DXC 컴파일러 (Shader Model 6.6+)
  - [ ] Shader Reflection (자동 Root Signature)
  - [ ] Hot Reload (선택적)
- [ ] **ECS 렌더링 통합**
  - [ ] MeshRendererComponent
  - [ ] RenderSystem (ECS 쿼리 기반)

**완료 시:** Blender 모델 로딩, ECS 기반 렌더링

---

### Phase 4.5: Frame Graph & Culling
**목표:** 효율적인 렌더링 파이프라인

- [ ] **Frame Graph**
  - [ ] Pass/Resource 의존성 그래프
  - [ ] 자동 Barrier 삽입
  - [ ] Resource Lifetime 관리
- [ ] **Culling System**
  - [ ] Frustum Culling (CPU)
  - [ ] AABB Bounding Box
- [ ] **Asset Pipeline 확장 (선택적)**
  - [ ] Hot Reload (텍스처, 셰이더)

**완료 시:** 1000개 오브젝트 효율적 렌더링

---

### Phase 5: PBR & Post-Processing
**목표:** 사실적인 렌더링

- [ ] **PBR Material**
  - [ ] Metallic-Roughness 워크플로우
  - [ ] Cook-Torrance BRDF
  - [ ] IBL (Skybox, PMREM, Irradiance Map)
  - [ ] Uber Shader & Permutations
- [ ] **Post-Processing**
  - [ ] 톤매핑 (Reinhard, ACES)
  - [ ] Auto Exposure
  - [ ] Bloom
  - [ ] TAA (Temporal Anti-Aliasing)

**완료 시:** PBR 재질의 다양한 오브젝트, IBL 조명

---

### Phase 5.5: Physics Integration (포트폴리오 마일스톤)
**목표:** 플레이 가능한 프로토타입 제작

> **중요:** 이 시점부터 포트폴리오 영상 제작 가능!

- [ ] **Physics Engine 통합**
  - [ ] Jolt Physics 또는 Bullet 선택
  - [ ] Physics World 초기화
- [ ] **Physics Components (ECS)**
  - [ ] RigidBodyComponent (Mass, Velocity)
  - [ ] ColliderComponent (Box, Sphere, Capsule)
  - [ ] Physics Material (Friction, Restitution)
- [ ] **PhysicsSystem**
  - [ ] Transform ↔ Physics Engine 동기화
  - [ ] Fixed Timestep
  - [ ] 충돌 이벤트 (OnCollisionEnter/Exit)
- [ ] **샌드박스 데모**
  - [ ] 중력으로 떨어지는 큐브들
  - [ ] 플레이어가 오브젝트 밀기 (마우스 Ray)
  - [ ] ImGui로 RigidBody 속성 편집
- [ ] **물리 디버그 렌더링**
  - [ ] Collider 와이어프레임
  - [ ] Contact Point 표시

**완료 시:** PBR 재질 + Physics 상호작용 데모 (포트폴리오 소재 확보)

---

### Phase 6: Shadows & Ambient
**목표:** 시각적 완성도 향상

- [ ] **Shadow Mapping**
  - [ ] 기본 Shadow Map (Depth, PCF)
  - [ ] Cascaded Shadow Maps (CSM)
  - [ ] Cascade 경계 블렌딩
- [ ] **Ambient Occlusion**
  - [ ] SSAO (Screen Space Ambient Occlusion)
  - [ ] Bilateral Blur
- [ ] **추가 효과 (선택적)**
  - [ ] SSR (Screen Space Reflections)
  - [ ] DOF (Depth of Field)

**완료 시:** Physics 샌드박스 + 동적 그림자 + SSAO

---

### Phase 7: Compute Shaders & GPU 가속
**목표:** GPU 연산 활용

- [ ] **Compute Pipeline**
  - [ ] Dispatch 프레임워크
  - [ ] UAV (Unordered Access View) 관리
- [ ] **GPU Particle System**
  - [ ] Compute Shader 기반 시뮬레이션
  - [ ] Indirect Drawing
  - [ ] Physics 오브젝트와 상호작용
- [ ] **GPU Culling (선택적)**
  - [ ] Frustum Culling (Compute)
  - [ ] Hi-Z Occlusion Culling

**완료 시:** 10만 개 파티클 + Physics 상호작용

---

### Phase 8: Bindless & GPU Driven
**목표:** 최신 렌더링 기법

- [ ] **Bindless Resources**
  - [ ] Descriptor Indexing (SM 6.6+)
  - [ ] Unbounded Descriptor Arrays
  - [ ] Material ID → Descriptor Index 매핑
- [ ] **GPU Driven Rendering**
  - [ ] ExecuteIndirect / DrawIndirect
  - [ ] GPU가 Draw Call 생성
  - [ ] Mesh Shaders (선택적, DX12 Ultimate)
- [ ] **Resource Management**
  - [ ] GPU 메모리 할당자
  - [ ] Async Upload 튜닝

**완료 시:** 1만 개 고유 메쉬 + Physics 렌더링

---

### Phase 9: Job System 확장 (Phase 3.5를 건너뛴 경우)
**목표:** CPU 병렬화

- [ ] Job System 구현 (Phase 3.5 참고)
- [ ] ECS System 병렬화
- [ ] PhysicsSystem 병렬화
- [ ] 렌더 스레드 분리

---

### Phase 10: AI & Gameplay Systems
**목표:** 게임 로직 프레임워크

- [ ] **Navigation**
  - [ ] NavMesh 생성 (Recast)
  - [ ] A* Pathfinding
  - [ ] Path Following
- [ ] **Behavior Tree**
  - [ ] Executor (Selector, Sequence, Action)
  - [ ] Blackboard (공유 데이터)
- [ ] **Gameplay Framework**
  - [ ] Event System (타입 안전)
  - [ ] Game State 관리

**완료 시:** AI 에이전트가 플레이어 추적, Physics 오브젝트 회피

---

### Phase 11: Audio & Tools
**목표:** 완성도 향상

- [ ] **Audio System**
  - [ ] XAudio2 통합
  - [ ] 3D Spatialization
  - [ ] 충돌 시 사운드 효과
- [ ] **Debug Tools 고도화**
  - [ ] ECS Inspector (컴포넌트 추가/삭제)
  - [ ] Profiler (CPU/GPU Timeline)
  - [ ] Console (명령어 시스템)

**완료 시:** 완전한 게임 엔진 경험

---

### Phase 12: 데모 게임 & 폴리싱
**목표:** 포트폴리오 완성

- [ ] **Demo Game (Simple TPS)**
  - [ ] Physics Character Controller
  - [ ] 발사 시스템 (Raycast + Force)
  - [ ] Enemy AI (NavMesh + Physics)
  - [ ] UI (체력, 탄약)
- [ ] **Optimization**
  - [ ] CPU/GPU Profiling
  - [ ] Physics 성능 최적화
  - [ ] LOD (Level of Detail)
- [ ] **포트폴리오 문서화**
  - [ ] 기술 블로그 (ECS + Physics 통합)
  - [ ] 영상 녹화 (플레이 + 기술 설명)
  - [ ] GitHub README 업데이트

**최종 결과:** 플레이 가능한 TPS 데모, 포트폴리오 제출 가능

---

## 백로그 (Backlog / On-Demand)

향후 필요에 따라 선택적으로 추가할 기능들:

- [ ] **Deferred Rendering** - Forward+와 비교/전환
- [ ] **Tessellation & Geometry Shaders** - 적응형 지형
- [ ] **독립형 Editor** - ImGui 툴을 넘어선 독립 에디터
- [ ] **Scripting** - Lua/Python 바인딩
- [ ] **Procedural Terrain** - Heightmap/Voxel 기반
- [ ] **Raytracing (DXR)** - DX12 Ultimate 기반 (RTX GPU 필요)
- [ ] **Advanced Animation** - 블렌딩 트리, IK
- [ ] **Volumetric Effects** - Fog, Clouds
- [ ] **Network Replication** - ECS 컴포넌트 복제, 멀티플레이어

---

## 주요 특징

### 아키텍처
- **모듈식 설계**: 명확한 책임 분리와 의존성 관리
- **ECS 아키텍처**: 데이터 지향 설계 (예정)
- **Framework 패턴**: Application 라이프사이클 자동 관리
- **What/How 분리**: Scene(논리)와 Renderer(구현) 분리

### 렌더링
- **멀티 텍스처**: PBR 워크플로우 준비
- **효율적인 리소스 관리**: 중앙집중식 ResourceManager
- **프레임 버퍼링**: 트리플 버퍼링으로 CPU-GPU 병렬성

### 코드 품질
- **현대적 C++20**: 스마트 포인터, 람다, constexpr 활용
- **일관된 코딩 컨벤션**: 문서화된 스타일 가이드
- **포괄적인 로깅**: 카테고리별 디버깅 지원

## 문서

- [아키텍처 가이드](./Docs/Architecture.md) - 엔진 설계와 구조
- [코딩 컨벤션](./Docs/CodingConvention.md) - 코딩 규칙
- [개발 일지](./Docs/DevelopmentLog.md) - 개발 히스토리

## 라이선스

이 프로젝트는 [MIT License](./LICENSE) 하에 공개되어 있습니다.

## 개발자

**rniman**
- GitHub: [@rniman](https://github.com/rniman)
- Email: shinmg00@gmail.com

## 감사의 말

- [DirectX 12 공식 문서](https://docs.microsoft.com/en-us/windows/win32/direct3d12/)
- [Learn OpenGL](https://learnopengl.com/) - 그래픽스 개념 학습
- [Game Engine Architecture by Jason Gregory](https://www.gameenginebook.com/)
- [3D Game Programming with DirectX 12 by Frank Luna](http://www.d3dcoder.net/)

---

**최종 업데이트:** 2025-11-06  
**로드맵 버전:** v3.2 (포트폴리오 중심)

⭐ 이 프로젝트가 도움이 되었다면 Star를 눌러주세요!