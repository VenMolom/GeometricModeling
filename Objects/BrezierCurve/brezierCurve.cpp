//
// Created by Molom on 31/03/2022.
//

#include "brezierCurve.h"

#include <utility>

using namespace std;
using namespace DirectX;

BrezierCurve::BrezierCurve(QString name, std::vector<std::weak_ptr<Point>> &&points)
        : Object(std::move(name), {0, 0, 0}),
          _points(points) {

}

void BrezierCurve::addPoint(weak_ptr<Point> point) {
    if (std::find_if(_points.begin(), _points.end(), [&](const weak_ptr<Point> &p) {
        return point.lock().get() == p.lock().get();
    }) != _points.end()) {
        return;
    }

    _points.push_back(point);
    updatePoints();
}

void BrezierCurve::movePoint(int index, Direction direction) {
    auto move = direction == Direction::UP ? -1 : 1;

    if (index + move >= _points.size()) return;

    swap(_points[index], _points[index + move]);
    updatePoints();
}

void BrezierCurve::removePoint(int index) {
    _points.erase(next(_points.begin(), index));
    updatePoints();
}

void BrezierCurve::draw(Renderer &renderer, XMMATRIX view, XMMATRIX projection, DrawType drawType) {
    if (_points.size() < 2) return;

    for (auto &point: _points) {
        if (!point.lock()) {
            updatePoints();
            break;
        }
    }
    auto mvp = modelMatrix() * view * projection;
    renderer.drawCurve4(vertices, indices, lastPatchSize,
                        XMLoadFloat3(&min), XMLoadFloat3(&max), mvp,
                        drawType != DEFAULT);

    if (polygonal) {
        renderer.drawLineStrip(vertices, mvp, drawType != DEFAULT);
    }
}


BoundingOrientedBox BrezierCurve::boundingBox() const {
    return {{}, {}, {0, 0, 0, 1.f}};
}

XMFLOAT3 BrezierCurve::newMax(XMFLOAT3 oldMax, XMFLOAT3 candidate) {
    return {max(oldMax.x, candidate.x), max(oldMax.y, candidate.y), max(oldMax.z, candidate.z)};
}

XMFLOAT3 BrezierCurve::newMin(XMFLOAT3 oldMin, XMFLOAT3 candidate) {
    return {min(oldMin.x, candidate.x), min(oldMin.y, candidate.y), min(oldMin.z, candidate.z)};
}