//
// Created by Molom on 2022-06-19.
//

#include "intersectHandler.h"

using namespace std;
using namespace DirectX;

IntersectHandler::IntersectHandler(bool cursorExists, ObjectFactory &factory)
        : hasCursor(cursorExists),
          factory(factory) {

}

shared_ptr<Object> IntersectHandler::calculateIntersection() {
    auto starting = probeStartingPoint();

    return findIntersectCurve(starting);
}

shared_ptr<Object> IntersectHandler::calculateIntersection(XMFLOAT3 cursorPos) {
    if (!_useCursor) return calculateIntersection();

    auto starting = probeCursorPoint(cursorPos);

    return findIntersectCurve(starting);
}

IntersectHandler::IntersectPoint IntersectHandler::probeStartingPoint() const {
    auto uPoints = static_cast<int>(floor(sqrt(_maxPoints)));
    auto vPoints = static_cast<int>(round(_maxPoints / uPoints));

    auto pPoints = generatePoints(surfaces[0], uPoints, vPoints);
    auto qPoints = generatePoints(surfaces[1], uPoints, vPoints);

    IntersectPoint closest{};
    float minDistance = INFINITY;
    auto pointsCount = uPoints * vPoints;
    for (int i = 0; i < pointsCount; ++i) {
        for (int j = 0; j < pointsCount; ++j) {
            auto p1 = pPoints[i];
            auto p2 = qPoints[j];

            auto distance = XMVector3Length(XMVectorSubtract(p1.second, p2.second)).m128_f32[0];
            if (distance < minDistance) {
                minDistance = distance;
                closest = {p1.first.first, p1.first.second, p2.first.first, p2.first.second};
            }
        }
    }

    return closest;
}

IntersectHandler::IntersectPoint IntersectHandler::probeCursorPoint(XMFLOAT3 cursorPos) const {
    auto uPoints = static_cast<int>(floor(sqrt(_maxPoints)));
    auto vPoints = static_cast<int>(round(_maxPoints / uPoints));

    auto pPoints = generatePoints(surfaces[0], uPoints, vPoints);
    auto qPoints = generatePoints(surfaces[1], uPoints, vPoints);

    IntersectPoint closest{};
    auto cursor = XMLoadFloat3(&cursorPos);
    auto pointsCount = uPoints * vPoints;
    float minDistance = INFINITY;
    for (int i = 0; i < pointsCount; ++i) {
        auto p = pPoints[i];

        auto distance = XMVector3Length(XMVectorSubtract(p.second, cursor)).m128_f32[0];
        if (distance < minDistance) {
            minDistance = distance;
            closest.u = p.first.first;
            closest.v = p.first.second;
        }
    }

    minDistance = INFINITY;
    for (int i = 0; i < pointsCount; ++i) {
        auto p = qPoints[i];

        auto distance = XMVector3Length(XMVectorSubtract(p.second, cursor)).m128_f32[0];
        if (distance < minDistance) {
            minDistance = distance;
            closest.s = p.first.first;
            closest.t = p.first.second;
        }
    }

    return closest;
}

shared_ptr<Object> IntersectHandler::findIntersectCurve(IntersectPoint starting) {
    IntersectPoint firstIntersect{};
    if (!findIntersectPoint(starting, firstIntersect)) return {};

    // TODO: iterative newton
    XMFLOAT3 pos{};
    XMStoreFloat3(&pos, surfaces[0]->value({firstIntersect.u, firstIntersect.v}));
    return factory.createPoint(pos);


    return {};
}

bool IntersectHandler::findIntersectPoint(IntersectPoint starting, IntersectPoint &intersect) const {
    auto funcValue = [this](const IntersectPoint &point) {
        return XMVector3LengthSq(XMVectorSubtract(surfaces[0]->value({point.u, point.v}),
                                                  surfaces[1]->value({point.s, point.t}))).m128_f32[0];
    };

    auto gradValue = [this](const IntersectPoint &point) -> IntersectPoint {
        auto pVal = surfaces[0]->value({point.u, point.v});
        auto qVal = surfaces[1]->value({point.s, point.t});

        auto pVal2 = XMVectorScale(XMVectorSubtract(pVal, qVal), 2);
        auto qVal2 = XMVectorScale(XMVectorSubtract(qVal, pVal), 2);

        return {
                XMVector3Dot(pVal2, surfaces[0]->tangent({point.u, point.v})).m128_f32[0],
                XMVector3Dot(pVal2, surfaces[0]->bitangent({point.u, point.v})).m128_f32[0],
                XMVector3Dot(qVal2, surfaces[1]->tangent({point.s, point.t})).m128_f32[0],
                XMVector3Dot(qVal2, surfaces[1]->bitangent({point.s, point.t})).m128_f32[0],
        };
    };

    IntersectPoint point = starting;
    IntersectPoint grad{};
    float a = 0.5f;
    float value = funcValue(point);

    int iterations = 0;
    while (iterations++ < _maxPoints) {
        grad = gradValue(point);

        auto nextPoint = point - grad * a;
        auto nextValue = funcValue(nextPoint);

        if (grad.lenght() < epsilon || (nextPoint - point).lenght() < epsilon) {
            if (nextValue >= epsilon) return false;

            auto pRange = surfaces[0]->range();
            auto qRange = surfaces[1]->range();
            if (nextPoint.outOfRange(pRange[0], pRange[1], qRange[0], qRange[1])) return false;

            intersect.u = nextPoint.u;
            intersect.v = nextPoint.v;
            intersect.s = nextPoint.s;
            intersect.t = nextPoint.t;
            return true;
        }

        if (nextValue >= value) {
            a /= 2;
            continue;
        }

        point = nextPoint;
        value = nextValue;
    }

    return false;
}

vector<pair<pair<float, float>, XMVECTOR>> IntersectHandler::generatePoints(shared_ptr<ParametricObject<2>> surface,
                                                                            int uPoints, int vPoints) const {
    auto range = surface->range();

    auto[startU, endU] = range[0];
    auto deltaU = (endU - startU) / static_cast<float>(uPoints - 1);

    auto[startV, endV] = range[1];
    auto deltaV = (endV - startV) / static_cast<float>(vPoints - 1);

    vector<pair<pair<float, float>, XMVECTOR>> points{};
    points.reserve(uPoints * vPoints);

    pair<float, float> point{startU, startV};
    for (int i = 0; i < vPoints; ++i) {
        for (int j = 0; j < uPoints; ++j) {
            points.emplace_back(point, surface->value({point.first, point.second}));
            point.first += deltaU;
        }
        point.first = startU;
        point.second += deltaV;
    }

    return points;
}
