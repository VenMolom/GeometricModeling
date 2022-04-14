//
// Created by Molom on 2022-02-27.
//

#include "object.h"

#include <utility>

using namespace DirectX;

Object::Object(QString name, XMFLOAT3 position, D3D11_PRIMITIVE_TOPOLOGY topology)
        : Renderable(topology),
          _position(position),
          _name(name) {
    calculateModel();
}

XMMATRIX Object::modelMatrix() const {
    return XMLoadFloat4x4(&model);
}

void Object::calculateModel() {
    XMStoreFloat4x4(&rotationMatrix, XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&_rotation.value())));
    XMStoreFloat4x4(&noScaleMatrix, XMLoadFloat4x4(&rotationMatrix) *
                                    XMMatrixTranslationFromVector(XMLoadFloat3(&_position.value())));
    XMStoreFloat4x4(&scaleMatrix, XMMatrixScalingFromVector(XMLoadFloat3(&_scale.value())));
    XMStoreFloat4x4(&model, XMLoadFloat4x4(&scaleMatrix) * XMLoadFloat4x4(&noScaleMatrix));
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

bool Object::equals(const std::shared_ptr<Object> &other) const {
    return other.get() == this;
}
