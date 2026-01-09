/**
 * @file CameraSystem.h
 * @brief 카메라 행렬 업데이트 및 관리 System
 *
 * 생성자 주입 방식으로 Registry를 받습니다.
 * 고수준(Entity 기반)과 저수준(Component 직접) API를 모두 제공합니다.
 *
 * 카메라는 씬에 1-2개이므로 매 프레임 행렬을 재계산합니다.
 * (Dirty Flag 불필요)
 */
#pragma once
#include "ECS/ISystem.h"
#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Entity.h"
#include "Core/Types.h"
#include "Math/MathTypes.h"

namespace ECS
{
	// 전방 선언
	class Registry;

	/**
	 * @brief 카메라 System
	 *
	 * 매 프레임 카메라 행렬을 업데이트합니다.
	 * 고수준(Entity)과 저수준(Component) API를 모두 제공합니다.
	 */
	class CameraSystem : public ISystem
	{
	public:
		/**
		 * @brief 생성자
		 * @param registry 이 System이 속한 Registry
		 */
		explicit CameraSystem(Registry& registry);
		~CameraSystem() override = default;

		//=========================================================================
		// ISystem 인터페이스 구현
		//=========================================================================

		void Initialize() override;
		void Update(Core::float32 deltaTime) override;
		void Shutdown() override;

		//=========================================================================
		// 고수준 API (Entity 기반)
		//=========================================================================

		Entity FindMainCamera();
		bool SetMainCamera(Entity entity);

		bool SetFovYDegrees(Entity entity, Core::float32 degrees);
		bool SetFovYRadians(Entity entity, Core::float32 radians);
		bool SetAspectRatio(Entity entity, Core::float32 aspectRatio);
		bool SetAspectRatio(Entity entity, Core::float32 width, Core::float32 height);
		bool SetClipPlanes(Entity entity, Core::float32 nearPlane, Core::float32 farPlane);

		bool SetUpMode(Entity entity, CameraUpMode mode);
		bool SetWorldUpReference(Entity entity, const Math::Vector3& worldUp);
		bool SetLocalUp(Entity entity, const Math::Vector3& localUp);

		/**
		 * @brief LookAt 방식으로 카메라 설정
		 *
		 * TransformComponent의 position을 설정하고 카메라 방향 벡터를 갱신합니다.
		 *
		 * @param entity 대상 Entity
		 * @param position 카메라 위치
		 * @param target 바라볼 지점
		 * @param worldUp 월드 up 벡터 (WorldUp 모드에서 worldUpReference로도 설정됨)
		 * @return 성공 여부
		 */
		bool SetLookAt(
			Entity entity,
			const Math::Vector3& position,
			const Math::Vector3& target,
			const Math::Vector3& worldUp = Math::Vector3::Up()
		);

		//=========================================================================
		// 저수준 API (public static)
		// - 어디서든 호출 가능
		// - 다른 System에서 카메라 정보 조회/업데이트 시 사용
		//=========================================================================

		static Entity FindMainCamera(Registry& registry);
		static void UpdateViewMatrix(const TransformComponent& transform, CameraComponent& camera);
		static void UpdateProjectionMatrix(CameraComponent& camera);
		static void UpdateAllCameras(Registry& registry);

	private:
		//=========================================================================
		// 내부 헬퍼 함수 (private static)
		// - 값 검증 포함
		// - 고수준 API에서 내부적으로 사용
		//=========================================================================

		static void SetFovYDegreesInternal(CameraComponent& camera, Core::float32 degrees);
		static void SetFovYRadiansInternal(CameraComponent& camera, Core::float32 radians);
		static void SetAspectRatioInternal(CameraComponent& camera, Core::float32 aspectRatio);
		static void SetClipPlanesInternal(CameraComponent& camera, Core::float32 nearPlane, Core::float32 farPlane);
		static void SetUpModeInternal(CameraComponent& camera, CameraUpMode mode);
		static void SetWorldUpReferenceInternal(CameraComponent& camera, const Math::Vector3& worldUp);
		static void SetLocalUpInternal(CameraComponent& camera, const Math::Vector3& localUp);

		static void SetLookAtInternal(
			TransformComponent& transform,
			CameraComponent& camera,
			const Math::Vector3& position,
			const Math::Vector3& target,
			const Math::Vector3& worldUp
		);
	};

} // namespace ECS
