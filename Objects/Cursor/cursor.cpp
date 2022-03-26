//
// Created by Molom on 2022-03-18.
//

#include <DirectXMath.h>
#include "cursor.h"

using namespace DirectX;

Cursor::Cursor(XMFLOAT3 position, XMINT2 screenPosition, Camera &camera)
        : Object("Cursor", position),
          _screenPosition(screenPosition),
          camera(camera) {
    positionHandler = this->bindablePosition().addNotifier([&] { updateScreenPosition(); });
    projectionHandler = camera.bindableProjection().addNotifier([&] { updateScreenPosition(); });
    viewHandler = camera.bindableView().addNotifier([&] { updateScreenPosition(); });
}

void Cursor::draw(Renderer &renderer, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, DrawType drawType) {
    auto mvp = modelMatrix() * view * projection;
    renderer.drawCursor(mvp);
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
    positionHandler = this->bindablePosition().addNotifier([&] { updateScreenPosition(); });
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
    return {{}, {}, {0, 0, 0, 1.f}};
}
