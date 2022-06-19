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
    //TODO: find starting intersect with simple gradient method
    return false;
}
