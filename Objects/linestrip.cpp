//
// Created by Molom on 2022-04-14.
//

#include "linestrip.h"

Linestrip::Linestrip() : Object(0, "", {0, 0, 0}, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP) {

}

void Linestrip::draw(Renderer &renderer, DrawType drawType) {
    if (Renderable::vertices.empty()) return;

    renderer.draw(*this, drawType != DEFAULT ? POLYGONAL_COLOR : DEFAULT_COLOR);
}

Type Linestrip::type() const {
    return VIRTUAL;
}

DirectX::BoundingOrientedBox Linestrip::boundingBox() const {
    return {{},
            {},
            {0, 0, 0, 1.f}};
}
