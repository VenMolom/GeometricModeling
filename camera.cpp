//
// Created by Molom on 2022-02-27.
//

#include "camera.h"

using namespace DirectX;

Camera::Camera() {
    calculateProjection(1);
    calculateView();
}

XMMATRIX Camera::viewMatrix() const {
    return XMLoadFloat4x4(&view.value());
}

XMMATRIX Camera::projectionMatrix() const {
    return XMLoadFloat4x4(&projection.value());
}

XMMATRIX Camera::cameraMatrix() const {
    return XMLoadFloat4x4(&view.value()) * XMLoadFloat4x4(&projection.value());
}

void Camera::resize(QSizeF newSize) {
    viewportSize = newSize;
    calculateProjection(newSize.width() / newSize.height());
}

void Camera::changeZoom(float delta) {
    if (delta <= 0) {
        zoom *= STEP;
    } else {
        zoom /= STEP;
    }

    calculateView();
}

void Camera::rotate(QPointF angle) {
    auto rotate = angle * SENSITIVITY;

    yaw += static_cast<float>(rotate.x());
    pitch = std::clamp<float>(pitch + static_cast<float>(rotate.y()), MIN_ANGLE, MAX_ANGLE);

    _direction = {
            cos(yaw) * cos(pitch),
            sin(pitch),
            sin(yaw) * cos(pitch),
    };
    XMStoreFloat3(&_direction, XMVector3Normalize(XMLoadFloat3(&_direction)));

    XMStoreFloat3(&right, XMVector3Normalize(
            XMVector3Cross(
                    XMLoadFloat3(&worldUp),
                    XMLoadFloat3(&_direction)
            )
    ));
    XMStoreFloat3(&up, XMVector3Normalize(
            XMVector3Cross(
                    XMLoadFloat3(&_direction),
                    XMLoadFloat3(&right)
            )
    ));

    calculateView();
}

void Camera::move(QPointF offset) {
    auto moveRight = XMVectorScale(XMLoadFloat3(&right), SPEED * offset.x());
    auto moveUp = XMVectorScale(XMLoadFloat3(&up), -SPEED * offset.y());
    auto move = XMVectorAdd(moveRight, moveUp);

    XMStoreFloat3(&_center, XMVectorAdd(XMLoadFloat3(&_center), move));

    calculateView();
}

void Camera::moveTo(DirectX::XMFLOAT3 position) {
    _center = position;
    calculateView();
}

void Camera::calculateView() {
    auto centerVector = XMLoadFloat3(&_center);
    auto fromCenterVector = XMVectorScale(XMLoadFloat3(&_direction), distance * zoom);

    XMFLOAT4X4 v{};
    XMStoreFloat4x4(&v, XMMatrixLookAtRH(
            XMVectorAdd(centerVector, fromCenterVector),
            centerVector,
            XMLoadFloat3(&up)));
    view.setValue(v);
}

void Camera::calculateProjection(float aspectRatio) {
    XMFLOAT4X4 proj{};
    XMStoreFloat4x4(&proj, XMMatrixPerspectiveFovRH(
            XMConvertToRadians(90),
            aspectRatio, _near, _far));
    projection.setValue(proj);
}