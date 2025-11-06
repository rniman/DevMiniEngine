// Engine/include/Framework/Scene/GameObject.h
#pragma once

#include "Core/Types.h"
#include "Math/MathTypes.h"
#include <memory>
#include <string>

namespace Graphics
{
	class Mesh;
	class Material;
}

namespace Framework
{
	/**
	 * @brief 게임 오브젝트 (임시 구현, 나중에 ECS Entity로 대체)
	 */
	class GameObject
	{
	public:
		explicit GameObject(const std::string& name);
		~GameObject();

		// 이름
		const std::string& GetName() const { return mName; }
		void SetName(const std::string& name) { mName = name; }

		// Transform
		void SetPosition(const Math::Vector3& position);
		void SetRotation(const Math::Quaternion& rotation);
		void SetScale(const Math::Vector3& scale);

		Math::Vector3 GetPosition() const { return mPosition; }
		Math::Quaternion GetRotation() const { return mRotation; }
		Math::Vector3 GetScale() const { return mScale; }

		Math::Matrix4x4 GetWorldMatrix() const;

		// 렌더링 컴포넌트
		void SetMesh(std::shared_ptr<Graphics::Mesh> mesh) { mMesh = mesh; }
		void SetMaterial(std::shared_ptr<Graphics::Material> material) { mMaterial = material; }

		std::shared_ptr<Graphics::Mesh> GetMesh() const { return mMesh; }
		std::shared_ptr<Graphics::Material> GetMaterial() const { return mMaterial; }

		// 활성화 상태
		void SetActive(bool active) { mIsActive = active; }
		bool IsActive() const { return mIsActive; }

		// 업데이트
		virtual void Update(Core::float32 deltaTime);

	private:
		std::string mName;
		bool mIsActive = true;

		// Transform
		Math::Vector3 mPosition = { 0, 0, 0 };
		Math::Quaternion mRotation = { 0, 0, 0, 1 };
		Math::Vector3 mScale = { 1, 1, 1 };
		mutable bool mWorldMatrixDirty = true;
		mutable Math::Matrix4x4 mCachedWorldMatrix;

		// 렌더링 컴포넌트
		std::shared_ptr<Graphics::Mesh> mMesh;
		std::shared_ptr<Graphics::Material> mMaterial;
	};

} // namespace Framework
