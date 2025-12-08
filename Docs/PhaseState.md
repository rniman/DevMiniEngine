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

#### 3.4 Debug Tools (다음 단계)
- [ ] ImGui 통합
- [ ] ECS Inspector
- [ ] 성능 모니터링

#### 3.5 Advanced (선택적)
- [ ] Transform 계층 구조
- [ ] Dirty Flag 최적화
- [ ] Archetype Storage (Phase 4+)