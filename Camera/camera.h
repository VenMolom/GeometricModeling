//
// Created by Molom on 2022-02-27.
//

#ifndef MG1_CAMERA_H
#define MG1_CAMERA_H

#include <DirectXMath.h>
#include <QRect>

#define STEP 1.2f
#define SENSITIVITY 0.005f
#define SPEED 0.01f
#define LIMIT DirectX::XM_PIDIV2 - 0.1f

class Camera {
public:
    Camera();

    Camera(Camera &&camera) = default;

    DirectX::XMMATRIX viewMatrix() const;

    void resize(QSizeF newSize);

    void changeZoom(float delta, QSizeF viewportSize);

    void rotate(QPointF angle);

    void move(QPointF direction);

private:
    DirectX::XMFLOAT4X4 projection;
    DirectX::XMFLOAT4X4 view;

    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 center;
    DirectX::XMFLOAT3 front;
    DirectX::XMFLOAT3 up;
    DirectX::XMFLOAT3 worldUp;
    DirectX::XMFLOAT3 right;

    float distance;
    float zoom;
    float yaw;
    float pitch;

    void calculateView();

    void calculateProjection(float aspectRatio);
};


#endif //MG1_CAMERA_H
