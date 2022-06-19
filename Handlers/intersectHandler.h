//
// Created by Molom on 2022-06-19.
//

#ifndef MG1_INTERSECTHANDLER_H
#define MG1_INTERSECTHANDLER_H

#include "Objects/Parametric/parametricObject.h"

class IntersectHandler {
public:
    IntersectHandler(bool cursorExists);

    bool canUseCursor() { return hasCursor; }

    int maxPoints() { return _maxPoints; }

    float step() { return _step; }

    void setMaxPoints(int maxPoints) { _maxPoints = maxPoints; }

    void setStep(float step) { _step = step; }

    void useCursor(bool use) { _useCursor = use; }

    void setSurfaces(std::array<std::shared_ptr<ParametricObject<2>>, 2> surfaces) {
        this->surfaces = std::move(surfaces);
    }

    //TODO: change return type
    std::shared_ptr<Object> calculateIntersection();
    std::shared_ptr<Object> calculateIntersection(DirectX::XMFLOAT3 cursorPos);

private:
    bool _useCursor{};
    bool hasCursor{};
    int _maxPoints{};
    float _step{};
    std::array<std::shared_ptr<ParametricObject<2>>, 2> surfaces{};

    struct IntersectPoint {
        float u, v, s, t;
    };

    IntersectPoint probeStartingPoint();

    IntersectPoint probeCursorPoint(DirectX::XMFLOAT3 cursorPos);

    bool findIntersectPoint(IntersectPoint starting, IntersectPoint &intersect);

    std::shared_ptr<Object> findIntersectCurve(IntersectPoint starting);
};

#endif //MG1_INTERSECTHANDLER_H
