//
// Created by Molom on 2022-03-18.
//

#include <DirectXMath.h>
#include "cursor.h"

using namespace DirectX;

Cursor::Cursor(XMFLOAT3 position, XMFLOAT2 screenPosition)
        : Object(position, {0, 0, 0}),
          _screenPosition(screenPosition) {
    positionHandler = this->bindablePosition().addNotifier([&] { updatePosition(); });
}

void Cursor::draw(Renderer &renderer, const DirectX::XMMATRIX &camera) const {
    auto mvp = modelMatrix() * camera;
    renderer.drawLines(vertices, mvp);
}

Type Cursor::type() const {
    return CURSOR;
}

const std::vector<VertexPositionColor> Cursor::vertices = {
        {{0, 0, 0}, {1, 0, 0}},
        {{1, 0, 0}, {1, 0, 0}},
        {{0, 0, 0}, {0, 1, 0}},
        {{0, 1, 0}, {0, 1, 0}},
        {{0, 0, 0}, {0, 0, 1}},
        {{0, 0, 1}, {0, 0, 1}}
};

void Cursor::setScreenPosition(DirectX::XMFLOAT2 position) {
    _screenPosition = position;

    // TODO: change position based on screen position
}

void Cursor::updatePosition() {
    // TODO: update screen position based on position
}
