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

std::shared_ptr<Intersection> ObjectFactory::createIntersection(const array<shared_ptr<ParametricObject<2>>, 2> &surfaces,
                                                                const vector<pair<float, float>> &firstParameters,
                                                                const vector<pair<float, float>> &secondParameters,
                                                                const vector<XMFLOAT3> &points, bool closed, Renderer &renderer) {
    return make_shared<Intersection>(nextId++, surfaces, firstParameters, secondParameters, points, closed, renderer);
}

std::shared_ptr<Intersection> ObjectFactory::createIntersection(const shared_ptr<ParametricObject<2>> &surface,
                                                                const vector<pair<float, float>> &firstParameters,
                                                                const vector<pair<float, float>> &secondParameters,
                                                                const vector<XMFLOAT3> &points,
                                                                bool closed, Renderer &renderer) {
    return make_shared<Intersection>(nextId++, surface, firstParameters, secondParameters, points, closed, renderer);
}

std::shared_ptr<CNCRouter> ObjectFactory::createCNCRouter(XMFLOAT3 position) {
    return make_shared<CNCRouter>(nextId++, position);
}


