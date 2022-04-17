//
// Created by Molom on 2022-04-15.
//

#include "interpolationCurveC2.h"

using namespace std;
using namespace DirectX;

InterpolationCurveC2::InterpolationCurveC2(vector<std::weak_ptr<Point>> &&points)
        : Curve("InterpolationC2", std::move(points), D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST),
        controlPoints() {
    updatePoints();
}

void InterpolationCurveC2::drawPolygonal(Renderer &renderer, DrawType drawType) {
    if (controlPoints.vertices().empty()) return;
    controlPoints.draw(renderer, drawType);
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
    controlPoints.vertices().clear();
}

void InterpolationCurveC2::pointUpdate(const shared_ptr<Point> &point, int index) {
    controlPoints.vertices().push_back({point->position(), {1, 1, 1}});

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
    vector<float> b(size, 2);
    vector<XMFLOAT3> c(size + 1);
    c[0] = {0, 0, 0};

    for (int i = 1; i < size; ++i) {
        auto w = alpha[i] / b[i - 1];
        b[i] = b[i] - w * beta[i - 1];
        XMStoreFloat3(&R[i], XMVectorSubtract(XMLoadFloat3(&R[i]),
                                              XMVectorScale(XMLoadFloat3(&R[i - 1]), w)));
    }

    if (_points.size() > 2) {
        XMStoreFloat3(&c.back(), XMVectorScale(XMLoadFloat3(&R.back()), 1.0f / b.back()));
    }

    for (int i = size - 1; i > 0; --i) {
        XMStoreFloat3(&c[i], XMVectorScale(
                XMVectorSubtract(XMLoadFloat3(&R[i - 1]),
                                 XMVectorScale(XMLoadFloat3(&c[i + 1]), beta[i - 1])),
                1.0f / b[i - 1]));
    }
    c.emplace_back(0, 0, 0);

    for (int i = 0; i < _points.size() - 1; ++i) {
        calculateControlPoints(c, i);
    }

    Curve::postUpdate();
    controlPoints.update();
}

void InterpolationCurveC2::pointMoved(const weak_ptr<Point> &point) {
    // TODO: only modify finite amount of control points with single move
    updatePoints();
}

Type InterpolationCurveC2::type() const {
    return INTERPOLATIONC2;
}

void InterpolationCurveC2::calculateControlPoints(const vector<DirectX::XMFLOAT3> &c, int index) {
    // TODO: there is probably better evaluation method than Horner
    XMFLOAT3 a{}, b{}, d{};

    auto currentC = XMLoadFloat3(&c[index]);
    auto nextC = XMLoadFloat3(&c[index + 1]);

    XMStoreFloat3(&d, XMVectorScale(XMVectorSubtract(nextC, currentC),
                                    1.0f / (3.0f * knotDistances[index])));

    auto position = _points[index].lock()->position();
    auto positionNext = _points[index + 1].lock()->position();
    XMVECTOR left = XMVectorScale(
            XMVectorSubtract(XMLoadFloat3(&positionNext), XMLoadFloat3(&position)),
            1.0f / knotDistances[index]);
    XMVECTOR right = XMVectorScale(XMVectorAdd(nextC, XMVectorScale(currentC, 2.0f)),
                                   knotDistances[index] / 3.0f);

    XMStoreFloat3(&b, XMVectorSubtract(left, right));

    a = position;

    vertices.push_back({d, {1.0f, 1.0f, 1.0f}});
    vertices.push_back({c[index], {1.0f, 1.0f, 1.0f}});
    vertices.push_back({b, {1.0f, 1.0f, 1.0f}});
    vertices.push_back({a, {knotDistances[index], 0, 0}});
}
