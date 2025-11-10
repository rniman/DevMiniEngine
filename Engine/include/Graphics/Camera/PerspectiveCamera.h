#pragma once
#include "Graphics/Camera/Camera.h"
#include "Graphics/GraphicsTypes.h"

namespace Graphics
{
	/**
	 * @brief 원근 투영 카메라
	 *
	 * 3D 렌더링에 사용되는 원근 투영 방식의 카메라입니다.
	 * FOV, Aspect Ratio, Near/Far Plane을 설정할 수 있습니다.
	 */
	class PerspectiveCamera : public Camera
	{
	public:
		// 기본값 상수
		static constexpr float32 DEFAULT_FOV_RADIANS = 1.047f;        // 60도
		static constexpr float32 DEFAULT_ASPECT_RATIO = 16.0f / 9.0f;
		static constexpr float32 DEFAULT_NEAR_PLANE = 0.1f;
		static constexpr float32 DEFAULT_FAR_PLANE = 1000.0f;

		/**
		 * @brief PerspectiveCamera 생성자
		 *
		 * @param fovY 수직 시야각 (라디안, 기본값 1.047 ≈ 60도)
		 * @param aspectRatio 종횡비 (width / height, 기본값 16:9)
		 * @param nearPlane 근평면 거리 (기본값 0.1)
		 * @param farPlane 원평면 거리 (기본값 1000.0)
		 */
		PerspectiveCamera(
			float32 fovY = DEFAULT_FOV_RADIANS,
			float32 aspectRatio = DEFAULT_ASPECT_RATIO,
			float32 nearPlane = DEFAULT_NEAR_PLANE,
			float32 farPlane = DEFAULT_FAR_PLANE
		);
		~PerspectiveCamera() override = default;

		PerspectiveCamera(const PerspectiveCamera&) = delete;
		PerspectiveCamera& operator=(const PerspectiveCamera&) = delete;

		// Camera override
		void UpdateProjectionMatrix() override;

		// FOV 설정 (라디안 또는 도 단위)
		void SetFovY(float32 fovY);
		void SetFovYDegrees(float32 degrees);

		// 종횡비 설정
		void SetAspectRatio(float32 aspectRatio);
		void SetAspectRatio(float32 width, float32 height);

		// 클리핑 평면 설정
		void SetNearPlane(float32 nearPlane);
		void SetFarPlane(float32 farPlane);
		void SetClipPlanes(float32 nearPlane, float32 farPlane);

		// Getters
		float32 GetFovY() const { return mFovY; }
		float32 GetFovYDegrees() const;
		float32 GetAspectRatio() const { return mAspectRatio; }
		float32 GetNearPlane() const { return mNearPlane; }
		float32 GetFarPlane() const { return mFarPlane; }

	private:
		float32 mFovY = DEFAULT_FOV_RADIANS;
		float32 mAspectRatio = DEFAULT_ASPECT_RATIO;
		float32 mNearPlane = DEFAULT_NEAR_PLANE;
		float32 mFarPlane = DEFAULT_FAR_PLANE;
	};
} // namespace Graphics
