//
// Created by Molom on 2022-06-19.
//

#include "intersectHandler.h"

using namespace std;
using namespace DirectX;

IntersectHandler::IntersectHandler(bool cursorExists)
        : hasCursor(cursorExists) {

}

shared_ptr<Object> IntersectHandler::calculateIntersection() {
    auto starting = probeStartingPoint();

    return findIntersectCurve(starting);
}

shared_ptr<Object> IntersectHandler::calculateIntersection(XMFLOAT3 cursorPos) {
    auto starting = probeCursorPoint(cursorPos);

    return findIntersectCurve(starting);
}

IntersectHandler::IntersectPoint IntersectHandler::probeStartingPoint() {
    //TODO: generate points and find closest to cursor
    return IntersectHandler::IntersectPoint();
}

IntersectHandler::IntersectPoint IntersectHandler::probeCursorPoint(XMFLOAT3 cursorPos) {
    //TODO: probe close point by generating points and find closest pair
    return IntersectHandler::IntersectPoint();
}

shared_ptr<Object> IntersectHandler::findIntersectCurve(IntersectPoint starting) {
    IntersectPoint firstIntersect{};
    if (!findIntersectPoint(starting, firstIntersect)) return {};

    // TODO: iterative newton

    return {};
}

bool IntersectHandler::findIntersectPoint(IntersectPoint starting, IntersectPoint &intersect) {
    auto funcValue = [this](IntersectPoint point) {
        return XMVector3LengthSq(XMVectorSubtract(surfaces[0]->value({point.u, point.v}),
                                                  surfaces[1]->value({point.s, point.t}))).m128_f32[0];
    };

    IntersectPoint point = starting;
    IntersectPoint grad{};
    float a = 0.5f;
    float value = funcValue(point);

    int iterations = 0;
    while (iterations++ < _maxPoints) {
        grad = {
                XMVector3LengthSq(surfaces[0]->tangent({point.u, point.v})).m128_f32[0],
                XMVector3LengthSq(surfaces[0]->bitangent({point.u, point.v})).m128_f32[0],
                XMVector3LengthSq(surfaces[1]->tangent({point.s, point.t})).m128_f32[0],
                XMVector3LengthSq(surfaces[1]->bitangent({point.s, point.t})).m128_f32[0],
        };

        auto nextPoint = point - grad * a;
        auto nextValue = funcValue(nextPoint);

        if (grad.lenght() < epsilon || (nextPoint - point).lenght() < epsilon) {
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