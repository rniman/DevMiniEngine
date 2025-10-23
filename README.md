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
  - 테스트 커버리지: 01_MemoryTest, 02_PoolAllocatorTest, 03_StackAllocatorTest

- 로깅 시스템
  - 다중 출력 Sink 아키텍처 (Console, File)
  - Mutex를 통한 스레드 안전 로깅
  - 카테고리 기반 필터링 (Core, Graphics, Physics, AI, Audio, Input, Memory)
  - 컬러 코딩된 콘솔 출력
  - 테스트 커버리지: 05_LoggingTest

**Math 라이브러리**
- SIMD 최적화 벡터/행렬 연산 (DirectXMath 래퍼)
- Vector2, Vector3, Vector4
- Matrix3x3, Matrix4x4 (행 우선)
- Quaternion 연산
- Transform 유틸리티 (Translation, Rotation, Scaling)
- 카메라 행렬 (LookAt, Perspective, Orthographic)
- 테스트 커버리지: 04_MathTest

**Platform 레이어**
- Win32 윈도우 생성 및 관리
- 이벤트 처리 시스템
- 입력 시스템
  - 키보드 상태 추적 (pressed/held/released)
  - 마우스 상태 추적 (버튼, 위치, 휠)
  - 프레임 정확도를 위한 더블 버퍼링
- 테스트 커버리지: 06_WindowTest, 07_InputTest

**Phase 2: DirectX 12 그래픽스 (25% 완료)**

**DirectX 12 초기화 (완료)**
- DX12Device: Device 및 Factory 생성, GPU 어댑터 선택
- DX12CommandQueue: Command Queue 생성 및 GPU 동기화
- DX12SwapChain: 더블 버퍼링 SwapChain 생성
- DX12DescriptorHeap: RTV Descriptor Heap 관리
- DX12CommandContext: Command Allocator/List 관리 및 Resource Barrier
- 첫 렌더링 성공 (Clear screen)
- 테스트 커버리지: 08_DX12Init

### 진행 중

**Phase 2: DirectX 12 그래픽스**
- 다음 우선순위: 삼각형 렌더링 (Vertex Buffer, Shader 컴파일, PSO)

### 프로젝트 통계

- 총 코드 라인 수: ~3,000
- 구현된 모듈: 5/12 서브시스템
- 테스트 커버리지: 8개 샘플 프로젝트
- 컴파일러 경고: 0개 (Level 4)

## 주요 기능

### 그래픽스 (부분 구현)
- DirectX 12 기반 렌더링 시스템 (초기화 완료)
  - Device 및 Command Queue 생성
  - SwapChain 및 Descriptor Heap 관리
  - Command List 기록 및 실행
  - Resource Barrier를 통한 리소스 상태 전환
- RHI(Render Hardware Interface) 추상화 레이어 (계획됨)
- PBR(Physically Based Rendering) 지원 (계획됨)
- 고수준 렌더링 시스템 (Material, Mesh, Camera) (계획됨)

### ECS 아키텍처 (계획됨)
- Entity 기반 게임 오브젝트 관리
- Component 기반 데이터 구조
- System 기반 로직 처리
- 효율적인 메모리 레이아웃

### 물리 엔진 (계획됨)
- 기본 충돌 감지
- 강체 역학
- Physics World 시뮬레이션

### 게임 AI (계획됨)
- A* 길찾기 알고리즘
- NavMesh 기반 내비게이션

