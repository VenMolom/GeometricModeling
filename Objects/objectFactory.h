//
// Created by Molom on 2022-04-20.
//

#ifndef MG1_OBJECTFACTORY_H
#define MG1_OBJECTFACTORY_H

#include "Objects/Point/point.h"
#include "Objects/Curve/brezierC0.h"
#include "Objects/Curve/brezierC2.h"
#include "Objects/Patch/bicubicC0.h"
#include "Objects/Patch/bicubicC2.h"
#include "Objects/Parametric/torus.h"
#include "Objects/Curve/interpolationCurveC2.h"
#include "Objects/Intersection/intersection.h"

class ObjectFactory {
public:
    std::shared_ptr<Point> createPoint(DirectX::XMFLOAT3 position);

    std::shared_ptr<Torus> createTorus(DirectX::XMFLOAT3 position);

    std::shared_ptr<BrezierC0> createBrezierC0(std::vector<std::weak_ptr<Point>> &&points);

    std::shared_ptr<BrezierC2> createBrezierC2(std::vector<std::weak_ptr<Point>> &&points,
                                                 QBindable<std::weak_ptr<Object>> bindableSelected);

    std::shared_ptr<InterpolationCurveC2> createInterpolationCurveC2(std::vector<std::weak_ptr<Point>> &&points);

    std::shared_ptr<BicubicC0> createBicubicC0(DirectX::XMFLOAT3 position, QBindable<std::weak_ptr<Object>> bindableSelected);

    std::shared_ptr<BicubicC2> createBicubicC2(DirectX::XMFLOAT3 position, QBindable<std::weak_ptr<Object>> bindableSelected);

    std::shared_ptr<Intersection> createIntersection(const std::vector<DirectX::XMFLOAT3> &points, bool closed);

    uint id() { return nextId++; }
private:
    friend class Scene;

    uint nextId = 1;
};

#endif //MG1_OBJECTFACTORY_H
