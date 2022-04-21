//
// Created by Molom on 2022-04-15.
//

#include "interpolationCurveC2.h"

using namespace std;
using namespace DirectX;

InterpolationCurveC2::InterpolationCurveC2(uint id, vector<std::weak_ptr<Point>> &&points)
        : Curve(id, "InterpolationC2", std::move(points), D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST) {
    updatePoints();
}

void InterpolationCurveC2::drawPolygonal(Renderer &renderer, DrawType drawType) {
    if (vertices.empty()) return;

    setTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
    switchIndexed(false);
    renderer.draw(*((Object *) this), drawType != DEFAULT ? POLYGONAL_COLOR : DEFAULT_COLOR);
    setTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
    switchIndexed(true);
}

void InterpolationCurveC2::drawCurve(Renderer &renderer, DrawType drawType) {
    renderer.draw(*this, drawType != DEFAULT ? SELECTED_COLOR : DEFAULT_COLOR);
}

void InterpolationCurveC2::preUpdate() {
    Curve::preUpdate();
    knotDistances.clear();
    alpha.clear();
    beta.clear();
    R.clear();
    c.clear();
    b.clear();
}

void InterpolationCurveC2::pointUpdate(const shared_ptr<Point> &point, int index) {
    // since we need next point to calculate difference and R, values are calculated for i = index - 1
    index--;
    if (index < 0) return;

    if (_points[index].expired()) {
        updatePoints();
        return;
    }
    auto position = _points[index].lock()->position();
    auto nextPosition = point->position();

    auto positionDiff = XMVectorSubtract(XMLoadFloat3(&nextPosition),
                                         XMLoadFloat3(&position));
    auto d = XMVector3Length(positionDiff).m128_f32[0];
    knotDistances.push_back(d);
    // TODO: handle knotDistance = 0

    // calculate alpha, beta and R for i = index - 1
    if (index == 0) return;

    auto prevD = knotDistances[index - 1];
    auto dSum = prevD + d;
    alpha.push_back(prevD / dSum);
    beta.push_back(d / dSum);

    if (_points[index - 1].expired()) {
        updatePoints();
        return;
    }
    auto prevPosition = _points[index - 1].lock()->position();
    auto prevPositionDiff = XMVectorSubtract(XMLoadFloat3(&position),
                                             XMLoadFloat3(&prevPosition));
    XMFLOAT3 r{};
    XMStoreFloat3(&r, XMVectorScale(
            XMVectorSubtract(XMVectorScale(positionDiff, 1 / d),
                             XMVectorScale(prevPositionDiff, 1 / prevD)),
            3.0f / dSum));
    R.push_back(r);
}

void InterpolationCurveC2::postUpdate() {
    if (_points.size() < 2) {
        Curve::postUpdate();
        return;
    }

    int size = _points.size() - 2;
    vector<float> mid(size, 2);
    c.resize(_points.size() - 1);
    b.resize(_points.size());
    c[0] = {0, 0, 0};

    for (int i = 1; i < size; ++i) {
        auto w = alpha[i] / mid[i - 1];
        mid[i] = mid[i] - w * beta[i - 1];
        XMStoreFloat3(&R[i], XMVectorSubtract(XMLoadFloat3(&R[i]),
                                              XMVectorScale(XMLoadFloat3(&R[i - 1]), w)));
    }

    if (_points.size() > 2) {
        XMStoreFloat3(&c.back(), XMVectorScale(XMLoadFloat3(&R.back()), 1.0f / mid.back()));
    }

    for (int i = size - 1; i > 0; --i) {
        XMStoreFloat3(&c[i], XMVectorScale(
                XMVectorSubtract(XMLoadFloat3(&R[i - 1]),
                                 XMVectorScale(XMLoadFloat3(&c[i + 1]), beta[i - 1])),
                1.0f / mid[i - 1]));
    }
    c.emplace_back(0, 0, 0);

    // calculate b (first derrivative) for all knots except last
    for (int i = 0; i < _points.size() - 1; ++i) {
        auto currentC = XMLoadFloat3(&c[i]);
        auto nextC = XMLoadFloat3(&c[i + 1]);

        if (_points[i].expired() || _points[i + 1].expired()) {
            updatePoints();
            return;
        }
        auto position = _points[i].lock()->position();
        auto positionNext = _points[i + 1].lock()->position();
        XMVECTOR left = XMVectorScale(
                XMVectorSubtract(XMLoadFloat3(&positionNext), XMLoadFloat3(&position)),
                1.0f / knotDistances[i]);
        XMVECTOR right = XMVectorScale(XMVectorAdd(nextC, XMVectorScale(currentC, 2.0f)),
                                       knotDistances[i] / 3.0f);

        XMStoreFloat3(&b[i], XMVectorSubtract(left, right));
    }

    // calculate last b
    {
        auto prevC = XMLoadFloat3(&c[c.size() - 2]);
        auto prevB = XMLoadFloat3(&b[b.size() - 2]);
        XMStoreFloat3(&b.back(),
                      XMVectorAdd(prevB,XMVectorScale(prevC, knotDistances.back())));
    }

    calculateControlPoints();

    Curve::postUpdate();
}

void InterpolationCurveC2::pointMoved(const weak_ptr<Point> &point) {
    // TODO: only modify finite amount of control points with single move
    updatePoints();
}

Type InterpolationCurveC2::type() const {
    return INTERPOLATIONC2;
}

void InterpolationCurveC2::calculateControlPoints() {
    for (int i = 0; i < _points.size() - 1; ++i) {
        if (_points[i].expired() || _points[i + 1].expired()) {
            updatePoints();
            return;
        }
        auto position = _points[i].lock()->position();
        auto positionNext = _points[i + 1].lock()->position();

        float scale = knotDistances[i] / 3.0f;
        XMFLOAT3 left{}, right{};
        XMStoreFloat3(&left, XMVectorAdd(XMLoadFloat3(&position), XMVectorScale(XMLoadFloat3(&b[i]), scale)));
        XMStoreFloat3(&right, XMVectorSubtract(XMLoadFloat3(&positionNext), XMVectorScale(XMLoadFloat3(&b[i + 1]), scale)));

        if (i == 0) {
            vertices.push_back({position, {1.0f, 1.0f, 1.0f}});
        }
        vertices.push_back({left, {1.0f, 1.0f, 1.0f}});
        vertices.push_back({right, {1.0f, 1.0f, 1.0f}});
        vertices.push_back({positionNext, {1.0f, 1.0f, 1.0f}});

        if (i == 0) {
            indices.push_back(0);
        } else {
            indices.push_back(indices.back());
        }
        indices.push_back(indices.back() + 1);
        indices.push_back(indices.back() + 1);
        indices.push_back(indices.back() + 1);
    }
}
