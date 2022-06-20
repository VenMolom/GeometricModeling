//
// Created by Molom on 2022-06-19.
//

#ifndef MG1_INTERSECTHANDLER_H
#define MG1_INTERSECTHANDLER_H

#include "Objects/Parametric/parametricObject.h"
#include "Objects/objectFactory.h"

class IntersectHandler {
public:
    IntersectHandler(bool cursorExists, ObjectFactory &factory);

    bool canUseCursor() const { return hasCursor; }

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

    float epsilon = 10e-6;
    std::array<std::shared_ptr<ParametricObject<2>>, 2> surfaces{};
    ObjectFactory &factory;

    struct IntersectPoint {
        float u, v, s, t;

        IntersectPoint operator-(const IntersectPoint &rhs) const {
            return {u - rhs.u, v - rhs.v, s - rhs.s, t - rhs.t};
        }

        IntersectPoint operator*(float a) const {
            return {u * a, v * a, s * a, t * a};
        }

        float lenght() const {
            return sqrt(u * u + v * v + s * s + t * t);
        }

        bool outOfRange(std::tuple<float, float> uRange, std::tuple<float, float> vRange,
                        std::tuple<float, float> sRange, std::tuple<float, float> tRange) {
            auto[startU, endU] = uRange;
            if (u > endU || u < startU) return false;

            auto[startV, endV] = vRange;
            if (v > endV || v < startV) return false;

            auto[startS, endS] = sRange;
            if (s > endS || s < startS) return false;

            auto[startT, endT] = tRange;
            if (t > endT || t < startT) return false;

            return true;
        }
    };

    IntersectPoint probeStartingPoint() const;

    IntersectPoint probeCursorPoint(DirectX::XMFLOAT3 cursorPos) const;

    bool findIntersectPoint(IntersectPoint starting, IntersectPoint &intersect) const;

    std::shared_ptr<Object> findIntersectCurve(IntersectPoint starting);

    std::vector<std::pair<std::pair<float, float>, DirectX::XMVECTOR>>
    generatePoints(std::shared_ptr<ParametricObject<2>> surface, int uPoints, int vPoints) const;
};

#endif //MG1_INTERSECTHANDLER_H
