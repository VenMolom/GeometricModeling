//
// Created by Molom on 2022-03-19.
//

#include "point.h"

using namespace DirectX;

const std::vector<VertexPositionColor> Point::pointVertices = {
        {{0.5,  0.5,  0}, {1, 1, 1}},
        {{-0.5, 0.5,  0}, {1, 1, 1}},
        {{-0.5, -0.5, 0}, {1, 1, 1}},
        {{0.5,  -0.5, 0}, {1, 1, 1}}
};

const std::vector<Index> Point::pointIndices = {
        0, 1, 2, 3, 0
};

Point::Point(uint id, DirectX::XMFLOAT3 position) : Object(id, "Point", position, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP) {
    Object::setScale(size);
    setBuffers(pointVertices, pointIndices);
}

void Point::draw(Renderer &renderer, DrawType drawType) {
    renderer.draw(*this, drawType != DEFAULT ? SELECTED_COLOR : DEFAULT_COLOR);
}

Type Point::type() const {
    return POINT3D;
}

bool Point::intersects(DirectX::XMFLOAT3 origin, DirectX::XMFLOAT3 direction, float &distance) const {
    // TODO: calculate depending on camera position
    auto boundingBox = BoundingOrientedBox{_position.value(), boundingBoxSize, rot};
    return boundingBox.Intersects(XMLoadFloat3(&origin), XMLoadFloat3(&direction), distance);
}