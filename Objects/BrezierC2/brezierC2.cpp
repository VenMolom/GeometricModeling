//
// Created by Molom on 2022-04-01.
//

#include <DirectXMath.h>
#include "brezierC2.h"

using namespace std;
using namespace DirectX;

BrezierC2::BrezierC2(vector<weak_ptr<Point>> &&points)
        : BrezierCurve("Brezier C2", std::move(points)) {
    updatePoints();
}

Type BrezierC2::type() const {
    return BREZIERC2;
}

void BrezierC2::drawPolygonal(Renderer &renderer, DirectX::XMMATRIX mvp, DrawType drawType) {
    if (_bernsteinBase || _bothPolygonals) {
        BrezierCurve::drawPolygonal(renderer, mvp, drawType);
    }
    if (!_bernsteinBase || _bothPolygonals) {
        renderer.drawLineStrip(bSplineVertices, mvp, drawType != DEFAULT);
    }

}

void BrezierC2::pointUpdate(const shared_ptr<Point> &point, int index) {
    pointsHandlers.push_back(point->bindablePosition().addNotifier([this] { updatePoints(); }));

    bSplineVertices.push_back({point->position(), {1, 1, 1}});

    min = newMin(min, point->position());
    max = newMax(max, point->position());

    shared_ptr<Point> right;
    // get next de Boor point
    while (index + 1 < _points.size() && !(right = _points[index + 1].lock())) {
        _points.erase(next(_points.begin(), index + 1));
    }
    if (!right) return;

    XMFLOAT3 pointPosition = point->position();
    XMFLOAT3 rightPosition = right->position();
    XMFLOAT3 b1{}, b2{};
    XMStoreFloat3(&b1, XMVectorLerp(
            XMLoadFloat3(&pointPosition),
            XMLoadFloat3(&rightPosition),
            1.0f / 3.0f)
    );
    XMStoreFloat3(&b2, XMVectorLerp(
            XMLoadFloat3(&pointPosition),
            XMLoadFloat3(&rightPosition),
            2.0f / 3.0f)
    );

    if (index == 0) {
        vertices.push_back({b2, {1, 1, 1}});
        return;
    }

    XMFLOAT3 b0{};
    XMStoreFloat3(&b0, XMVectorLerp(
            XMLoadFloat3(&vertices.back().position),
            XMLoadFloat3(&b1),
            0.5f)
    );
    if (index == 1) {
        vertices[0].position = b0;
    } else {
        vertices.push_back({b0, {1, 1, 1}});
    }
    vertices.push_back({b1, {1, 1, 1}});
    vertices.push_back({b2, {1, 1, 1}});

    auto size = vertices.size();
    indices.push_back(size - 3);
    indices.push_back(size - 2);
    indices.push_back(size - 1);
    indices.push_back(size);
}

void BrezierC2::preUpdate() {
    BrezierCurve::preUpdate();
    bSplineVertices.clear();
}

void BrezierC2::postUpdate() {
    lastPatchSize = 0;
    if (vertices.size() > 2) {
        vertices.resize(vertices.size() - 2);
    }

    canDraw = _points.size() >= 4;
}