## 프로젝트 구조
```
DevMiniEngine/
├── Engine/                          # 엔진 코어
│   ├── include/                     # 모든 모듈의 public 헤더
│   │   ├── Core/                    # Core 헤더
│   │   │   ├── Memory/              # 메모리 관리 헤더
│   │   │   └── Logging/             # 로깅 시스템 헤더
│   │   ├── Math/                    # Math 라이브러리 헤더
│   │   ├── Platform/                # Platform 레이어 헤더
│   │   │   ├── Windows/             # Win32 헤더
│   │   │   └── Input/               # 입력 시스템 헤더
│   │   ├── Graphics/                # 그래픽스 헤더
│   │   │   ├── DX12/                # DirectX 12 헤더
│   │   │   ├── RHI/                 # RHI 헤더 (계획됨)
│   │   │   └── Renderer/            # 고수준 렌더링 헤더 (계획됨)
│   │   ├── ECS/                     # ECS 헤더 (계획됨)
│   │   ├── Physics/                 # 물리 엔진 헤더 (계획됨)
│   │   ├── AI/                      # 게임 AI 헤더 (계획됨)
│   │   └── Scene/                   # 씬 관리 헤더 (계획됨)
│   │
│   ├── src/                         # 모든 모듈의 구현
│   │   ├── Core/                    # Core 구현
│   │   │   ├── Memory/              # 메모리 관리 구현
│   │   │   └── Logging/             # 로깅 시스템 구현
│   │   ├── Math/                    # Math 라이브러리 구현
│   │   ├── Platform/                # Platform 레이어 구현
│   │   │   ├── Windows/             # Win32 구현
│   │   │   └── Input/               # 입력 시스템 구현
│   │   ├── Graphics/                # 그래픽스 구현
│   │   │   ├── DX12/                # DirectX 12 구현
│   │   │   ├── RHI/                 # RHI 구현 (계획됨)
│   │   │   └── Renderer/            # 고수준 렌더링 구현 (계획됨)
│   │   ├── ECS/                     # ECS 구현 (계획됨)
│   │   ├── Physics/                 # 물리 엔진 구현 (계획됨)
│   │   ├── AI/                      # 게임 AI 구현 (계획됨)
│   │   └── Scene/                   # 씬 관리 구현 (계획됨)
│   │
│   ├── Core/                        # Core 모듈 프로젝트
│   │   └── Core.vcxproj
│   ├── Math/                        # Math 모듈 프로젝트
│   │   └── Math.vcxproj
│   ├── Platform/                    # Platform 모듈 프로젝트
│   │   └── Platform.vcxproj
│   ├── Graphics/                    # Graphics 모듈 프로젝트
│   │   └── Graphics.vcxproj
│   │
│   ├── Sandbox/                     # 샌드박스 게임 프로젝트
│   │   ├── Assets/                  # 게임 에셋
│   │   ├── Source/                  # 게임 소스
│   │   └── Sandbox.vcxproj
│   │
│   └── DemoGame/                    # 데모 게임 프로젝트 (계획됨)
│       ├── Assets/
│       ├── Source/
│       └── DemoGame.vcxproj
│
├── Samples/                         # 학습 샘플 프로젝트
│   ├── 01_MemoryTest/               # LinearAllocator 테스트
│   ├── 02_PoolAllocatorTest/        # PoolAllocator 테스트
│   ├── 03_StackAllocatorTest/       # StackAllocator 테스트
│   ├── 04_MathTest/                 # Math 라이브러리 테스트
│   ├── 05_LoggingTest/              # 로깅 시스템 테스트
│   ├── 06_WindowTest/               # 윈도우 생성 테스트
│   ├── 07_InputTest/                # 입력 시스템 테스트
│   └── 08_DX12Init/                 # DirectX 12 초기화 테스트
│
├── Docs/                            # 문서
│   ├── Sample
│   │   └── 08_DX12Init.md           # 08_DX12Init 샘플 문서
│   │
│   ├── Architecture.md              # 상세 아키텍처 가이드
│   ├── CodingConvention.md          # 코딩 표준
│   └── DevelopmentLog.md            # 개발 진행 상황
│ 
├── bin/                             # 빌드 출력물
│   ├── Debug
│   └── Release
│ 
└── intermediate/                   # 중간 빌드 파일
```

아키텍처 설계 및 모듈 의존성에 대한 자세한 내용은 [아키텍처 문서](./Docs/Architecture.md)를 참조하세요.

## 빌드 및 실행

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
# 예시: 메모리 할당자 테스트 실행
bin/Debug/01_MemoryTest.exe

