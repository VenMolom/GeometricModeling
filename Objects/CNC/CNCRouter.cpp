//
// Created by Molom on 14/10/2022.
//

#include "CNCRouter.h"
#include "Utils/utils3D.h"

using namespace std;
using namespace DirectX;

const XMFLOAT3 CNCRouter::NEUTRAL_TOOL_POSITION = {0.f, 0.f, 10.f};

CNCRouter::CNCRouter(uint id, XMFLOAT3 position)
        : Object(id, "Router", position, D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST),
          tool(NEUTRAL_TOOL_POSITION),
          drawPaths() {
    drawPaths.setPosition(position);
    drawPaths.setRotation(XMFLOAT3(-XM_PIDIV2, 0.f, 0.f));
    Object::setPosition(position);
    Object::setRotation(XMFLOAT3(-XM_PIDIV2, 0.f, 0.f));
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
    tool.setType(type);
}

void CNCRouter::setToolSize(int size) {
    tool.setSize(size);
}

void CNCRouter::setShowPaths(bool show) {
    _showPaths = show;
}

void CNCRouter::setSimulationSpeed(int speed) {
    _simulationSpeed = speed;
}

void CNCRouter::draw(Renderer &renderer, DrawType drawType) {
    // TODO: draw block
    tool.draw(renderer, modelMatrix());

    if (_showPaths) {
        drawPaths.draw(renderer, drawType);
    }
}

void CNCRouter::update(Renderer &renderer, float frameTime) {
    if (_state != RouterState::Started && _state != RouterState::Skipped) return;

    if (drawPaths.vertices().size() < 2) {
        _state = RouterState::Finished;
        tool.setPosition(NEUTRAL_TOOL_POSITION);
        drawPaths.update();
        return;
    }

    auto currentPosition = tool.position();
    if (_state == RouterState::Started) {

        auto distanceToTravel = _simulationSpeed * TOOL_SPEED * frameTime;

        while (distanceToTravel > 0.f && drawPaths.vertices().size() >= 2) {
            auto nextTarget = (drawPaths.vertices().end() - 2)->position;
            auto distanceToTarget = XMVector3Length(
                    XMVectorSubtract(XMLoadFloat3(&nextTarget), XMLoadFloat3(&currentPosition))).m128_f32[0];
            if (distanceToTravel < distanceToTarget) {
                Utils3D::storeFloat3Lerp(drawPaths.vertices().back().position, currentPosition, nextTarget,
                                         distanceToTravel / distanceToTarget);
                break;
            }

            distanceToTravel -= distanceToTarget;
            drawPaths.vertices().pop_back();
            currentPosition = nextTarget;

            // TODO: update texture from path
        }

        if (!drawPaths.vertices().empty()) {
            tool.setPosition(drawPaths.vertices().back().position);
        }
        drawPaths.update();
    } else {
        drawPaths.vertices().pop_back();

        // TODO: update texture from path
    }
    _progress = 100 - static_cast<int>(std::floor(static_cast<float>(drawPaths.vertices().size() * 100) / static_cast<float>(routerPath.moves.size())));
}

void CNCRouter::loadPath(CNCPath &&path) {
    _filename = QString::fromStdString(path.filename);
    routerPath = path;

    tool.setSize(path.size);
    tool.setType(path.type);

    fillDrawPaths();
    _state = fresh ? RouterState::FirstPathLoaded : RouterState::NextPathLoaded;
    _progress = 0;
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
    _state = RouterState::Started;
    tool.setPosition(drawPaths.vertices().back().position);
}

void CNCRouter::skip() {
    fresh = false;
    _state = RouterState::Skipped;
}

void CNCRouter::reset() {
    fresh = true;
    _state = routerPath.moves.empty() ? RouterState::Created : RouterState::FirstPathLoaded;
    _progress = 0;
    if (!routerPath.moves.empty()) {
        fillDrawPaths();
    }
    // TODO: implement
}
