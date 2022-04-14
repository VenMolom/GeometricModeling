//
// Created by Molom on 2022-03-19.
//

#include "point.h"

using namespace DirectX;

const std::vector<VertexPositionColor> Point::pointVertices = {
        {{1,  1,  0}, {1, 1, 1}},
        {{-1, 1,  0}, {1, 1, 1}},
        {{-1, -1, 0}, {1, 1, 1}},
        {{1,  -1, 0}, {1, 1, 1}}
};

const std::vector<Index> Point::pointIndices = {
        0, 1, 2, 3, 0
};

Point::Point(DirectX::XMFLOAT3 position) : Object("Point", position, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP) {
    _scale.setValue(size);
    setBuffers(pointVertices, pointIndices);
}

void Point::draw(Renderer &renderer, DrawType drawType) {
    renderer.draw(*this, drawType != DEFAULT ? SELECTED_COLOR : DEFAULT_COLOR);
    // TODO: move to shader
//    auto t = XMMatrixTranslationFromVector(XMLoadFloat3(&_position.value()));
//    auto s = XMMatrixScalingFromVector(XMLoadFloat3(&size));
//    auto w = XMFLOAT4(0, 0, 0, 1);
//    auto vInv = XMMatrixInverse(nullptr, view);
//    auto r = vInv * XMMatrixTranslationFromVector(XMVectorScale(
//            XMVector4Transform(XMLoadFloat4(&w), vInv), -1));
//
//    auto mvp = r * s * t * view * projection;
//    renderer.drawIndexed(pointVertices, pointIndices, LineStrip, mvp,
//                         drawType != DEFAULT ? SELECTED_COLOR : DEFAULT_COLOR);
}

Type Point::type() const {
    return POINT3D;
}

BoundingOrientedBox Point::boundingBox() const {
    auto pos = _position.value();
    return {pos, boundingBoxSize, rot};
}
