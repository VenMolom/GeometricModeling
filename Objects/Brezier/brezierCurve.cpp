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
    if (std::find_if(_points.begin(), _points.end(), [&point](const weak_ptr<Point> &p) {
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
    for (auto &point: _points) {
        if (!point.lock()) {
            updatePoints();
            break;
        }
    }
    auto mvp = view * projection;

    if (_polygonal) {
        drawPolygonal(renderer, mvp, drawType);
    }

    if (!canDraw) return;
    renderer.drawCurve4(vertices, indices, lastPatchSize,
                        XMLoadFloat3(&min), XMLoadFloat3(&max), mvp,
                        drawType != DEFAULT ? SELECTED_COLOR : DEFAULT_COLOR);
}

void BrezierCurve::drawPolygonal(Renderer &renderer, XMMATRIX mvp, DrawType drawType) {
    renderer.draw(vertices, LineStrip, mvp, drawType != DEFAULT ? POLYGONAL_COLOR : DEFAULT_COLOR);
}

BoundingOrientedBox BrezierCurve::boundingBox() const {
    return {{},
            {},
            {0, 0, 0, 1.f}};
}

XMFLOAT3 BrezierCurve::newMax(XMFLOAT3 oldMax, XMFLOAT3 candidate) {
    return {max(oldMax.x, candidate.x), max(oldMax.y, candidate.y), max(oldMax.z, candidate.z)};
}

XMFLOAT3 BrezierCurve::newMin(XMFLOAT3 oldMin, XMFLOAT3 candidate) {
    return {min(oldMin.x, candidate.x), min(oldMin.y, candidate.y), min(oldMin.z, candidate.z)};
}

void BrezierCurve::updatePoints() {
    preUpdate();

    shared_ptr<Point> point;
    for (int i = 0; i < _points.size(); ++i) {
        if (!(point = _points[i].lock())) {
            _points.erase(next(_points.begin(), i));
            --i;
            continue;
        }

        weak_ptr<Point> weakPoint = point;
        pointsHandlers.push_back(point->bindablePosition().addNotifier([this, weakPoint] {
            pointMoved(weakPoint);
        }));

        pointUpdate(point, i);
    }

    postUpdate();
    pointsChanged.setValue(pointsChanged.value() + 1);
}

void BrezierCurve::preUpdate() {
    vertices.clear();
    indices.clear();
    pointsHandlers.clear();
}

void BrezierCurve::pointUpdate(const shared_ptr<Point> &point, int index) {
    if (indices.size() > 0 && indices.size() % 4 == 0) {
        indices.push_back(indices.back());
    }

    vertices.push_back({point->position(), {1, 1, 1}});
    indices.push_back(index);

    min = newMin(min, point->position());
    max = newMax(max, point->position());
}

void BrezierCurve::postUpdate() {
    if (vertices.size() > 1) {
        lastPatchSize = indices.size() % 4;
        if (lastPatchSize == 0) lastPatchSize = 4;

        while (indices.size() % 4 != 0) indices.push_back(vertices.size() - 1);
    }

    canDraw = vertices.size() >= 2;
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
}