# 예시: DirectX 12 초기화 테스트 실행
bin/Debug/08_DX12Init.exe
```

## 샘플 프로젝트

| 샘플 | 상태 | 설명 | 학습 내용 |
|------|------|------|----------|
| 01_MemoryTest | 완료 | LinearAllocator 테스트 | 커스텀 메모리 할당 |
| 02_PoolAllocatorTest | 완료 | PoolAllocator 테스트 | 객체 풀링 |
| 03_StackAllocatorTest | 완료 | StackAllocator 테스트 | 스코프 할당 |
| 04_MathTest | 완료 | Math 라이브러리 테스트 | SIMD 벡터, 행렬 |
| 05_LoggingTest | 완료 | 로깅 시스템 테스트 | 구조화된 로깅 |
| 06_WindowTest | 완료 | 윈도우 생성 | Win32 API, 이벤트 |
| 07_InputTest | 완료 | 입력 처리 | 키보드, 마우스 추적 |
| 08_DX12Init | 완료 | DirectX 12 초기화 | Device, SwapChain, Clear |
| 09_HelloTriangle | 계획됨 | 기본 렌더링 | Vertex Buffer, Shader, PSO |

## 기술 스택

### 핵심 기술
- **C++20**: 최신 C++ 표준
- **DirectX 12**: 저수준 그래픽스 API
- **Win32 API**: Windows 플랫폼 인터페이스
- **DirectXMath**: SIMD 최적화 수학 라이브러리

### 개발 도구
- **Visual Studio 2022**: 주 개발 환경
- **Git/GitHub**: 버전 관리
- **RenderDoc**: 그래픽스 디버깅 (계획됨)
- **PIX**: DirectX 12 프로파일링 (계획됨)

## 문서

- [아키텍처 가이드](./Docs/Architecture.md) - 완전한 엔진 설계와 구조
- [코딩 컨벤션](./Docs/CodingConvention.md) - 프로젝트 코딩 규칙
- [개발 일지](./Docs/DevelopmentLog.md) - 상세한 개발 히스토리

## 로드맵

### Phase 1: 기반 시스템 (100% 완료)
- [x] 프로젝트 구조
- [x] Core 시스템
  - [x] 메모리 할당자 (Linear, Pool, Stack)
  - [x] 로깅 시스템 (Console, File sinks)
  - [x] 어설션 매크로
- [x] Math 라이브러리 (SIMD와 함께 Vector, Matrix, Quaternion)
- [x] Platform 레이어 (Window, Input)

### Phase 2: 그래픽스 (25% 완료)
- [x] DirectX 12 초기화
- [ ] 기본 렌더링 파이프라인 (Vertex Buffer, Shader, PSO)
- [ ] 메시 및 텍스처 로딩
- [ ] 카메라 시스템

### Phase 3: 고급 ECS (시작 안 됨)
- [ ] ECS 코어 구현
- [ ] 컴포넌트 쿼리
- [ ] 시스템 의존성
- [ ] 이벤트 시스템

### Phase 4: 게임플레이 시스템 (시작 안 됨)
- [ ] 물리 엔진 통합
- [ ] 충돌 감지
- [ ] AI 길찾기 (A*)
- [ ] 지형 생성

### Phase 5: 향후 계획
- [ ] 스레딩 시스템 (Job system, thread pool)
- [ ] PBR 머티리얼
- [ ] 그림자 매핑
- [ ] 포스트 프로세싱

## 코드 품질

- 경고 레벨: 4 (최고)
- 활성 경고: 0개
- C++ 표준: C++20
- 코드 스타일: CodingConvention.md를 통해 강제

## 기여

이 프로젝트는 개인 학습 목적이지만, 피드백과 제안은 언제나 환영합니다!

### 피드백 방법
- **Issues**: 버그 리포트나 기능 제안
- **Discussions**: 기술적 질문이나 아이디어 공유

## 라이선스

이 프로젝트는 [MIT License](./LICENSE) 하에 공개되어 있습니다.

## 개발자

**rniman**
- GitHub: [@rniman](https://github.com/rniman)
- Email: shinmg00@gmail.com
- Blog: [기술 블로그](https://rniman.github.io/)

## 감사의 말

- [DirectX 12 공식 문서](https://docs.microsoft.com/en-us/windows/win32/direct3d12/)
- [Learn OpenGL](https://learnopengl.com/) - 그래픽스 개념 학습
- [Game Programming Patterns](https://gameprogrammingpatterns.com/) - 디자인 패턴 참고
- [Game Engine Architecture by Jason Gregory](https://www.gameenginebook.com/)
- [3D Game Programming with DirectX 12 by Frank Luna](http://www.d3dcoder.net/)

---

최종 업데이트: 2025-10-23

⭐ 이 프로젝트가 도움이 되었다면 Star를 눌러주세요!