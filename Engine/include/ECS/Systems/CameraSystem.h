/**
 * @file CameraSystem.h
 * @brief 카메라 행렬 업데이트 및 관리 System
 *
 * 생성자 주입 방식으로 Registry를 받습니다.
 * 고수준(Entity 기반)과 저수준(Component 직접) API를 모두 제공합니다.
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

		bool SetLookAt(
			Entity entity,
			const Math::Vector3& position,
			const Math::Vector3& target,
			const Math::Vector3& up = Math::Vector3::Up()
		);

		//=========================================================================
		// 저수준 API (정적) - System 내부, 다른 System에서 호출
		//=========================================================================

		static Entity FindMainCamera(Registry& registry);
		static void UpdateViewMatrix(const TransformComponent& transform, CameraComponent& camera);
		static void UpdateProjectionMatrix(CameraComponent& camera);
		static void UpdateAllCameras(Registry& registry);

		static void SetFovYDegrees(CameraComponent& camera, Core::float32 degrees);
		static void SetFovYRadians(CameraComponent& camera, Core::float32 radians);
		static void SetAspectRatio(CameraComponent& camera, Core::float32 aspectRatio);
		static void SetAspectRatio(CameraComponent& camera, Core::float32 width, Core::float32 height);
		static void SetClipPlanes(CameraComponent& camera, Core::float32 nearPlane, Core::float32 farPlane);

		static void SetLookAt(
			TransformComponent& transform,
			CameraComponent& camera,
			const Math::Vector3& position,
			const Math::Vector3& target,
			const Math::Vector3& up = Math::Vector3::Up()
		);
	};

} // namespace ECS
