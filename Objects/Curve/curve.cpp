//
// Created by Molom on 2022-04-15.
//

#include "curve.h"

#include <utility>

using namespace std;
using namespace DirectX;

Curve::Curve(uint id, QString name, vector<std::weak_ptr<Point>> &&points, D3D11_PRIMITIVE_TOPOLOGY topology)
        : Object(id, std::move(name), {0, 0, 0}, topology),
          _points(points) {

}

void Curve::addPoint(weak_ptr<Point> point) {
    _points.push_back(point);
    updatePoints();
}

void Curve::movePoint(int index, Direction direction) {
    auto move = direction == Direction::UP ? -1 : 1;

    if (index + move >= _points.size()) return;

    swap(_points[index], _points[index + move]);
    updatePoints();
}

void Curve::removePoint(int index) {
    _points.erase(next(_points.begin(), index));
    updatePoints();
}

void Curve::draw(Renderer &renderer, DrawType drawType) {
    for (auto &point: _points) {
        if (!point.lock()) {
            updatePoints();
            break;
        }
    }

    if (_polygonal) {
        drawPolygonal(renderer, drawType);
    }

    drawCurve(renderer, drawType);
}

void Curve::updatePoints() {
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

void Curve::preUpdate() {
    vertices.clear();
    indices.clear();
    pointsHandlers.clear();
}

void Curve::postUpdate() {
    updateBuffers();
}

Curve::Curve(const MG1::Bezier &curve, const list<shared_ptr<Object>> &sceneObjects,
             D3D11_PRIMITIVE_TOPOLOGY topology)
        : Curve(curve.GetId(), QString::fromStdString(curve.name), {}, topology) {
    vector<weak_ptr<Point>> points{};

    for (auto &point: curve.controlPoints) {
        auto it = find_if(sceneObjects.begin(), sceneObjects.end(),
                          [&point](const shared_ptr<Object> &ob) {
                              return ob->type() == POINT3D && ob->id() == point.GetId();
                          });

        if (it != sceneObjects.end()) {
            weak_ptr<Point> p = dynamic_pointer_cast<Point>(*it);
            if (p.lock()) {
                points.push_back(p);
            }
        }
    }

    _points = std::move(points);
}