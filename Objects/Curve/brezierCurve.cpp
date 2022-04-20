//
// Created by Molom on 31/03/2022.
//

#include "brezierCurve.h"

#include <utility>

using namespace std;
using namespace DirectX;

BrezierCurve::BrezierCurve(uint id, QString name, vector<std::weak_ptr<Point>> &&points)
        : Curve(id, std::move(name), std::move(points),
                D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST) {

}

void BrezierCurve::drawPolygonal(Renderer &renderer, DrawType drawType) {
    if (vertices.empty()) return;

    setTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
    switchIndexed(false);
    renderer.draw(*((Object *) this), drawType != DEFAULT ? POLYGONAL_COLOR : DEFAULT_COLOR);
    setTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
    switchIndexed(true);
}

void BrezierCurve::drawCurve(Renderer &renderer, DrawType drawType) {
    if (!canDraw) return;
    renderer.draw(*this, drawType != DEFAULT ? SELECTED_COLOR : DEFAULT_COLOR);
}

XMFLOAT3 BrezierCurve::newMax(XMFLOAT3 oldMax, XMFLOAT3 candidate) {
    return {max(oldMax.x, candidate.x), max(oldMax.y, candidate.y), max(oldMax.z, candidate.z)};
}

XMFLOAT3 BrezierCurve::newMin(XMFLOAT3 oldMin, XMFLOAT3 candidate) {
    return {min(oldMin.x, candidate.x), min(oldMin.y, candidate.y), min(oldMin.z, candidate.z)};
}

void BrezierCurve::pointUpdate(const shared_ptr<Point> &point, int index) {
    if (indices.size() > 0 && indices.size() % 4 == 0) {
        indices.push_back(indices.back());
    }

    vertices.push_back({point->position(), {1, 1, 1}});
    indices.push_back(index);

    minPos = newMin(minPos, point->position());
    maxPos = newMax(maxPos, point->position());
}

void BrezierCurve::postUpdate() {
    if (vertices.size() > 1) {
        _lastPatchSize = indices.size() % 4;
        if (_lastPatchSize == 0) _lastPatchSize = 4;

        while (indices.size() % 4 != 0) indices.push_back(vertices.size() - 1);
    }

    canDraw = vertices.size() >= 2;
    updateBuffers();
}

void BrezierCurve::pointMoved(const weak_ptr<Point> &point) {
    if (_points.size() < 2) return;

    shared_ptr<Point> moved;
    if (!(moved = point.lock())) {
        updatePoints();
        return;
    }

    auto it = std::find_if(_points.begin(), _points.end(), [&moved](const weak_ptr<Point> &p) {
        return moved.get() == p.lock().get();
    });

    if (it == _points.end()) {
        updatePoints();
        return;
    }

    int index = it - _points.begin();
    vertices[index].position = moved->position();
    updateBuffers();
}