//
// Created by Molom on 2022-02-27.
//

#include "camera.h"

using namespace DirectX;

Camera::Camera() : direction(0, 0, 1),
                   center(0, 0, 0),
                   up(0, 1, 0),
                   worldUp(0, 1, 0),
                   right(1, 0, 0),
                   distance(10),
                   zoom(1),
                   yaw(XM_PIDIV2),
                   pitch(0) {
    calculateProjection(1);
    calculateView();
}

XMMATRIX Camera::viewMatrix() const {
    return XMLoadFloat4x4(&view) * XMLoadFloat4x4(&projection);
}

void Camera::resize(QSizeF newSize) {
    calculateProjection(newSize.width() / newSize.height());
}

void Camera::changeZoom(float delta, QSizeF viewportSize) {
    if (delta <= 0) {
        zoom *= STEP;
    } else {
        zoom /= STEP;
    }

    resize(viewportSize);
    calculateView();
}

void Camera::rotate(QPointF angle) {
    auto rotate = angle * SENSITIVITY;

    yaw += static_cast<float>(rotate.x());
    pitch = std::clamp<float>(pitch + static_cast<float>(rotate.y()), MIN_ANGLE, MAX_ANGLE);

    direction = {
            cos(yaw) * cos(pitch),
            sin(pitch),
            sin(yaw) * cos(pitch),
    };
    XMStoreFloat3(&direction, XMVector3Normalize(XMLoadFloat3(&direction)));

    XMStoreFloat3(&right, XMVector3Normalize(
            XMVector3Cross(
                    XMLoadFloat3(&worldUp),
                    XMLoadFloat3(&direction)
            )
    ));
    XMStoreFloat3(&up, XMVector3Normalize(
            XMVector3Cross(
                    XMLoadFloat3(&direction),
                    XMLoadFloat3(&right)
            )
    ));

    calculateView();
}

void Camera::move(QPointF offset) {
    auto moveRight = XMVectorScale(XMLoadFloat3(&right), SPEED * offset.x());
    auto moveUp = XMVectorScale(XMLoadFloat3(&up), -SPEED * offset.y());
    auto move = XMVectorAdd(moveRight, moveUp);

    XMStoreFloat3(&center, XMVectorAdd(XMLoadFloat3(&center), move));

    calculateView();
}

void Camera::calculateView() {
    auto centerVector = XMLoadFloat3(&center);
    auto fromCenterVector = XMVectorScale(XMLoadFloat3(&direction), distance * zoom);

    XMStoreFloat4x4(&view, XMMatrixLookAtRH(
            XMVectorAdd(centerVector, fromCenterVector),
            centerVector,
            XMLoadFloat3(&up)
    ));
}

void Camera::calculateProjection(float aspectRatio) {
    XMStoreFloat4x4(&projection, XMMatrixPerspectiveFovRH(
            XMConvertToRadians(90),
            aspectRatio, 0.1f, 100.0f));
}
