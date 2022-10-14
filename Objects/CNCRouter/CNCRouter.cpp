//
// Created by Molom on 14/10/2022.
//

#include "CNCRouter.h"

using namespace std;
using namespace DirectX;

CNCRouter::CNCRouter(uint id, XMFLOAT3 position)
        : Object(id, "Router", position, D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST),
          drawPaths() {
    drawPaths.setPosition(position);
    drawPaths.setRotation(XMFLOAT3(-XM_PIDIV2, 0.f, 0.f));
}

Type CNCRouter::type() const {
    return CNCROUTER;
}

void CNCRouter::draw(Renderer &renderer, DrawType drawType) {
    // TODO: draw block
    // TODO: draw paths if selected
    drawPaths.draw(renderer, drawType);
}

void CNCRouter::update(float frameTime) {
    // TODO: update texture from path
}

void CNCRouter::loadPath(CNCPath &&path) {
    routerPath = path;
    fillDrawPaths();
}

void CNCRouter::fillDrawPaths() {
    auto size = routerPath.moves.size();
    drawPaths.vertices().clear();
    drawPaths.vertices().resize(size);

    // lines are stored in reverse to optimise their deletion when traversing
    for (auto i = 0; i < size; ++i) {
        drawPaths.vertices()[i].position = routerPath.moves[size - i - 1].target;
    }
    drawPaths.update();
}

void CNCRouter::setPosition(DirectX::XMFLOAT3 position) {
    drawPaths.setPosition(position);
    Object::setPosition(position);
}
