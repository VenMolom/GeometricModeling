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


DirectX::XMFLOAT3 Camera::position() const {
    XMFLOAT3 position{};
    XMStoreFloat3(&position, XMVectorAdd(XMLoadFloat3(&_center),
                                         XMVectorScale(XMLoadFloat3(&_direction), distance * zoom)));
    return position;
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

    XMStoreFloat3(&_right, XMVector3Normalize(
            XMVector3Cross(
                    XMLoadFloat3(&worldUp),
                    XMLoadFloat3(&_direction)
            )
    ));
    XMStoreFloat3(&_up, XMVector3Normalize(
            XMVector3Cross(
                    XMLoadFloat3(&_direction),
                    XMLoadFloat3(&_right)
            )
    ));

    calculateView();
}

void Camera::move(QPointF offset) {
    auto moveRight = XMVectorScale(XMLoadFloat3(&_right), SPEED * offset.x());
    auto moveUp = XMVectorScale(XMLoadFloat3(&_up), -SPEED * offset.y());
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
            XMLoadFloat3(&_up)));
    view.setValue(v);
}

void Camera::calculateProjection(float aspectRatio) {
    _aspectRatio = aspectRatio;
    XMFLOAT4X4 proj{};
    XMStoreFloat4x4(&proj, XMMatrixPerspectiveFovRH(
            XMConvertToRadians(FOV),
            _aspectRatio, _near, _far));
    projection.setValue(proj);

//    _z = ((viewportSize.height() / 2.0f) / tan(FOV / 2.0f)) / viewport().height();
    _z = 1.0f / tan(XMConvertToRadians(FOV) / 2.0f);
}

std::tuple<XMMATRIX, XMMATRIX> Camera::stereoscopicViewMatrix() const {
    auto center = XMLoadFloat3(&_center);
    auto position = XMVectorAdd(center, XMVectorScale(
            XMLoadFloat3(&_direction), distance * zoom));
    auto shift = XMVectorScale(XMLoadFloat3(&_right), _eyesDistance / 2.0f);

    auto viewLeft = XMMatrixLookAtRH(XMVectorAdd(position, shift),
            XMVectorAdd(center, shift), XMLoadFloat3(&_up));
    auto viewRight = XMMatrixLookAtRH(XMVectorAdd(position, shift),
                                     XMVectorAdd(center, shift), XMLoadFloat3(&_up));

    return std::make_tuple(viewLeft, viewRight);
}

std::tuple<XMMATRIX, XMMATRIX> Camera::stereoscopicProjectionMatrix() const {
    float shift = (_eyesDistance / 2.0f) * _near / _focusDistance;
    float top = tan(XMConvertToRadians(FOV) / 2.0f) * _near;
    float right = _aspectRatio * top;

    auto projLeft = XMMatrixPerspectiveOffCenterRH(-right + shift, right + shift,
                                                   -top, top, _near, _far);
    auto projRight = XMMatrixPerspectiveOffCenterRH(-right - shift, right - shift,
                                                    -top, top, _near, _far);

    return std::make_tuple(projLeft, projRight);
}
