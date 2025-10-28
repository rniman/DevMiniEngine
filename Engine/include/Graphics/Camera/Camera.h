#pragma once
#include "Math/MathTypes.h"

namespace Graphics
{
	/**
	 * @brief 카메라 기본 클래스
	 *
	 * View와 Projection 행렬을 관리하는 추상 기본 클래스입니다.
	 * PerspectiveCamera와 OrthographicCamera의 공통 인터페이스를 제공합니다.
	 */
	class Camera
	{
	public:
		Camera();
		virtual ~Camera() = default;

		Camera(const Camera&) = delete;
		Camera& operator=(const Camera&) = delete;

		void MoveForward(float distance);
		void MoveRight(float distance);
		void MoveUp(float distance);
		void RotateYaw(float angle);
		void RotatePitch(float angle);

		/**
		 * @brief View 행렬을 업데이트 (Dirty Flag 패턴)
		 */
		void UpdateViewMatrix();

		/**
		 * @brief Projection 행렬을 업데이트 (파생 클래스에서 구현)
		 */
		virtual void UpdateProjectionMatrix() = 0;

		void SetPosition(const Math::Vector3& position);
		void SetTarget(const Math::Vector3& target);
		void SetUpVector(const Math::Vector3& up);

		/**
		 * @brief 카메라의 위치, 타겟, Up 벡터를 한번에 설정
		 * @param position 새로운 위치
		 * @param target 새로운 타겟 위치
		 * @param up 새로운 Up 벡터
		 */
		void SetLookAt(const Math::Vector3& position, const Math::Vector3& target, const Math::Vector3& up);

		/**
		 * @brief 카메라의 위치, 방향, Up 벡터를 한번에 설정
		 * @param position 새로운 위치
		 * @param direction 카메라가 바라보는 방향 벡터
		 * @param up 새로운 Up 벡터
		 */
		void SetLookTo(const Math::Vector3& position, const Math::Vector3& direction, const Math::Vector3& up);
		
		const Math::Vector3& GetPosition() const { return mPosition; }
		const Math::Vector3& GetTarget() const { return mTarget; }
		const Math::Vector3& GetUpVector() const { return mUpVector; }
		Math::Vector3 GetForwardVector() const;
		Math::Vector3 GetRightVector() const;
		const Math::Matrix4x4& GetViewMatrix() const { return mViewMatrix; }
		const Math::Matrix4x4& GetProjectionMatrix() const { return mProjectionMatrix; }

		/**
		 * @brief View와 Projection을 곱한 행렬 반환
		 * @return View * Projection 행렬
		 */
		Math::Matrix4x4 GetViewProjectionMatrix() const;

	protected:
		Math::Vector3 mPosition = Math::Vector3(0.0f, 0.0f, 0.0f);
		Math::Vector3 mTarget = Math::Vector3(0.0f, 0.0f, 1.0f);
		Math::Vector3 mUpVector = Math::Vector3(0.0f, 1.0f, 0.0f);

		Math::Matrix4x4 mViewMatrix;       
		Math::Matrix4x4 mProjectionMatrix; 

		bool mViewDirty = true;           // View 행렬 갱신 필요 여부
		bool mProjectionDirty = true;     // Projection 행렬 갱신 필요 여부

	private:
		void MoveAlongDirection(const Math::Vector3& direction, float distance);

	};

} // namespace Graphics