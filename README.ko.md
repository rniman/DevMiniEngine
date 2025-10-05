# DevMiniEngine

**한국어** | **[English](./README.md)**

**DirectX 12 기반 학습용 미니 게임 엔진**

## 📋 프로젝트 개요

DevMiniEngine은 DirectX 12를 기반으로 하는 개인 학습 및 포트폴리오 목적의 게임 엔진 프로젝트입니다. ECS(Entity Component System) 아키텍처와 현대적인 C++ 디자인 패턴을 학습하고 실험하기 위해 제작되었습니다.

### 🎯 주요 목표

- **ECS 아키텍처 학습**: 데이터 지향 설계와 엔티티-컴포넌트 시스템 구현
- **최신 그래픽스 기술**: DirectX 12를 활용한 현대적 렌더링 파이프라인 구축
- **게임 엔진 기초**: 물리, AI, 지형 생성 등 핵심 엔진 시스템 구현
- **포트폴리오 제작**: 실무 수준의 코드 품질과 문서화

## ✨ 주요 기능

### 🎨 그래픽스
- DirectX 12 기반 렌더링 시스템
- RHI(Render Hardware Interface) 추상화 레이어
- PBR(Physically Based Rendering) 지원 준비
- 고수준 렌더링 시스템 (Material, Mesh, Camera)

### ⚙️ ECS 아키텍처
- Entity 기반 게임 오브젝트 관리
- Component 기반 데이터 구조
- System 기반 로직 처리
- 효율적인 메모리 레이아웃

### 🌍 지형 시스템
- 절차적 지형 생성 (Procedural Generation)
- Height Map 기반 렌더링
- LOD(Level of Detail) 지원 계획

### ⚛️ 물리 엔진
- 기본 충돌 감지 (Collision Detection)
- 강체 역학 (Rigid Body Dynamics)
- Physics World 시뮬레이션

### 🤖 게임 AI
- A* 길찾기 알고리즘
- NavMesh 기반 내비게이션
- Behavior Tree 지원 계획

## 🏗️ 프로젝트 구조

```
DevMiniEngine/
├── Engine/                          # 엔진 코어
│   ├── Core/                        # 기본 시스템 (메모리, 스레딩, 로깅)
│   ├── Math/                        # 수학 라이브러리
│   ├── ECS/                         # Entity Component System
│   ├── Graphics/                    # 렌더링 시스템
│   │   ├── RHI/                     # 렌더링 추상화 계층
│   │   ├── DX12/                    # DirectX 12 구현
│   │   └── Renderer/                # 고수준 렌더링
│   ├── Physics/                     # 물리 엔진
│   ├── Terrain/                     # 지형 시스템
│   ├── AI/                          # 게임 AI
│   ├── Scene/                       # 씬 관리
│   └── Input/                       # 입력 처리
│
├── Platform/                        # 플랫폼 계층 (Windows)
├── Samples/                         # 학습 샘플 프로젝트
├── Tools/                           # 개발 도구
├── Assets/                          # 리소스 (셰이더, 텍스처, 모델)
├── Tests/                           # 단위 테스트
├── Docs/                            # 문서
├── bin/                             # 빌드 출력물 (exe, lib, dll)
│   ├── Debug/
│   └── Release/
│
└── intermediate/                    # 중간 빌드 파일 (obj, pch)
   ├── Debug/
   └── Release/
```

아키텍처 설계 및 모듈 의존성에 대한 자세한 내용은 [아키텍처 문서](./Docs/Architecture.md)를 참조하세요.

## 🚀 빌드 및 실행 (예정)

### 요구 사항

- **OS**: Windows 10/11 (64-bit)
- **IDE**: Visual Studio 2022 이상
- **SDK**: Windows 10 SDK (10.0.19041.0 이상)
- **DirectX**: DirectX 12 지원 GPU

### 빌드 방법

1. **저장소 클론**
```bash
git clone https://github.com/rniman/DevMiniEngine.git
cd DevMiniEngine
```

2. **솔루션 열기**
```bash
# Visual Studio 2022로 열기
DevMiniEngine.sln
```

3. **빌드 구성 선택**
   - `Debug` - 디버깅용 (최적화 비활성화)
   - `Release` - 릴리즈용 (최적화 활성화)

4. **빌드 실행**
   - `Ctrl + Shift + B` 또는 메뉴에서 `빌드 > 솔루션 빌드`

### 샘플 실행

```bash
# 샘플 프로젝트를 시작 프로젝트로 설정 후 실행
bin/Debug/01_HelloTriangle.exe
```

## 📚 학습 샘플 (예정)

| 샘플 | 설명 | 학습 내용 |
|------|------|----------|
| **01_HelloTriangle** | 기본 삼각형 렌더링 | DirectX 12 기초, 렌더링 파이프라인 |
| **02_BasicLighting** | 조명 시스템 | Phong Shading, 노멀 매핑 |
| **03_TextureMapping** | 텍스처 적용 | 텍스처 샘플링, UV 매핑 |
| **04_ECS_Basics** | ECS 시스템 데모 | Entity, Component, System 개념 |
| **05_Physics_Demo** | 물리 시뮬레이션 | 충돌 감지, 강체 역학 |
| **06_Terrain_Demo** | 지형 생성 및 렌더링 | 절차적 생성, Height Map |
| **07_AI_Pathfinding** | AI 길찾기 | A* 알고리즘, NavMesh |

