#include "CameraManager.h"

CameraManager::CameraManager()
{
    camera = {
        .position = { 30.0f, 60.0f, 30.0f },
        .target = { 0.f, 0.f, 0.f },
        .up = { 0.0f, 1.0f, 0.0f },
        .fovy = 90.0f,
        .projection = CAMERA_PERSPECTIVE,
    };
    // since we are using CAMERA_PERSPECTIVE, this is static apart from the position (m12, m13, m14)
    cameraViewMatrix = GetCameraMatrix(camera);

    camera2D = {
        .offset = { 0.f, 0.f },
        .target = { 0.f, 0.f },
        .rotation = 0.f,
        .zoom = 1.f
    };
}

void CameraManager::update()
{
    float dt = GetFrameTime();
    // Camera panning
    float cameraPan = 150.0f * dt;

    if (IsKeyDown(KEY_A))      CameraMoveRight(&camera, -cameraPan, true);
    else if (IsKeyDown(KEY_D)) CameraMoveRight(&camera, cameraPan, true);

    if (IsKeyDown(KEY_W))      CameraMoveForward(&camera, cameraPan, true);
    else if (IsKeyDown(KEY_S)) CameraMoveForward(&camera, -cameraPan, true);

    // Camera zooming
    float maxDistance = 120.f;
    float minDistance = 10.f;
    float scrollAmount = 100.f * dt;
    float scroll = -GetMouseWheelMove(); // inverted for a reason
    if (scroll)
    {
        bool scrollUp = scroll == 1;
        float distance = Vector3Distance(camera.position, camera.target);
        if (scroll == 1 && distance + scrollAmount > maxDistance) // scrolling up and will go beyond max
            scroll = maxDistance - distance;

        else if (scroll == -1 && distance - scrollAmount < minDistance) // scrolling down and will go beyond min
            scroll = minDistance - distance;

        CameraMoveToTarget(&camera, scroll);
    }
}

Camera3D& CameraManager::getCamera()
{
    return camera;
}

Camera2D& CameraManager::getCamera2D()
{
    return camera2D;
}

Matrix CameraManager::getCameraViewMatrix()
{
    return cameraViewMatrix;
}

Ray CameraManager::getMouseRay()
{
    return GetMouseRay(GetMousePosition(), camera);
}

Vector2 CameraManager::getWorldToScreen(Vector3 position)
{
    return GetWorldToScreen(position, camera);
}

float CameraManager::calculateCircleRadius2D(Vector3 position, float radius)
{
    Vector3 right = { cameraViewMatrix.m0, cameraViewMatrix.m1, cameraViewMatrix.m2 };
    Vector3 rightScaled = Vector3Scale(right, radius);
    Vector3 edgeOfCircle = Vector3Add(position, rightScaled);

    return Vector2Distance(getWorldToScreen(position), getWorldToScreen(edgeOfCircle));
}

Circle CameraManager::convertSphereToCircle(Vector3 position, float radius)
{
    float radius2D = calculateCircleRadius2D(position, radius);
    Vector2 position2D = getWorldToScreen(position);

    return Circle({ radius2D, position2D });
}
