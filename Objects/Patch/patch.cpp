//
// Created by Molom on 2022-05-13.
//

#include "patch.h"
#include "Objects/Point/composableVirtualPoint.h"

using namespace std;
using namespace DirectX;

Patch::Patch(uint id, QString name, XMFLOAT3 position, array<int, PATCH_DIM> density,
             bool cylinder, QBindable<weak_ptr<Object>> bindableSelected)
        : ParametricObject<PATCH_DIM>(id, name, position, density,
                                      {make_tuple(0, 1.f), make_tuple(0, 1.f)},
                                      D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST),
          VirtualPointsHolder(bindableSelected),
          cylinder(cylinder),
          startingPosition(position) {
    XMStoreFloat4x4(&modificationMatrixInverse, XMMatrixIdentity());
}

void Patch::draw(Renderer &renderer, DrawType drawType) {
    renderer.draw(*this, drawType != DEFAULT ? SELECTED_COLOR : DEFAULT_COLOR);

    if (!points.empty()) {
        for (auto &point: points) {
            auto isSelected = !selected.expired() && point->equals(selected.lock());
            point->draw(renderer, isSelected ? SELECTED : DEFAULT);
        }
    }

    if (_polygonal && !bezierMesh.vertices().empty()) {
        bezierMesh.draw(renderer, drawType);
    }

    if (drawType == SELECTED) {
        Cursor::drawCursor(renderer, position(), rotation());
    }
}

std::array<bool, PATCH_DIM> Patch::looped() const {
    return {false, cylinder};
}

const vector<std::shared_ptr<VirtualPoint>> &Patch::virtualPoints() {
    return points;
}

void Patch::pointMoved(const weak_ptr<VirtualPoint> &point, int index) {
    shared_ptr<VirtualPoint> moved;
    if (!(moved = point.lock())) {
        return;
    }

    auto newPos = moved->position();
    XMStoreFloat3(&startingPositions[index], XMVector3TransformCoord(XMLoadFloat3(&newPos),
                                                                     XMLoadFloat4x4(&modificationMatrixInverse)));

    vertices[index].position = newPos;
    bezierMesh.vertices()[index].position = newPos;

    updateBuffers();
    bezierMesh.update();
}

void Patch::addPoint(DirectX::XMFLOAT3 position) {
    auto point = make_shared<ComposableVirtualPoint>(position);
    weak_ptr<VirtualPoint> weakPoint = point;
    auto index = static_cast<int>(points.size());
    pointsHandlers.push_back(point->bindablePosition().addNotifier([this, weakPoint, index] {
        pointMoved(weakPoint, index);
    }));
    points.push_back(std::move(point));
    startingPositions.push_back(position);

    vertices.push_back({position, {1, 1, 1}});
    bezierMesh.vertices().push_back({position, {1, 1, 1}});
}

void Patch::setPosition(DirectX::XMFLOAT3 position) {
    Object::setPosition(position);
    updatePoints();
}

void Patch::setRotation(DirectX::XMFLOAT3 rotation) {
    Object::setRotation(rotation);
    updatePoints();
}

void Patch::setScale(DirectX::XMFLOAT3 scale) {
    Object::setScale(scale);
    updatePoints();
}

void Patch::updatePoints() {
    auto rotation = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&_rotation.value()));
    auto scaling = XMMatrixScalingFromVector(XMLoadFloat3(&_scale.value()));

    auto translationBack = XMMatrixTranslationFromVector(XMLoadFloat3(&_position.value()));
    auto translationToCenter = XMMatrixTranslationFromVector(XMVectorNegate(XMLoadFloat3(&startingPosition)));
    auto modifyMatrix = translationToCenter * scaling * rotation * translationBack;
    XMStoreFloat4x4(&modificationMatrixInverse, XMMatrixInverse(nullptr, modifyMatrix));

    int i = 0;
    for (auto &point : points) {
        auto pos = startingPositions[i];
        XMStoreFloat3(&pos, XMVector3TransformCoord(XMLoadFloat3(&pos), modifyMatrix));
        point->setPositionSilently(pos);
        vertices[i].position = pos;
        bezierMesh.vertices()[i++].position = pos;
    }

    updateBuffers();
    bezierMesh.update();
}

void Patch::clear() {
    vertices.clear();
    indices.clear();
    points.clear();
    startingPositions.clear();
    pointsHandlers.clear();
    startingPosition = _position.value();
    bezierMesh.vertices().clear();
    bezierMesh.indices().clear();
}


