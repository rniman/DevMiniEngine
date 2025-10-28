#pragma once
#include "Graphics/Camera/Camera.h"

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
		static constexpr float DEFAULT_FOV_RADIANS = 1.047f;        // 60도
		static constexpr float DEFAULT_ASPECT_RATIO = 16.0f / 9.0f;
		static constexpr float DEFAULT_NEAR_PLANE = 0.1f;
		static constexpr float DEFAULT_FAR_PLANE = 1000.0f;

		/**
		 * @brief PerspectiveCamera 생성자
		 *
		 * @param fovY 수직 시야각 (라디안, 기본값 1.047 ≈ 60도)
		 * @param aspectRatio 종횡비 (width / height, 기본값 16:9)
		 * @param nearPlane 근평면 거리 (기본값 0.1)
		 * @param farPlane 원평면 거리 (기본값 1000.0)
		 */
		PerspectiveCamera(
			float fovY = DEFAULT_FOV_RADIANS,
			float aspectRatio = DEFAULT_ASPECT_RATIO,
			float nearPlane = DEFAULT_NEAR_PLANE,
			float farPlane = DEFAULT_FAR_PLANE
		);
		~PerspectiveCamera() override = default;

		PerspectiveCamera(const PerspectiveCamera&) = delete;
		PerspectiveCamera& operator=(const PerspectiveCamera&) = delete;

		// Camera override
		void UpdateProjectionMatrix() override;

		// FOV 설정 (라디안 또는 도 단위)
		void SetFovY(float fovY);
		void SetFovYDegrees(float degrees);

		// 종횡비 설정
		void SetAspectRatio(float aspectRatio);
		void SetAspectRatio(float width, float height);

		// 클리핑 평면 설정
		void SetNearPlane(float nearPlane);
		void SetFarPlane(float farPlane);
		void SetClipPlanes(float nearPlane, float farPlane);

		// Getters
		float GetFovY() const { return mFovY; }
		float GetFovYDegrees() const;
		float GetAspectRatio() const { return mAspectRatio; }
		float GetNearPlane() const { return mNearPlane; }
		float GetFarPlane() const { return mFarPlane; }

	private:
		float mFovY = DEFAULT_FOV_RADIANS;           
		float mAspectRatio = DEFAULT_ASPECT_RATIO;   
		float mNearPlane = DEFAULT_NEAR_PLANE;       
		float mFarPlane = DEFAULT_FAR_PLANE;         
	};
} // namespace Graphics