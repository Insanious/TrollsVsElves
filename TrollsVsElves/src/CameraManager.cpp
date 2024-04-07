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
    // Camera panning
    float cameraPan = 2.0f;

    if (IsKeyDown(KEY_A))      CameraMoveRight(&camera, -cameraPan, true);
    else if (IsKeyDown(KEY_D)) CameraMoveRight(&camera, cameraPan, true);

    if (IsKeyDown(KEY_W))      CameraMoveForward(&camera, cameraPan, true);
    else if (IsKeyDown(KEY_S)) CameraMoveForward(&camera, -cameraPan, true);

    // Camera zooming
    float maxDistance = 120.f;
    float minDistance = 10.f;
    float scrollAmount = 1.f;
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
