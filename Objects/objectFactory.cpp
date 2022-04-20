//
// Created by Molom on 2022-04-20.
//

#include "objectFactory.h"

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
