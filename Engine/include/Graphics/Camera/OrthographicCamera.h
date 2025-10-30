#pragma once

#include "Graphics/GraphicsTypes.h"
#include "Math/MathTypes.h"

#include "Graphics/Camera/Camera.h"

namespace Graphics
{
	/**
	 * @brief 직교 투영 카메라
	 *
	 * UI 렌더링 및 2D 게임에 사용되는 직교 투영 방식의 카메라입니다.
	 * 원근감 없이 물체의 크기가 거리에 관계없이 일정하게 표현됩니다.
	 */
	class OrthographicCamera : public Camera
	{
	public:
		/**
		 * @brief OrthographicCamera 생성자
		 *
		 * @param width 뷰포트 너비
		 * @param height 뷰포트 높이
		 * @param nearPlane 근평면 거리
		 * @param farPlane 원평면 거리
		 */
		OrthographicCamera(
			float32 width = 1280.0f,
			float32 height = 720.0f,
			float32 nearPlane = 0.1f,
			float32 farPlane = 1000.0f
		);
		~OrthographicCamera() override = default;

		OrthographicCamera(const OrthographicCamera&) = delete;
		OrthographicCamera& operator=(const OrthographicCamera&) = delete;

		// Camera override
		void UpdateProjectionMatrix() override;

		// 뷰포트 크기 설정
		void SetSize(float32 width, float32 height);
		void SetWidth(float32 width);
		void SetHeight(float32 height);

		// 클리핑 평면 설정
		void SetNearPlane(float32 nearPlane);
		void SetFarPlane(float32 farPlane);
		void SetClipPlanes(float32 nearPlane, float32 farPlane);

		// Getters
		float32 GetWidth() const { return mWidth; }
		float32 GetHeight() const { return mHeight; }
		float32 GetNearPlane() const { return mNearPlane; }
		float32 GetFarPlane() const { return mFarPlane; }

	private:
		float32 mWidth = 1280.0f;
		float32 mHeight = 720.0f;
		float32 mNearPlane = 0.1f;
		float32 mFarPlane = 1000.0f;
	};
} // namespace Graphics