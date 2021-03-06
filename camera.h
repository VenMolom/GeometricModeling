//
// Created by Molom on 2022-02-27.
//

#ifndef MG1_CAMERA_H
#define MG1_CAMERA_H

#include <DirectXMath.h>
#include <QRect>
#include <QProperty>
#include <tuple>

#define STEP 1.1f
#define SENSITIVITY 0.005f
#define SPEED 0.05f
#define LIMIT DirectX::XM_PIDIV2 - 0.001f
#define MAX_ANGLE  LIMIT
#define MIN_ANGLE -(LIMIT)

#define FOV 90.0f

class Camera {
public:
    Camera();

    DirectX::XMMATRIX viewMatrix() const;

    QBindable<DirectX::XMFLOAT4X4> bindableView() { return &view; }

    DirectX::XMMATRIX projectionMatrix() const;

    QBindable<DirectX::XMFLOAT4X4> bindableProjection() { return &projection; }

    std::tuple<DirectX::XMMATRIX, DirectX::XMMATRIX> stereoscopicProjectionMatrix() const;

    std::tuple<DirectX::XMMATRIX, DirectX::XMMATRIX> stereoscopicViewMatrix() const;

    DirectX::XMMATRIX cameraMatrix() const;

    QSizeF viewport() const { return viewportSize; }

    DirectX::XMFLOAT3 position() const;

    DirectX::XMFLOAT3 right() const { return _right; }

    DirectX::XMFLOAT3 up() const { return _up; }

    DirectX::XMFLOAT3 center() const { return _center; }

    DirectX::XMFLOAT3 direction() const { return _direction; }

    void setEyesDistance(float eyesDistance) { _eyesDistance = eyesDistance; }

    void setFocusDistance(float focusDistance) { _focusDistance = focusDistance; }

    float nearZ() const { return _near; }

    float farZ() const { return _far; }

    float z() const { return _z; }

    void resize(QSizeF newSize);

    void changeZoom(float delta);

    void rotate(QPointF angle);

    void move(QPointF offset);

    void moveTo(DirectX::XMFLOAT3 position);

private:
    QProperty<DirectX::XMFLOAT4X4> projection;
    QProperty<DirectX::XMFLOAT4X4> view;

    DirectX::XMFLOAT3 _direction{0, 0, 1};
    DirectX::XMFLOAT3 _center{0, 0, 0};
    DirectX::XMFLOAT3 _up{0, 1, 0};
    DirectX::XMFLOAT3 _right{1, 0, 0};
    DirectX::XMFLOAT3 worldUp{0, 1, 0};

    QSizeF viewportSize;

    float distance{10};
    float zoom{1};
    float yaw{DirectX::XM_PIDIV2};
    float pitch{0};
    float _near{0.1f};
    float _far{100.0f};
    float _z{0};
    float _aspectRatio{1};

    float _eyesDistance{1};
    float _focusDistance{10};

    void calculateView();

    void calculateProjection(float aspectRatio);
};


#endif //MG1_CAMERA_H
