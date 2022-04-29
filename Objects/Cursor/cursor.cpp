//
// Created by Molom on 2022-03-18.
//

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

std::unique_ptr<Cursor> Cursor::instance = {};

void Cursor::drawCursor(Renderer &renderer, const XMFLOAT3 &position, const XMFLOAT3 &rotation) {
    if (!instance) {
        instance = std::make_unique<Cursor>(XMFLOAT3(0, 0, 0), XMINT2(0, 0), std::shared_ptr<Camera>());
    }

    instance->setPosition(position);
    instance->setRotation(rotation);
    renderer.draw(*instance, DEFAULT_COLOR);
}

Cursor::Cursor(XMFLOAT3 position, XMINT2 screenPosition, std::shared_ptr<Camera> camera)
        : Object(0, "Cursor", position, D3D11_PRIMITIVE_TOPOLOGY_LINELIST),
          _screenPosition(screenPosition),
          camera(std::move(camera)) {
    if (camera) {
        positionHandler = this->bindablePosition().addNotifier([this] { updateScreenPosition(); });
        projectionHandler = camera->bindableProjection().addNotifier([this] { updateScreenPosition(); });
        viewHandler = camera->bindableView().addNotifier([this] { updateScreenPosition(); });
    }

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

    auto ray = Utils3D::getRayFromScreen(screenPosition(), camera);

    auto plane = Utils3D::getPerpendicularPlaneThroughPoint(camera->direction(), camera->center());

    // xD
    positionHandler = {};
    setPosition(Utils3D::getRayCrossWithPlane(ray, plane));
    positionHandler = this->bindablePosition().addNotifier([this] { updateScreenPosition(); });
}

void Cursor::updateScreenPosition() {
    auto pos = position();
    auto viewport = camera->viewport();
    auto screen = XMVector3Project(XMLoadFloat3(&pos), 0, 0,
                                   viewport.width(), viewport.height(), 0.0f, 1.0f,
                                   camera->projectionMatrix(), camera->viewMatrix(), XMMatrixIdentity());
    XMFLOAT2 screenPos{};
    XMStoreFloat2(&screenPos, screen);
    _screenPosition = {static_cast<int32_t>(screenPos.x), static_cast<int32_t>(screenPos.y)};
}