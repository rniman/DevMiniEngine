#include "pch.h"
#include "Framework/Scene/GameObject.h"
#include "Math/MathUtils.h"

namespace Framework
{
	GameObject::GameObject(const std::string& name)
		: mName(name)
	{
		mCachedWorldMatrix = Math::MatrixIdentity();
		LOG_TRACE("GameObject '%s' created", mName.c_str());
	}

	GameObject::~GameObject()
	{
		LOG_TRACE("GameObject '%s' destroyed", mName.c_str());
	}

	void GameObject::SetPosition(const Math::Vector3& position)
	{
		mPosition = position;
		mWorldMatrixDirty = true;
	}

	void GameObject::SetRotation(const Math::Quaternion& rotation)
	{
		mRotation = rotation;
		mWorldMatrixDirty = true;
	}

	void GameObject::SetScale(const Math::Vector3& scale)
	{
		mScale = scale;
		mWorldMatrixDirty = true;
	}

	Math::Matrix4x4 GameObject::GetWorldMatrix() const
	{
		if (mWorldMatrixDirty)
		{
			// Scale -> Rotation -> Translation 순서로 변환 행렬 생성
			Math::Matrix4x4 scaleMatrix = Math::MatrixScaling(mScale.x, mScale.y, mScale.z);
			Math::Matrix4x4 rotationMatrix = Math::MatrixRotationQuaternion(mRotation);
			Math::Matrix4x4 translationMatrix = Math::MatrixTranslation(mPosition.x, mPosition.y, mPosition.z);

			// 행렬 결합: Scale * Rotation * Translation
			mCachedWorldMatrix = Math::MatrixMultiply(scaleMatrix, rotationMatrix);
			mCachedWorldMatrix = Math::MatrixMultiply(mCachedWorldMatrix, translationMatrix);

			mWorldMatrixDirty = false;
		}
		return mCachedWorldMatrix;
	}

	void GameObject::Update(Core::float32 deltaTime)
	{
		// 기본 구현은 비어있음
		(void)deltaTime;
	}

} // namespace Framework
