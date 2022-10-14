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

void CNCRouter::setPosition(DirectX::XMFLOAT3 position) {
    drawPaths.setPosition(position);
    Object::setPosition(position);
}

void CNCRouter::setSize(DirectX::XMFLOAT3 size) {
    _size = size;
    // TODO: update some matrix idk
}

void CNCRouter::setPointsDensity(std::pair<int, int> density) {
    _pointsDensity = density;
    // TODO: update texture
}

void CNCRouter::setMaxDepth(float depth) {
    _maxDepth = depth;
}

void CNCRouter::setToolType(CNCType type) {
    _toolType = type;
}

void CNCRouter::setToolSize(int size) {
    _toolSize = size;
}

void CNCRouter::setShowPaths(bool show) {
    _showPaths = show;
}

void CNCRouter::setSimulationSpeed(int speed) {
    _simulationSpeed = speed;
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
    _state = fresh ? RouterState::FirstPathLoaded : RouterState::NextPathLoaded;
    routerPath = path;
    fillDrawPaths();

    // TODO: implement
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

void CNCRouter::start() {
    fresh = false;
    // TODO: implement
}

void CNCRouter::skip() {
    // TODO: implement
}

void CNCRouter::reset() {
    fresh = true;
    _state = RouterState::FirstPathLoaded;
    // TODO: implement
}
