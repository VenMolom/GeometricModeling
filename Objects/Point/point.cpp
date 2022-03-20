//
// Created by Molom on 2022-03-19.
//

#include <DirectXMath.h>
#include "point.h"

using namespace DirectX;

Point::Point(DirectX::XMFLOAT3 position) : Object("Point", position) {

}

void Point::draw(Renderer &renderer, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, DrawType drawType) const {
    auto t = XMMatrixTranslationFromVector(XMLoadFloat3(&_position.value()));
    auto s = XMMatrixScalingFromVector(XMLoadFloat3(&size));
    auto w = XMFLOAT4(0, 0, 0, 1);
    auto vInv = XMMatrixInverse(nullptr, view);
    auto r = vInv * XMMatrixTranslationFromVector(XMVectorScale(
            XMVector4Transform(XMLoadFloat4(&w), vInv), -1));

    auto mvp = r * s * t * view * projection;
    renderer.drawPoint(mvp, drawType != DEFAULT);
}

Type Point::type() const {
    return POINT3D;
}

BoundingOrientedBox Point::boundingBox() const {
    auto pos = _position.value();
    return {pos, size, rot};
}
