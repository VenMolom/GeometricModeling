//
// Created by Molom on 2022-02-27.
//

#include "object.h"

using namespace DirectX;

Object::Object(XMFLOAT3 position, XMFLOAT3 color)
        : _position(position),
          _color(color) {

    calculateModel();
}

XMMATRIX Object::modelMatrix() const {
    return XMLoadFloat4x4(&model);
}

void Object::calculateModel() {
    XMStoreFloat4x4(&model,
                    XMMatrixScalingFromVector(XMLoadFloat3(&_scale.value())) *
                    XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&_rotation.value())) *
                    XMMatrixTranslationFromVector(XMLoadFloat3(&_position.value()))
    );
}

void Object::setPosition(DirectX::XMFLOAT3 position) {
    _position = position;
    calculateModel();
}

void Object::setRotation(DirectX::XMFLOAT3 rotation) {
    _rotation = rotation;
    calculateModel();
}

void Object::setScale(DirectX::XMFLOAT3 scale) {
    _scale = scale;
    calculateModel();
}

void Object::setColor(DirectX::XMFLOAT3 color) {
    _color = color;
}