## 🛠️ 기술 스택

### 핵심 기술
- **C++20**: 최신 C++ 표준 활용
- **DirectX 12**: 저수준 그래픽스 API
- **Win32 API**: Windows 플랫폼 인터페이스

### 외부 라이브러리
- **DirectXTK12**: DirectX 유틸리티 라이브러리
- **ImGui**: 디버그 UI
- **spdlog**: 고성능 로깅 라이브러리

### 개발 도구
- **Visual Studio 2022**: 주 개발 환경
- **Git/GitHub**: 버전 관리
- **RenderDoc**: 그래픽스 디버깅

## 📖 문서

- [아키텍처 가이드](./Docs/Architecture.md) - 엔진 설계 철학과 구조
- [코딩 컨벤션](./Docs/CodingConvention.md) - 프로젝트 코딩 규칙
- [개발 일지](./Docs/DevelopmentLog.md) - 개발 진행 상황 기록
- [기술 노트](./Docs/TechnicalNotes/) - 구현 기술 상세 설명

## 🎓 학습 자료 (예정)

### ECS 아키텍처
- [ECS 개념 정리](./Docs/TechnicalNotes/ECS_Basics.md)
- [컴포넌트 설계 패턴](./Docs/TechnicalNotes/Component_Design.md)

### 그래픽스
- [DirectX 12 시작하기](./Docs/TechnicalNotes/DX12_Basics.md)
- [렌더링 파이프라인](./Docs/TechnicalNotes/Render_Pipeline.md)

### 물리 시뮬레이션
- [충돌 감지 알고리즘](./Docs/TechnicalNotes/Collision_Detection.md)
- [강체 역학 기초](./Docs/TechnicalNotes/Rigid_Body_Physics.md)

## 🔧 개발 스크립트 (예정)

```bash
# 프로젝트 클린
Scripts/clean.bat

# 전체 빌드
Scripts/build.bat

# 프로젝트 파일 재생성 (필요 시)
Scripts/generate_project.bat
```

## 🧪 테스트 (예정)

```bash
# 단위 테스트 실행
bin/Debug/CoreTests.exe
bin/Debug/ECSTests.exe
bin/Debug/MathTests.exe
```

## 📝 코딩 컨벤션 핵심 규칙

### 네이밍
- **클래스/함수**: `PascalCase`
- **지역 변수**: `camelCase`
- **멤버 변수**: `m` 접두어 + `PascalCase` (예: `mFrameIndex`)
- **전역 변수**: `g` 접두어 + `PascalCase`

### 스타일
- 포인터는 타입과 붙임: `ID3D12Device* device`
- `nullptr` 사용 (~~`NULL`~~ 금지)
- 중괄호 `{}` 항상 사용
- 초기화는 `=` 우선 사용

### 메모리 관리
- 스마트 포인터 우선: `std::unique_ptr`, `std::shared_ptr`
- DirectX COM 객체: `Microsoft::WRL::ComPtr`
- 읽기 전용 인자: `const T&`

자세한 내용은 [코딩 컨벤션 문서](./Docs/CodingConvention.md)를 참조하세요.

## 🗺️ 로드맵

### Phase 0: 프로젝트 설정 (현재)

### Phase 1: 기반 시스템 
- [ ] DirectX 12 초기화
- [ ] 기본 렌더링 파이프라인
- [ ] ECS 코어 시스템 구현
- [ ] 수학 라이브러리 완성

### Phase 2: 렌더링 확장
- [ ] PBR 머티리얼 시스템
- [ ] 그림자 매핑
- [ ] 포스트 프로세싱

### Phase 3: 게임플레이 시스템
- [ ] 물리 엔진 통합
- [ ] AI 시스템 구현
- [ ] 지형 생성 시스템

### Phase 4: 최적화 및 도구
- [ ] 멀티스레딩 렌더링
- [ ] 리소스 관리 최적화
- [ ] 에디터 프로토타입

## 🤝 기여

이 프로젝트는 개인 학습 목적이지만, 피드백과 제안은 언제나 환영합니다!

### 피드백 방법
- **Issues**: 버그 리포트나 기능 제안
- **Discussions**: 기술적 질문이나 아이디어 공유

## 📄 라이선스

이 프로젝트는 [MIT License](./LICENSE) 하에 공개되어 있습니다.

## 👤 개발자

**rniman**
- GitHub: [@rniman](https://github.com/rniman)
- Email: shinmg00@gmail.com
- Blog: [기술 블로그 URL](https://rniman.github.io/)

## 🙏 감사의 말

- [DirectX 12 공식 문서](https://docs.microsoft.com/en-us/windows/win32/direct3d12/)
- [Learn OpenGL](https://learnopengl.com/) - 그래픽스 개념 학습
- [Game Programming Patterns](https://gameprogrammingpatterns.com/) - 디자인 패턴 참고

---

⭐ 이 프로젝트가 도움이 되었다면 Star를 눌러주세요!