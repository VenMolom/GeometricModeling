//
// Created by Molom on 2022-03-19.
//

#include <DirectXMath.h>
#include "point.h"

using namespace DirectX;

Point::Point(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 color) : Object("Point", position, color) {

}

void Point::draw(Renderer &renderer, const Camera &camera) const {
    auto t = XMMatrixTranslationFromVector(XMLoadFloat3(&_position.value()));
    auto s = XMMatrixScaling(0.5f, 0.5f, 0.5f);
    auto w = XMFLOAT4(0, 0, 0, 1);
    auto vInv = XMMatrixInverse(nullptr, camera.viewMatrix());
    auto r = vInv * XMMatrixTranslationFromVector(XMVectorScale(
            XMVector4Transform(XMLoadFloat4(&w), vInv), -1));

    auto mvp = r * s * t * camera.cameraMatrix();
    renderer.drawPoint(mvp);
}

Type Point::type() const {
    return POINT3D;
}
