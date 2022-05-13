//
// Created by Molom on 2022-05-13.
//

#include "patch.h"

using namespace std;
using namespace DirectX;

Patch::Patch(uint id, QString name, XMFLOAT3 position, array<int, PATCH_DIM> density,
             bool cylinder, QBindable<weak_ptr<Object>> bindableSelected)
        : ParametricObject<PATCH_DIM>(id, name, position, density,
                                      {make_tuple(0, 1.f), make_tuple(0, 1.f)},
                                      D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST),
          VirtualPointsHolder(bindableSelected),
          cylinder(cylinder) {
}

void Patch::draw(Renderer &renderer, DrawType drawType) {
    renderer.draw(*this, drawType != DEFAULT ? SELECTED_COLOR : DEFAULT_COLOR);
    // TODO: draw polygonal
    if (!points.empty()) {
        for (auto &point: points) {
            auto isSelected = !selected.expired() && point->equals(selected.lock());
            point->draw(renderer, isSelected ? SELECTED : DEFAULT);
        }
    }
}

std::array<bool, PATCH_DIM> Patch::looped() const {
    return {cylinder, false};
}

const vector<std::shared_ptr<VirtualPoint>> &Patch::virtualPoints() {
    return points;
}

void Patch::pointMoved(const weak_ptr<VirtualPoint> &point, int index) {
    shared_ptr<VirtualPoint> moved;
    if (!(moved = point.lock())) {
        return;
    }

    vertices[index].position = moved->position();
    updateBuffers();
}

void Patch::addPoint(DirectX::XMFLOAT3 position) {
    auto point = make_shared<VirtualPoint>(position);
    weak_ptr<VirtualPoint> weakPoint = point;
    auto index = static_cast<int>(points.size());
    pointsHandlers.push_back(point->bindablePosition().addNotifier([this, weakPoint, index] {
        pointMoved(weakPoint, index);
    }));
    points.push_back(std::move(point));

    vertices.push_back({position, {1, 1, 1}});
}
