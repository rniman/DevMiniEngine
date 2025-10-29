#include "pch.h"
#include "Graphics/Camera/PerspectiveCamera.h"
#include "Math/MathUtils.h"
#include "Core/Logging/LogMacros.h"
#include "Core/Assert.h"

using namespace Math;

namespace Graphics
{
    PerspectiveCamera::PerspectiveCamera(
        float fovY,
        float aspectRatio,
        float nearPlane,
        float farPlane
    )
        : mFovY(fovY)
        , mAspectRatio(aspectRatio)
        , mNearPlane(nearPlane)
        , mFarPlane(farPlane)
    {
        CORE_ASSERT(fovY > 0.0f && fovY < PI, "FOV must be between 0 and ¥ð radians");
        CORE_ASSERT(aspectRatio > 0.0f, "Aspect ratio must be positive");
        CORE_ASSERT(nearPlane > 0.0f && nearPlane < farPlane, "Invalid clip planes");

        UpdateProjectionMatrix();
    }

    void PerspectiveCamera::UpdateProjectionMatrix()
    {
        if (mProjectionDirty)
        {
            mProjectionMatrix = MatrixPerspectiveFovLH(
                mFovY,
                mAspectRatio,
                mNearPlane,
                mFarPlane
            );
            mProjectionDirty = false;
        }
    }

    void PerspectiveCamera::SetFovY(float fovY)
    {
        CORE_ASSERT(fovY > 0.0f && fovY < PI, "FOV must be between 0 and ¥ð radians");

        mFovY = fovY;
        mProjectionDirty = true;
    }

    void PerspectiveCamera::SetFovYDegrees(float degrees)
    {
        CORE_ASSERT(degrees > 0.0f && degrees < 180.0f, "FOV must be between 0 and 180 degrees");

        mFovY = DegToRad(degrees);
        mProjectionDirty = true;
    }

    void PerspectiveCamera::SetAspectRatio(float aspectRatio)
    {
        CORE_ASSERT(aspectRatio > 0.0f, "Aspect ratio must be positive");

        mAspectRatio = aspectRatio;
        mProjectionDirty = true;
    }

    void PerspectiveCamera::SetAspectRatio(float width, float height)
    {
        CORE_ASSERT(width > 0.0f && height > 0.0f, "Width and height must be positive");

        mAspectRatio = width / height;
        mProjectionDirty = true;
    }

    void PerspectiveCamera::SetNearPlane(float nearPlane)
    {
        if (nearPlane <= 0.0f || nearPlane >= mFarPlane)
        {
            LOG_WARN("[PerspectiveCamera] Invalid near plane (%f), must be > 0 and < far plane (%f)", nearPlane, mFarPlane);
        }

        mNearPlane = nearPlane;
        mProjectionDirty = true;
    }

    void PerspectiveCamera::SetFarPlane(float farPlane)
    {
        if (farPlane <= mNearPlane)
        {
            LOG_WARN("[PerspectiveCamera] Far plane (%f) <= Near plane (%f)", farPlane, mNearPlane);
        }

        mFarPlane = farPlane;
        mProjectionDirty = true;
    }

    void PerspectiveCamera::SetClipPlanes(float nearPlane, float farPlane)
    {
        CORE_ASSERT(nearPlane > 0.0f && nearPlane < farPlane, "Invalid clip planes");

        mNearPlane = nearPlane;
        mFarPlane = farPlane;
        mProjectionDirty = true;
    }

    float PerspectiveCamera::GetFovYDegrees() const
    {
        return RadToDeg(mFovY);
    }

} // namespace Graphics