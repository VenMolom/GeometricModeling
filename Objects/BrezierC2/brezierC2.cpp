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

void BrezierC2::pointMoved(const weak_ptr<Point> &point) {
    if (_points.size() < 4) return;

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

    bSplineVertices[index].position = moved->position();

    XMFLOAT3 pointPosition = moved->position();
    int under = (index - 1) * 3;

    // TODO: there are overlaps in calculated points, should use already calculated points where possible

    // fix under left neighbour
    if (index > 1) {
        shared_ptr<Point> left = _points[index - 1].lock();
        shared_ptr<Point> leftLeft = _points[index - 2].lock();
        if (!left || !leftLeft) {
            updatePoints();
            return;
        }

        XMFLOAT3 leftPosition = left->position();
        XMFLOAT3 leftLeftPosition = leftLeft->position();
        XMFLOAT3 b1{}, b2{};
        XMStoreFloat3(&b1, XMVectorLerp(
                XMLoadFloat3(&leftPosition),
                XMLoadFloat3(&pointPosition),
                1.0f / 3.0f)
        );
        XMStoreFloat3(&b2, XMVectorLerp(
                XMLoadFloat3(&leftLeftPosition),
                XMLoadFloat3(&leftPosition),
                2.0f / 3.0f)
        );
        XMStoreFloat3(&vertices[under - 3].position, XMVectorLerp(
                XMLoadFloat3(&b1),
                XMLoadFloat3(&b2),
                0.5f)
        );
    }

    // fix between self and left neighbour
    if (index > 1 && index < _points.size() - 1) {
        shared_ptr<Point> left = _points[index - 1].lock();
        if (!left) {
            updatePoints();
            return;
        }

        XMFLOAT3 leftPosition = left->position();
        XMStoreFloat3(&vertices[under - 2].position, XMVectorLerp(
                XMLoadFloat3(&leftPosition),
                XMLoadFloat3(&pointPosition),
                1.0f / 3.0f)
        );
        XMStoreFloat3(&vertices[under - 1].position, XMVectorLerp(
                XMLoadFloat3(&leftPosition),
                XMLoadFloat3(&pointPosition),
                2.0f / 3.0f)
        );
    }

    // fix under yourself
    if (index > 0 && index < _points.size() - 1) {
        shared_ptr<Point> left = _points[index - 1].lock();
        shared_ptr<Point> right = _points[index + 1].lock();
        if (!left || !right) {
            updatePoints();
            return;
        }

        XMFLOAT3 leftPosition = left->position();
        XMFLOAT3 rightPosition = right->position();
        XMFLOAT3 b1{}, b2{};
        XMStoreFloat3(&b1, XMVectorLerp(
                XMLoadFloat3(&leftPosition),
                XMLoadFloat3(&pointPosition),
                2.0f / 3.0f)
        );
        XMStoreFloat3(&b2, XMVectorLerp(
                XMLoadFloat3(&pointPosition),
                XMLoadFloat3(&rightPosition),
                1.0f / 3.0f)
        );
        XMStoreFloat3(&vertices[under].position, XMVectorLerp(
                XMLoadFloat3(&b1),
                XMLoadFloat3(&b2),
                0.5f)
        );
    }

    // fix between self and right neighbour
    if (index > 0 && index < _points.size() - 2) {
        shared_ptr<Point> right = _points[index + 1].lock();
        if (!right) {
            updatePoints();
            return;
        }

        XMFLOAT3 rightPosition = right->position();
        XMStoreFloat3(&vertices[under + 1].position, XMVectorLerp(
                XMLoadFloat3(&pointPosition),
                XMLoadFloat3(&rightPosition),
                1.0f / 3.0f)
        );
        XMStoreFloat3(&vertices[under + 2].position, XMVectorLerp(
                XMLoadFloat3(&pointPosition),
                XMLoadFloat3(&rightPosition),
                2.0f / 3.0f)
        );
    }

    // fix under right neighbour
    if (index < _points.size() - 2) {
        shared_ptr<Point> right = _points[index + 1].lock();
        shared_ptr<Point> rightRight = _points[index + 2].lock();
        if (!right || !rightRight) {
            updatePoints();
            return;
        }

        XMFLOAT3 rightPosition = right->position();
        XMFLOAT3 rightRightPosition = rightRight->position();
        XMFLOAT3 b1{}, b2{};
        XMStoreFloat3(&b1, XMVectorLerp(
                XMLoadFloat3(&pointPosition),
                XMLoadFloat3(&rightPosition),
                2.0f / 3.0f)
        );
        XMStoreFloat3(&b2, XMVectorLerp(
                XMLoadFloat3(&rightPosition),
                XMLoadFloat3(&rightRightPosition),
                1.0f / 3.0f)
        );
        XMStoreFloat3(&vertices[under + 3].position, XMVectorLerp(
                XMLoadFloat3(&b1),
                XMLoadFloat3(&b2),
                0.5f)
        );
    }
}
