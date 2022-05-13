//
// Created by Molom on 2022-03-11.
//

#include "torus.h"
#include <algorithm>

using namespace std;
using namespace DirectX;

Torus::Torus(uint id, XMFLOAT3 position)
        : ParametricObject<TORUS_DIM>(id, "Torus", position, {15, 15},
                                      {make_tuple(0, XM_2PI), make_tuple(0, XM_2PI)},
                                      D3D11_PRIMITIVE_TOPOLOGY_LINELIST) {
    calculateVertices(density(), range());
    calculateIndices(density());
    updateBuffers();
}

void Torus::calculateVertices(const array<int, TORUS_DIM> &density, const array<tuple<float, float>, TORUS_DIM> &range) {
    auto[startU, endU] = range[0];
    auto deltaU = (endU - startU) / static_cast<float>(density[0]);

    auto[startV, endV] = range[1];
    auto deltaV = (endV - startV) / static_cast<float>(density[1]);

    auto u = startU;
    for (auto i = 0; i < density[0]; ++i, u += deltaU) { // minor _rotation
        auto v = startV;
        for (auto j = 0; j < density[1]; ++j, v += deltaV) { // major _rotation
            vertices.push_back({
                                       {
                                               (_majorRadius + _minorRadius * cos(u)) * cos(v),
                                                  _minorRadius * sin(u),
                                                     (_majorRadius + _minorRadius * cos(u)) * sin(v)
                                       },
                                       {       1, 1, 1}
                               });
        }
    }
}

void Torus::calculateIndices(const array<int, TORUS_DIM> &density) {
    auto verticesSize = density[0] * density[1];
    for (auto i = 0; i < density[1]; ++i) { // major _rotation
        for (auto j = 0; j < density[0]; ++j) { // minor _rotation
            // lines around minor _rotation
            auto index = j * density[1] + i;
            indices.push_back(index);
            indices.push_back((index + density[1]) % verticesSize);

            indices.push_back(index);
            indices.push_back((index + 1) % density[1] + j * density[1]);
        }
    }
}

array<bool, 2> Torus::looped() const {
    return {true, true};
}

void Torus::setMajorRadius(float radius) {
    _majorRadius = radius;
    calculateVertices(density(), range());
    calculateIndices(density());
    updateBuffers();
}

void Torus::setMinorRadius(float radius) {
    _minorRadius = radius;
    calculateVertices(density(), range());
    calculateIndices(density());
    updateBuffers();
}

bool Torus::intersects(DirectX::XMFLOAT3 origin, DirectX::XMFLOAT3 direction, DirectX::XMMATRIX viewMatrix,
                       float nearZ, float farZ, float &distance) const {
    auto pos = _position.value();
    auto size = XMFLOAT3(_scale.value().x * (_majorRadius + _minorRadius), _scale.value().y * _minorRadius,
                         _scale.value().z * (_majorRadius + _minorRadius));
    XMFLOAT4 rotation{};
    XMStoreFloat4(&rotation,
                  XMQuaternionRotationRollPitchYaw(_rotation.value().x, _rotation.value().y, _rotation.value().z));
    auto boundingBox =  BoundingOrientedBox{pos, size, rotation};

    return boundingBox.Intersects(XMLoadFloat3(&origin), XMLoadFloat3(&direction), distance);
}

void Torus::densityUpdated() {
    calculateVertices(density(), range());
    calculateIndices(density());
    updateBuffers();
}

void Torus::draw(Renderer &renderer, DrawType drawType) {
    renderer.draw(*this, drawType != DEFAULT ? SELECTED_COLOR : DEFAULT_COLOR);
    if (drawType == SELECTED) {
        Cursor::drawCursor(renderer, position(), rotation());
    }
}
