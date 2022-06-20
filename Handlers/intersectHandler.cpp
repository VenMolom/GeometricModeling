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

IntersectHandler::IntersectPoint IntersectHandler::probeStartingPoint() {
    //TODO: generate points and find closest to cursor
    return {};
}

IntersectHandler::IntersectPoint IntersectHandler::probeCursorPoint(XMFLOAT3 cursorPos) {
    //TODO: probe close point by generating points and find closest pair
    return {};
}

shared_ptr<Object> IntersectHandler::findIntersectCurve(IntersectPoint starting) {
    IntersectPoint firstIntersect{};
    if (!findIntersectPoint(starting, firstIntersect)) return {};

    //TODO: remove
    XMFLOAT3 pos{};
    XMStoreFloat3(&pos, surfaces[0]->value({firstIntersect.u, firstIntersect.v}));
    return factory.createPoint(pos);

    // TODO: iterative newton

    return {};
}

bool IntersectHandler::findIntersectPoint(IntersectPoint starting, IntersectPoint &intersect) {
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