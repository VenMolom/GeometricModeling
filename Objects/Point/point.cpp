//
// Created by Molom on 2022-03-19.
//

#include "point.h"

using namespace DirectX;

float POINT_SIZE = 2.f;

Point::Point(uint id, DirectX::XMFLOAT3 position) : Object(id, "Point", position, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST) {
    vertices.push_back({{0, 0, 0},
                        {1, 1, 1}});
    updateBuffers();
}

void Point::draw(Renderer &renderer, DrawType drawType) {
    renderer.draw(*this, drawType != DEFAULT ? SELECTED_COLOR : DEFAULT_COLOR);
}

Type Point::type() const {
    return POINT3D;
}

bool Point::intersects(DirectX::XMFLOAT3 origin, DirectX::XMFLOAT3 direction, DirectX::XMMATRIX viewMatrix,
                       float nearZ, float farZ, float &distance) const {
    auto ndc = XMVector3Transform(XMLoadFloat3(&_position.value()), viewMatrix);
    auto depth = (abs(ndc.m128_f32[2]) - nearZ) / (farZ - nearZ);

    auto size = POINT_SIZE * depth;
    auto boundingSphere = BoundingSphere{_position.value(), size};
    return boundingSphere.Intersects(XMLoadFloat3(&origin), XMLoadFloat3(&direction), distance);
}

MG1::Point Point::serialize() {
    MG1::Point point{};
    point.SetId(id());
    point.name = name().toStdString();
    auto pos = _position.value();
    point.position = {pos.x, pos.y, pos.z};
    return point;
}

Point::Point(const MG1::Point &point) : Point(point.GetId(), {point.position.x, point.position.y, point.position.z}) {
    setName(QString::fromStdString(point.name));
}
