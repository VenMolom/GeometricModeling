//
// Created by Molom on 2022-02-27.
//

#include "object.h"

using namespace DirectX;

Object::Object(XMFLOAT3 position, XMFLOAT3 color)
        : position(position), color(color) {
    XMStoreFloat4x4(&model, XMMatrixTranslationFromVector(XMLoadFloat3(&position)));
}

XMMATRIX Object::modelMatrix() const {
    return XMLoadFloat4x4(&model);
}
