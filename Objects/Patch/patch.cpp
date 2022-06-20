//
// Created by Molom on 2022-05-13.
//

#include "patch.h"

#include <utility>
#include "Objects/Point/composableVirtualPoint.h"

using namespace std;
using namespace DirectX;

Patch::Patch(uint id, QString name, XMFLOAT3 position, array<int, PATCH_DIM> density,
             array<int, PATCH_DIM> segments, bool cylinder, QBindable<weak_ptr<Object>> bindableSelected)
        : Patch(id, std::move(name), position, density, segments, cylinder, bindableSelected,
                D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST) {
}

Patch::Patch(uint id, QString name, XMFLOAT3 position, array<int, PATCH_DIM> density,
             array<int, PATCH_DIM> segments, bool cylinder, QBindable<weak_ptr<Object>> bindableSelected,
             D3D11_PRIMITIVE_TOPOLOGY topology)
        : ParametricObject<PATCH_DIM>(id, std::move(name), position, density,
                                      {make_tuple(0, 1.f * segments[0]), make_tuple(0, 1.f * segments[1])},
                                      topology),
          VirtualPointsHolder(bindableSelected),
          segments(segments),
          loopedU(cylinder),
          startingPosition(position) {
    XMStoreFloat4x4(&modificationMatrixInverse, XMMatrixIdentity());
}

void Patch::draw(Renderer &renderer, DrawType drawType) {
    drawMesh(renderer, drawType);

    drawPoints(renderer, drawType);

    if (_polygonal && !bezierMesh.vertices().empty()) {
        bezierMesh.draw(renderer, drawType);
    }

    drawCursor(renderer, drawType);
}

void Patch::drawMesh(Renderer &renderer, DrawType drawType) {
    renderer.draw(*this, drawType != DEFAULT ? SELECTED_COLOR : DEFAULT_COLOR);
}

void Patch::drawPoints(Renderer &renderer, DrawType drawType) {
    if (!points.empty()) {
        for (auto &point: points) {
            auto isSelected = !selected.expired() && point->equals(selected.lock());
            point->draw(renderer, isSelected ? SELECTED : DEFAULT);
        }
    }
}

void Patch::drawCursor(Renderer &renderer, DrawType drawType) {
    if (drawType == SELECTED) {
        Cursor::drawCursor(renderer, position(), rotation());
    }
}

std::array<bool, PATCH_DIM> Patch::looped() const {
    return {loopedU, loopedV};
}

const vector<std::shared_ptr<VirtualPoint>> &Patch::virtualPoints() {
    return points;
}

void Patch::createSegments(array<int, PATCH_DIM> segments, array<float, PATCH_DIM> size) {
    clear();
    if (loopedU) {
        createCylinderSegments(segments, size);
    } else {
        createPlaneSegments(segments, size);
    }
    calculateMeshIndices(segments, bezierMesh);
    updatePoints();
}

