//
// Created by Molom on 2022-03-25.
//

#include <DirectXMath.h>
#include "brezierC0.h"

using namespace std;
using namespace DirectX;

BrezierC0::BrezierC0(vector<weak_ptr<Point>> points)
        : Object("Brezier C0", {0, 0, 0}),
          points(points) {
}

void BrezierC0::addPoint(weak_ptr<Point> point) {
    // TODO: check if point already in
    points.push_back(point);
}

void BrezierC0::removePoint(int index) {
    points.erase(next(points.begin(), index));
}

void BrezierC0::draw(Renderer &renderer, XMMATRIX view, XMMATRIX projection, DrawType drawType) {
    if (points.size() < 4) return;

    vector<VertexPositionColor> vertices{};
    std::shared_ptr<Point> point;

    for (int i = 0; i < points.size(); ++i) {
        if (!(point = points[i].lock())) {
            removePoint(i);
            --i;
            continue;
        }
        vertices.push_back({point->position(), {1, 1, 1}});
    }
    // TODO: slip drawing into 4 point parts
    auto mvp = modelMatrix() * view * projection;
    renderer.drawCurve4(vertices, mvp, drawType != DEFAULT);

    if (polygonal && drawType != DEFAULT) {
        renderer.drawLineStrip(vertices, mvp, true);
    }
}

Type BrezierC0::type() const {
    return BREZIERC0;
}

BoundingOrientedBox BrezierC0::boundingBox() const {
    return {};
}
