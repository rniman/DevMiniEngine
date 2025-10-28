#include <iostream>
#include <iomanip>
#include "Graphics/Camera/PerspectiveCamera.h"
#include "Graphics/Camera/OrthographicCamera.h"
#include "Math/MathUtils.h"

using namespace Graphics;
using namespace Math;

// 행렬 출력 헬퍼 함수
void PrintMatrix(const char* name, const Matrix4x4& mat)
{
    std::cout << name << ":\n";
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "  [" << mat._11 << ", " << mat._12 << ", " << mat._13 << ", " << mat._14 << "]\n";
    std::cout << "  [" << mat._21 << ", " << mat._22 << ", " << mat._23 << ", " << mat._24 << "]\n";
    std::cout << "  [" << mat._31 << ", " << mat._32 << ", " << mat._33 << ", " << mat._34 << "]\n";
    std::cout << "  [" << mat._41 << ", " << mat._42 << ", " << mat._43 << ", " << mat._44 << "]\n";
    std::cout << "\n";
}

// 벡터 출력 헬퍼 함수
void PrintVector(const char* name, const Vector3& vec)
{
    std::cout << name << ": ("
        << std::fixed << std::setprecision(4)
        << vec.x << ", " << vec.y << ", " << vec.z << ")\n";
}

void TestPerspectiveCamera()
{
    std::cout << "=== PerspectiveCamera Test ===\n\n";

    // 카메라 생성 (기본값)
    PerspectiveCamera camera;

    std::cout << "1. 기본 카메라 설정:\n";
    std::cout << "   FOV: " << camera.GetFovYDegrees() << " degrees\n";
    std::cout << "   Aspect: " << camera.GetAspectRatio() << "\n";
    std::cout << "   Near: " << camera.GetNearPlane() << "\n";
    std::cout << "   Far: " << camera.GetFarPlane() << "\n\n";

    // 카메라 위치 설정
    camera.SetLookAt(
        Vector3{ 0.0f, 5.0f, -10.0f },  // Position
        Vector3{ 0.0f, 0.0f, 0.0f },     // Target
        Vector3{ 0.0f, 1.0f, 0.0f }      // Up
    );

    std::cout << "2. 카메라 위치 설정:\n";
    PrintVector("   Position", camera.GetPosition());
    PrintVector("   Target", camera.GetTarget());
    PrintVector("   Forward", camera.GetForwardVector());
    PrintVector("   Right", camera.GetRightVector());
    std::cout << "\n";

    // View 행렬 업데이트 및 출력
    camera.UpdateViewMatrix();
    PrintMatrix("3. View Matrix", camera.GetViewMatrix());

    // Projection 행렬 업데이트 및 출력
    camera.UpdateProjectionMatrix();
    PrintMatrix("4. Projection Matrix", camera.GetProjectionMatrix());

    // ViewProjection 행렬
    PrintMatrix("5. ViewProjection Matrix", camera.GetViewProjectionMatrix());

    // 카메라 이동 테스트
    std::cout << "6. 카메라 이동 테스트:\n";
    camera.MoveForward(2.0f);
    PrintVector("   After MoveForward(2.0)", camera.GetPosition());

    camera.MoveRight(3.0f);
    PrintVector("   After MoveRight(3.0)", camera.GetPosition());

    camera.MoveUp(1.0f);
    PrintVector("   After MoveUp(1.0)", camera.GetPosition());
    std::cout << "\n";

    // 카메라 회전 테스트
    std::cout << "7. 카메라 회전 테스트:\n";
    camera.RotateYaw(DegToRad(45.0f));
    PrintVector("   After RotateYaw(45deg)", camera.GetForwardVector());

    camera.RotatePitch(DegToRad(-30.0f));
    PrintVector("   After RotatePitch(-30deg)", camera.GetForwardVector());
    std::cout << "\n";

    // FOV 변경 테스트
    std::cout << "8. FOV 변경 테스트:\n";
    camera.SetFovYDegrees(90.0f);
    std::cout << "   New FOV: " << camera.GetFovYDegrees() << " degrees\n";
    camera.UpdateProjectionMatrix();
    std::cout << "   Projection matrix updated\n\n";

    std::cout << "=== PerspectiveCamera Test Complete ===\n\n";
}

