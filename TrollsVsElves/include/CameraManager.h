#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include "utils.h"
#include "structs.h"
#include "rcamera.h"

class CameraManager
{
private:
    CameraManager();
    Camera3D camera;
    Camera2D camera2D;
    Matrix cameraViewMatrix;


public:
    static CameraManager& get()
    {
        static CameraManager instance;
        return instance;
    }

    void update();

    Camera3D& getCamera();
    Camera2D& getCamera2D();
    Matrix getCameraViewMatrix();
    Ray getMouseRay();
    Vector2 getWorldToScreen(Vector3 position);

    float calculateCircleRadius2D(Vector3 position, float radius);
    Circle convertSphereToCircle(Vector3 position, float radius);
};

#endif
