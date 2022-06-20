//
// Created by Molom on 2022-04-20.
//

#include "objectFactory.h"
#include "Objects/Patch/bicubicC0Creator.h"
#include "Objects/Patch/bicubicC2Creator.h"

using namespace std;
using namespace DirectX;

shared_ptr<Point> ObjectFactory::createPoint(XMFLOAT3 position) {
    return make_shared<Point>(nextId++, position);
}

shared_ptr<Torus> ObjectFactory::createTorus(XMFLOAT3 position) {
    return make_shared<Torus>(nextId++, position);
}

shared_ptr<BrezierC0> ObjectFactory::createBrezierC0(vector<weak_ptr<Point>> &&points) {
    return make_shared<BrezierC0>(nextId++, std::move(points));
}

shared_ptr<BrezierC2> ObjectFactory::createBrezierC2(vector<weak_ptr<Point>> &&points,
                                                     QBindable<weak_ptr<Object>> bindableSelected) {
    return make_shared<BrezierC2>(nextId++, std::move(points), bindableSelected);
}

shared_ptr<InterpolationCurveC2>
ObjectFactory::createInterpolationCurveC2(vector<weak_ptr<Point>> &&points) {
    return make_shared<InterpolationCurveC2>(nextId++, std::move(points));
}

std::shared_ptr<BicubicC0> ObjectFactory::createBicubicC0(DirectX::XMFLOAT3 position,
                                                          QBindable<weak_ptr<Object>> bindableSelected) {
    return make_shared<BicubicC0Creator>(position, bindableSelected);
}

std::shared_ptr<BicubicC2> ObjectFactory::createBicubicC2(DirectX::XMFLOAT3 position,
                                                          QBindable<weak_ptr<Object>> bindableSelected) {
    return make_shared<BicubicC2Creator>(position, bindableSelected);
}

std::shared_ptr<Intersection> ObjectFactory::createIntersection(const vector<XMFLOAT3> &points) {
    return make_shared<Intersection>(nextId++, points);
}