void Patch::pointMoved(const weak_ptr<VirtualPoint> &point, int index) {
    if (ignoreSignal) return;

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

void Patch::addPoint(XMFLOAT3 position) {
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

void Patch::replacePoint(shared_ptr<VirtualPoint> point, shared_ptr<VirtualPoint> newPoint) {
    auto pos = std::find_if(points.begin(), points.end(),
                            [&point](const shared_ptr<VirtualPoint> &p) {
                                return point->equals(p);
                            });

    if (pos == points.end()) {
        return;
    }
    int index = pos - points.begin();

    weak_ptr<VirtualPoint> weakPoint = newPoint;
    pointsHandlers[index] = newPoint->bindablePosition().addNotifier([this, weakPoint, index] {
        pointMoved(weakPoint, index);
    });
    points[index] = newPoint;

    auto newPosition = newPoint->position();
    startingPositions[index] = newPosition;
    vertices[index].position = newPosition;
    bezierMesh.vertices()[index].position = newPosition;
    updateBuffers();
    bezierMesh.update();
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
    ignoreSignal = true;
    auto rotation = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&_rotation.value()));
    auto scaling = XMMatrixScalingFromVector(XMLoadFloat3(&_scale.value()));

    auto translationBack = XMMatrixTranslationFromVector(XMLoadFloat3(&_position.value()));
    auto translationToCenter = XMMatrixTranslationFromVector(XMVectorNegate(XMLoadFloat3(&startingPosition)));
    auto modifyMatrix = translationToCenter * scaling * rotation * translationBack;
    XMStoreFloat4x4(&modificationMatrixInverse, XMMatrixInverse(nullptr, modifyMatrix));

    int i = 0;
    for (auto &point: points) {
        auto pos = startingPositions[i];
        XMStoreFloat3(&pos, XMVector3TransformCoord(XMLoadFloat3(&pos), modifyMatrix));
        point->setPosition(pos);
        vertices[i].position = pos;
        bezierMesh.vertices()[i++].position = pos;
    }

    updateBuffers();
    bezierMesh.update();
    ignoreSignal = false;
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

void Patch::calculateCenter() {
    XMFLOAT3 center{};
    XMVECTOR c = XMLoadFloat3(&center);
    for (auto &point: points) {
        auto oc = point->position();
        c = XMVectorAdd(c, XMLoadFloat3(&oc));
    }
    XMStoreFloat3(&center, XMVectorScale(c, 1.0f / static_cast<float>(points.size())));
    startingPosition = center;
    Object::setPosition(center);
}

void Patch::deserializePatch(const MG1::BezierPatch &patch, const std::map<uint, int> &pointMap) {
    // vRow
    for (int i = 0; i < 4; ++i) {
        // uRow
        for (int j = 0; j < 4; ++j) {
            auto index = i * 4 + j;
            auto pointRef = patch.controlPoints[index];
            indices.push_back(pointMap.at(pointRef.GetId()));
        }
    }
}

Patch::Patch(const MG1::BezierSurfaceC0 &surface, vector<MG1::Point> &serializedPoints,
             QBindable<weak_ptr<Object>> bindableSelected)
        : ParametricObject<PATCH_DIM>(surface.GetId(), QString::fromStdString(surface.name), {0, 0, 0},
                                      {static_cast<int>(surface.patches[0].samples.x),
                                       static_cast<int>(surface.patches[0].samples.y)},
                                      {make_tuple(0, 1.f * surface.size.x), make_tuple(0, 1.f * surface.size.x)},
                                      D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST),
          VirtualPointsHolder(bindableSelected),
          segments({static_cast<int>(surface.size.x), static_cast<int>(surface.size.y)}),
          loopedU(surface.uWrapped),
          loopedV(surface.vWrapped) {
    deserializePatches(surface.patches, serializedPoints);
    calculateCenter();
    XMStoreFloat4x4(&modificationMatrixInverse, XMMatrixIdentity());
}

Patch::Patch(const MG1::BezierSurfaceC2 &surface, vector<MG1::Point> &serializedPoints,
             QBindable<weak_ptr<Object>> bindableSelected)
        : ParametricObject<PATCH_DIM>(surface.GetId(), QString::fromStdString(surface.name), {0, 0, 0},
                                      {static_cast<int>(surface.patches[0].samples.x),
                                       static_cast<int>(surface.patches[0].samples.y)},
                                      {make_tuple(0, 1.f * surface.size.x), make_tuple(0, 1.f * surface.size.y)},
                                      D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST),
          VirtualPointsHolder(bindableSelected),
          segments({static_cast<int>(surface.size.x), static_cast<int>(surface.size.y)}),
          loopedU(surface.uWrapped),
          loopedV(surface.vWrapped) {
    deserializePatches(surface.patches, serializedPoints);
    calculateCenter();
    XMStoreFloat4x4(&modificationMatrixInverse, XMMatrixIdentity());
}

std::array<int, PATCH_DIM> Patch::size() const {
    return segments;
}

std::array<DirectX::XMFLOAT3, 16> Patch::getControlPoints(float &u, float &v) {
    int segmentU = std::clamp(static_cast<int>(u), 0, segments[0] - 1);
    int segmentV = std::clamp(static_cast<int>(v), 0, segments[1] - 1);

    int startIndex = segmentU * segments[1] * 16 + segmentV * 16;

    array<XMFLOAT3, 16> control{
            vertices[indices[startIndex]].position,
            vertices[indices[startIndex + 1]].position,
            vertices[indices[startIndex + 2]].position,
            vertices[indices[startIndex + 3]].position,

            vertices[indices[startIndex + 4]].position,
            vertices[indices[startIndex + 5]].position,
            vertices[indices[startIndex + 6]].position,
            vertices[indices[startIndex + 7]].position,

            vertices[indices[startIndex + 8]].position,
            vertices[indices[startIndex + 9]].position,
            vertices[indices[startIndex + 10]].position,
            vertices[indices[startIndex + 11]].position,

            vertices[indices[startIndex + 12]].position,
            vertices[indices[startIndex + 13]].position,
            vertices[indices[startIndex + 14]].position,
            vertices[indices[startIndex + 15]].position,
    };

    u -= segmentU;
    v -= segmentV;

    return control;
}
