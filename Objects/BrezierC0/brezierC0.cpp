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
    pointsChanged.setValue(_points.size());
}

void BrezierC0::removePoint(int index) {
    _points.erase(next(_points.begin(), index));
    pointsChanged.setValue(_points.size());
}

void BrezierC0::draw(Renderer &renderer, XMMATRIX view, XMMATRIX projection, DrawType drawType) {
    if (_points.size() < 2) return;

    vector<VertexPositionColor> vertices{};
    vector<VertexPositionColor> segments{};
    std::shared_ptr<Point> point;

    auto mvp = modelMatrix() * view * projection;
    for (int i = 0; i < _points.size(); ++i) {
        if (!(point = _points[i].lock())) {
            removePoint(i);
            --i;
            continue;
        }

        VertexPositionColor vert = {point->position(), {1, 1, 1}};
        vertices.push_back(vert);
        segments.push_back(vert);

        if (vertices.size() == 4) {
            renderer.drawCurve4(vertices, mvp, drawType != DEFAULT);
            vertices.erase(vertices.begin(), next(vertices.begin(), 3));
        }
    }

    if (vertices.size() > 1) {
        renderer.drawCurve4(vertices, mvp, drawType != DEFAULT);
    }

    if (polygonal && drawType != DEFAULT) {
        renderer.drawLineStrip(segments, mvp, true);
    }
}

Type BrezierC0::type() const {
    return BREZIERC0;
}

BoundingOrientedBox BrezierC0::boundingBox() const {
    return {};
}
