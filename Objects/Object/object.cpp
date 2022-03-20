//
// Created by Molom on 2022-02-27.
//

#include "object.h"

#include <utility>

using namespace DirectX;

Object::Object(QString name, XMFLOAT3 position)
        : _position(position),
          _name(name) {
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

void Object::setName(QString name) {
    _name = std::move(name);
}