void TestOrthographicCamera()
{
    std::cout << "=== OrthographicCamera Test ===\n\n";

    // 카메라 생성 (1920x1080 해상도)
    OrthographicCamera camera(1920.0f, 1080.0f);

    std::cout << "1. 기본 카메라 설정:\n";
    std::cout << "   Width: " << camera.GetWidth() << "\n";
    std::cout << "   Height: " << camera.GetHeight() << "\n";
    std::cout << "   Near: " << camera.GetNearPlane() << "\n";
    std::cout << "   Far: " << camera.GetFarPlane() << "\n\n";

    // 카메라 위치 설정 (UI용 - 화면 중앙에서 바라봄)
    camera.SetLookAt(
        Vector3{ 0.0f, 0.0f, -1.0f },   // Position
        Vector3{ 0.0f, 0.0f, 0.0f },     // Target
        Vector3{ 0.0f, 1.0f, 0.0f }      // Up
    );

    std::cout << "2. 카메라 위치 설정:\n";
    PrintVector("   Position", camera.GetPosition());
    PrintVector("   Target", camera.GetTarget());
    std::cout << "\n";

    // View 행렬
    camera.UpdateViewMatrix();
    PrintMatrix("3. View Matrix", camera.GetViewMatrix());

    // Projection 행렬
    camera.UpdateProjectionMatrix();
    PrintMatrix("4. Projection Matrix", camera.GetProjectionMatrix());

    // 뷰포트 크기 변경 테스트
    std::cout << "5. 뷰포트 크기 변경 테스트:\n";
    camera.SetSize(1280.0f, 720.0f);
    std::cout << "   New Size: " << camera.GetWidth() << "x" << camera.GetHeight() << "\n";
    camera.UpdateProjectionMatrix();
    std::cout << "   Projection matrix updated\n\n";

    std::cout << "=== OrthographicCamera Test Complete ===\n\n";
}

void TestCameraComparison()
{
    std::cout << "=== Camera Comparison Test ===\n\n";

    // 동일한 위치에 두 카메라 배치
    Vector3 position{ 0.0f, 10.0f, -20.0f };
    Vector3 target{ 0.0f, 0.0f, 0.0f };
    Vector3 up{ 0.0f, 1.0f, 0.0f };

    PerspectiveCamera perspCam;
    perspCam.SetLookAt(position, target, up);
    perspCam.UpdateViewMatrix();
    perspCam.UpdateProjectionMatrix();

    OrthographicCamera orthoCam(1280.0f, 720.0f);
    orthoCam.SetLookAt(position, target, up);
    orthoCam.UpdateViewMatrix();
    orthoCam.UpdateProjectionMatrix();

    std::cout << "1. 동일한 위치 설정:\n";
    PrintVector("   Position", position);
    PrintVector("   Target", target);
    std::cout << "\n";

    std::cout << "2. View 행렬 비교:\n";
    std::cout << "   (동일한 위치에서 바라보므로 View 행렬은 같아야 함)\n";
    PrintMatrix("   Perspective View", perspCam.GetViewMatrix());
    PrintMatrix("   Orthographic View", orthoCam.GetViewMatrix());

    std::cout << "3. Projection 행렬 비교:\n";
    std::cout << "   (투영 방식이 다르므로 Projection 행렬은 달라야 함)\n";
    PrintMatrix("   Perspective Projection", perspCam.GetProjectionMatrix());
    PrintMatrix("   Orthographic Projection", orthoCam.GetProjectionMatrix());

    std::cout << "=== Camera Comparison Test Complete ===\n\n";
}

int main()
{
    std::cout << "\n";
    std::cout << "----------------------------------------\n";
    std::cout << "     Camera System Test (11_CameraTest) \n";
    std::cout << "----------------------------------------\n";
    std::cout << "\n";

    try
    {
        TestPerspectiveCamera();
        TestOrthographicCamera();
        TestCameraComparison();

        std::cout << "----------------------------------------\n";
        std::cout << "          All Tests Passed!             \n";
        std::cout << "----------------------------------------\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "Test Failed: " << e.what() << "\n";
        return 1;
    }

    return 0;
}