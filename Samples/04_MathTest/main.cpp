#include "Math/MathTypes.h"
#include "Math/MathUtils.h"
#include <iostream>
#include <iomanip>

using namespace Math;

void PrintVector3(const char* name, const Vector3& v)
{
    std::cout << name << ": ("
        << std::fixed << std::setprecision(2)
        << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
}

void PrintMatrix4x4(const char* name, const Matrix4x4& m)
{
    std::cout << name << ":" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    for (int row = 0; row < 4; ++row)
    {
        std::cout << "  ";
        for (int col = 0; col < 4; ++col)
        {
            std::cout << std::setw(8) << m.m[row][col] << " ";
        }
        std::cout << std::endl;
    }
}

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "    Math Library Test" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // Test 1: Vector Operations
    std::cout << "Test 1: Vector Operations" << std::endl;
    Vector3 a = { 1.0f, 2.0f, 3.0f };
    Vector3 b = { 4.0f, 5.0f, 6.0f };

    PrintVector3("  Vector A", a);
    PrintVector3("  Vector B", b);

    Vector3 sum = Add(a, b);
    PrintVector3("  A + B", sum);

    Vector3 diff = Subtract(a, b);
    PrintVector3("  A - B", diff);

    float dotProduct = Dot(a, b);
    std::cout << "  Dot(A, B): " << dotProduct << std::endl;

    Vector3 cross = Cross(a, b);
    PrintVector3("  Cross(A, B)", cross);

    float len = Length(a);
    std::cout << "  Length(A): " << len << std::endl;

    Vector3 normalized = Normalize(a);
    PrintVector3("  Normalize(A)", normalized);
    std::cout << std::endl;

    // Test 2: Matrix Operations
    std::cout << "Test 2: Matrix Operations" << std::endl;

    Matrix4x4 identity = MatrixIdentity();
    PrintMatrix4x4("  Identity Matrix", identity);
    std::cout << std::endl;

    Matrix4x4 translation = MatrixTranslation(10.0f, 20.0f, 30.0f);
    PrintMatrix4x4("  Translation(10, 20, 30)", translation);
    std::cout << std::endl;

    Matrix4x4 rotationY = MatrixRotationY(DegToRad(45.0f));
    PrintMatrix4x4("  Rotation Y (45 degrees)", rotationY);
    std::cout << std::endl;

    Matrix4x4 scaling = MatrixScaling(2.0f, 2.0f, 2.0f);
    PrintMatrix4x4("  Scaling(2, 2, 2)", scaling);
    std::cout << std::endl;

    // Test 3: Transform Composition
    std::cout << "Test 3: Transform Composition" << std::endl;
    Matrix4x4 transform = MatrixMultiply(scaling, rotationY);
    transform = MatrixMultiply(transform, translation);
    PrintMatrix4x4("  Scale * Rotation * Translation", transform);
    std::cout << std::endl;

    // Test 4: Camera Matrices
    std::cout << "Test 4: Camera Matrices" << std::endl;

    Vector3 eye = { 0.0f, 5.0f, -10.0f };
    Vector3 target = { 0.0f, 0.0f, 0.0f };
    Vector3 up = { 0.0f, 1.0f, 0.0f };

    Matrix4x4 view = MatrixLookAtLH(eye, target, up);
    PrintMatrix4x4("  View Matrix", view);
    std::cout << std::endl;

    Matrix4x4 proj = MatrixPerspectiveFovLH(
        DegToRad(60.0f),  // FOV
        16.0f / 9.0f,     // aspect ratio
        0.1f,             // near plane
        1000.0f           // far plane
    );
    PrintMatrix4x4("  Projection Matrix (FOV 60¡Æ, 16:9)", proj);
    std::cout << std::endl;

    // Test 5: Utility Functions
    std::cout << "Test 5: Utility Functions" << std::endl;
    std::cout << "  45 degrees to radians: " << DegToRad(45.0f) << std::endl;
    std::cout << "  PI radians to degrees: " << RadToDeg(PI) << std::endl;
    std::cout << "  Clamp(5.5, 0, 10): " << Clamp(5.5f, 0.0f, 10.0f) << std::endl;
    std::cout << "  Clamp(15, 0, 10): " << Clamp(15.0f, 0.0f, 10.0f) << std::endl;

    Vector3 v1 = { 0.0f, 0.0f, 0.0f };
    Vector3 v2 = { 10.0f, 0.0f, 0.0f };
    Vector3 lerped = Lerp(v1, v2, 0.5f);
    PrintVector3("  Lerp(v1, v2, 0.5)", lerped);
    std::cout << std::endl;

    std::cout << "========================================" << std::endl;
    std::cout << "    All tests completed!" << std::endl;
    std::cout << "========================================" << std::endl;

    // Test 6: Quaternion Operations
    std::cout << "Test 6: Quaternion Operations" << std::endl;

    // Identity quaternion
    Quaternion identityQuat = QuaternionIdentity();
    std::cout << "  Identity Quat: ("
        << identityQuat.x << ", " << identityQuat.y << ", "
        << identityQuat.z << ", " << identityQuat.w << ")" << std::endl;

    // From Euler angles (Y-axis 90 degrees)
    Quaternion rotQuat = QuaternionFromEuler(0.0f, DegToRad(90.0f), 0.0f);
    std::cout << "  Rotation(90¡Æ Y): ("
        << rotQuat.x << ", " << rotQuat.y << ", "
        << rotQuat.z << ", " << rotQuat.w << ")" << std::endl;

    // Rotate a vector
    Vector3 forward = { 0.0f, 0.0f, 1.0f };
    Vector3 rotated = QuaternionRotateVector(rotQuat, forward);
    PrintVector3("  Rotated Forward", rotated);

    // Quaternion to matrix
    Matrix4x4 rotMatrix = MatrixRotationQuaternion(rotQuat);
    PrintMatrix4x4("  Quaternion to Matrix", rotMatrix);

    // Slerp
    Quaternion quat1 = QuaternionFromEuler(0.0f, 0.0f, 0.0f);
    Quaternion quat2 = QuaternionFromEuler(0.0f, DegToRad(180.0f), 0.0f);
    Quaternion slerped = QuaternionSlerp(quat1, quat2, 0.5f);
    std::cout << "  Slerp(0¡Æ to 180¡Æ, t=0.5): ("
        << slerped.x << ", " << slerped.y << ", "
        << slerped.z << ", " << slerped.w << ")" << std::endl;
    std::cout << std::endl;

}