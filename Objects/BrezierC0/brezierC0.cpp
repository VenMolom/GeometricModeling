//
// Created by Molom on 2022-03-25.
//

#include <DirectXMath.h>

#include <utility>
#include "brezierC0.h"

using namespace std;
using namespace DirectX;

BrezierC0::BrezierC0(vector<weak_ptr<Point>> points)
        : Object("Brezier C0", {0, 0, 0}),
          _points(std::move(points)) {
}

void BrezierC0::addPoint(weak_ptr<Point> point) {
    if (std::find_if(_points.begin(), _points.end(), [&](const weak_ptr<Point> &p) {
        return point.lock().get() == p.lock().get();
    }) != _points.end()) {
        return;
    }

    _points.push_back(point);
    pointsChanged.setValue(pointsChanged.value() + 1);
}

void BrezierC0::movePoint(int index, Direction direction) {
    auto move = direction == Direction::UP ? -1 : 1;

    if (index + move >= _points.size()) return;

    swap(_points[index], _points[index + move]);
    pointsChanged.setValue(pointsChanged.value() + 1);
}

void BrezierC0::removePoint(int index) {
    _points.erase(next(_points.begin(), index));
    pointsChanged.setValue(pointsChanged.value() + 1);
}

void BrezierC0::draw(Renderer &renderer, XMMATRIX view, XMMATRIX projection, DrawType drawType) {
    if (_points.size() < 2) return;

    vector<VertexPositionColor> vertices{};
    vector<Index> indices{};
    std::shared_ptr<Point> point;
    XMFLOAT3 min{INFINITY, INFINITY, INFINITY}, max{-INFINITY, -INFINITY, -INFINITY};

    auto mvp = modelMatrix() * view * projection;
    for (int i = 0; i < _points.size(); ++i) {
        if (!(point = _points[i].lock())) {
            removePoint(i);
            --i;
            continue;
        }

        if (indices.size() > 0 && indices.size() % 4 == 0) {
            indices.push_back(i - 1);
        }

        vertices.push_back({point->position(), {1, 1, 1}});
        indices.push_back(i);

        min = newMin(min, point->position());
        max = newMax(max, point->position());
    }

    if (vertices.size() > 1) {
        int lastPatchSize = indices.size() % 4;
        if (lastPatchSize == 0) lastPatchSize = 4;

        while (indices.size() % 4 != 0) indices.push_back(vertices.size() - 1);

        renderer.drawCurve4(vertices, indices, lastPatchSize, XMLoadFloat3(&min), XMLoadFloat3(&max), mvp,
                            drawType != DEFAULT);
    }

    if (polygonal && drawType != DEFAULT) {
        renderer.drawLineStrip(vertices, mvp, true);
    }
}

Type BrezierC0::type() const {
    return BREZIERC0;
}

BoundingOrientedBox BrezierC0::boundingBox() const {
    return {{},
            {},
            {0, 0, 0, 1.f}};
}

XMFLOAT3 BrezierC0::newMax(XMFLOAT3 oldMax, XMFLOAT3 candidate) {
    return {max(oldMax.x, candidate.x), max(oldMax.y, candidate.y), max(oldMax.z, candidate.z)};
}

XMFLOAT3 BrezierC0::newMin(XMFLOAT3 oldMin, XMFLOAT3 candidate) {
    return {min(oldMin.x, candidate.x), min(oldMin.y, candidate.y), min(oldMin.z, candidate.z)};
}
