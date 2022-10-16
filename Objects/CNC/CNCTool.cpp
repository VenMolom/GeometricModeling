//
// Created by Molom on 15/10/2022.
//

#include "CNCTool.h"

using namespace DirectX;

const XMFLOAT3 CNCTool::COLOR = {0.5f, 0.5f, 0.5f};

CNCTool::CNCTool(XMFLOAT3 position) {
    setPosition(position);
    XMStoreFloat4x4(&scaleMatrix, XMMatrixIdentity());
    XMStoreFloat4x4(&sphericalScaleMatrix, XMMatrixIdentity());

    cylinder = Mesh::cylinder(0.5f, HEIGHT, 1, 32, COLOR, XMMatrixTranslation(0.f, 0.f, HEIGHT / 2.f));
    topCap = Mesh::disk(0.5f, 32, COLOR, XMMatrixTranslation(0.f, 0.f, HEIGHT));
    bottomCap = Mesh::disk(0.5f, 32, COLOR, XMMatrixRotationX(XM_PI));
    sphericalCap = Mesh::dome(0.5f, 32, 32, COLOR, XMMatrixRotationX(XM_PI));
}

void CNCTool::setType(CNCType type) {
    if (_type == type) return;
    _type = type;

    switch (_type) {
        case Flat:
            XMStoreFloat4x4(&positionMatrix,
                            XMLoadFloat4x4(&positionMatrix) * XMMatrixTranslation(0.f, 0.f, -_size / 2.f));
            break;
        case Round:
            XMStoreFloat4x4(&positionMatrix,
                            XMLoadFloat4x4(&positionMatrix) * XMMatrixTranslation(0.f, 0.f, _size / 2.f));
            break;
    }
}

void CNCTool::setSize(int size) {
    _size = size / 10.f;
    XMStoreFloat4x4(&scaleMatrix, XMMatrixScaling(_size, _size, 1.f));
    XMStoreFloat4x4(&sphericalScaleMatrix, XMMatrixScaling(_size, _size, _size));
}

void CNCTool::draw(Renderer &renderer, XMMATRIX parentMatrix) {
    auto scale = XMLoadFloat4x4(&scaleMatrix);
    auto position = XMLoadFloat4x4(&positionMatrix);
    renderer.draw(cylinder, scale * cylinder.modelMatrix() * position * parentMatrix);
    renderer.draw(topCap, scale * topCap.modelMatrix() * position * parentMatrix);
    if (_type == CNCType::Flat) {
        renderer.draw(bottomCap, scale * bottomCap.modelMatrix() * position * parentMatrix);
    } else {
        renderer.draw(sphericalCap,
                      XMLoadFloat4x4(&sphericalScaleMatrix) * sphericalCap.modelMatrix() * position * parentMatrix);
    }
}

void CNCTool::setPosition(DirectX::XMFLOAT3 position) {
    _position = position;
    XMStoreFloat4x4(&positionMatrix, XMMatrixTranslationFromVector(XMLoadFloat3(&position))
                                     * (_type == CNCType::Round
                                        ? XMMatrixTranslation(0.f, 0.f, _size / 2.f)
                                        : XMMatrixIdentity()));
}
