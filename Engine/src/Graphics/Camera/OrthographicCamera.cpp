#include "pch.h"
#include "Graphics/Camera/OrthographicCamera.h"
#include "Math/MathUtils.h"
#include "Core/Logging/LogMacros.h"
#include "Core/Assert.h"

using namespace Math;

namespace Graphics
{
	OrthographicCamera::OrthographicCamera(
		float width,
		float height,
		float nearPlane,
		float farPlane
	)
		: mWidth(width)
		, mHeight(height)
		, mNearPlane(nearPlane)
		, mFarPlane(farPlane)
	{
		CORE_ASSERT(width > 0.0f && height > 0.0f, "Width and height must be positive");
		CORE_ASSERT(nearPlane < farPlane, "Near plane must be less than far plane");

		UpdateProjectionMatrix();
	}

	void OrthographicCamera::UpdateProjectionMatrix()
	{
		if (mProjectionDirty)
		{
			mProjectionMatrix = MatrixOrthographicLH(
				mWidth,
				mHeight,
				mNearPlane,
				mFarPlane
			);
			mProjectionDirty = false;
		}
	}

	void OrthographicCamera::SetSize(float width, float height)
	{
		CORE_ASSERT(width > 0.0f && height > 0.0f, "Width and height must be positive");

		mWidth = width;
		mHeight = height;
		mProjectionDirty = true;
	}

	void OrthographicCamera::SetWidth(float width)
	{
		CORE_ASSERT(width > 0.0f, "Width must be positive");

		mWidth = width;
		mProjectionDirty = true;
	}

	void OrthographicCamera::SetHeight(float height)
	{
		CORE_ASSERT(height > 0.0f, "Height must be positive");

		mHeight = height;
		mProjectionDirty = true;
	}

	void OrthographicCamera::SetNearPlane(float nearPlane)
	{
		if (nearPlane >= mFarPlane)
		{
			LOG_WARN("[OrthographicCamera] Near plane (%f) >= Far plane (%f)", nearPlane, mFarPlane);
		}

		mNearPlane = nearPlane;
		mProjectionDirty = true;
	}

	void OrthographicCamera::SetFarPlane(float farPlane)
	{
		if (farPlane <= mNearPlane)
		{
			LOG_WARN("[OrthographicCamera] Far plane (%f) <= Near plane (%f)", farPlane, mNearPlane);
		}

		mFarPlane = farPlane;
		mProjectionDirty = true;
	}

	void OrthographicCamera::SetClipPlanes(float nearPlane, float farPlane)
	{
		CORE_ASSERT(nearPlane < farPlane, "Near plane must be less than far plane");

		mNearPlane = nearPlane;
		mFarPlane = farPlane;
		mProjectionDirty = true;
	}

} // namespace Graphics