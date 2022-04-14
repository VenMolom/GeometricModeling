//
// Created by Molom on 2022-03-18.
//

#include <DirectXMath.h>
#include "cursor.h"

using namespace DirectX;

const std::vector<VertexPositionColor> Cursor::cursorVertices = {
        {{0, 0, 0}, {1, 0, 0}},
        {{1, 0, 0}, {1, 0, 0}},
        {{0, 0, 0}, {0, 1, 0}},
        {{0, 1, 0}, {0, 1, 0}},
        {{0, 0, 0}, {0, 0, 1}},
        {{0, 0, 1}, {0, 0, 1}}
};

void Cursor::drawCursor(Renderer &renderer, const DirectX::XMMATRIX &model) {
    renderer.draw(cursorVertices, model, DEFAULT_COLOR);
}

Cursor::Cursor(XMFLOAT3 position, XMINT2 screenPosition, Camera &camera)
        : Object("Cursor", position, D3D11_PRIMITIVE_TOPOLOGY_LINELIST),
          _screenPosition(screenPosition),
          camera(camera) {
    positionHandler = this->bindablePosition().addNotifier([this] { updateScreenPosition(); });
    projectionHandler = camera.bindableProjection().addNotifier([this] { updateScreenPosition(); });
    viewHandler = camera.bindableView().addNotifier([this] { updateScreenPosition(); });

    setBuffers(cursorVertices, {});
}

void Cursor::draw(Renderer &renderer, DrawType drawType) {
    renderer.draw(*this, DEFAULT_COLOR);
}

Type Cursor::type() const {
    return CURSOR;
}

void Cursor::setScreenPosition(DirectX::XMINT2 position) {
    if (position.x == _screenPosition.value().x &&
        position.y == _screenPosition.value().y) {
        return;
    }

    _screenPosition.setValueBypassingBindings(position);

    auto screenSize = XMFLOAT2(camera.viewport().width(), camera.viewport().height());
    auto ray = Utils3D::getRayFromScreen(screenPosition(), screenSize, camera.nearZ(), camera.farZ(),
                                         camera.projectionMatrix(), camera.viewMatrix());

    auto plane = Utils3D::getPerpendicularPlaneThroughPoint(camera.direction(), camera.center());

    // xD
    positionHandler = {};
    setPosition(Utils3D::getRayCrossWithPlane(ray, plane));
    positionHandler = this->bindablePosition().addNotifier([this] { updateScreenPosition(); });
}

void Cursor::updateScreenPosition() {
    auto pos = position();
    auto viewport = camera.viewport();
    auto screen = XMVector3Project(XMLoadFloat3(&pos), 0, 0,
                                   viewport.width(), viewport.height(), camera.nearZ(), camera.farZ(),
                                   camera.projectionMatrix(), camera.viewMatrix(), XMMatrixIdentity());
    XMFLOAT2 screenPos{};
    XMStoreFloat2(&screenPos, screen);
    _screenPosition = {static_cast<int32_t>(screenPos.x), static_cast<int32_t>(screenPos.y)};
}

BoundingOrientedBox Cursor::boundingBox() const {
    return {{},
            {},
            {0, 0, 0, 1.f}};
}
