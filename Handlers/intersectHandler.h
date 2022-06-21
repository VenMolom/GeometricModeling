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

    std::shared_ptr<Object> calculateIntersection();

    std::shared_ptr<Object> calculateIntersection(DirectX::XMFLOAT3 hint);

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

        float length() const {
            return sqrt(u * u + v * v + s * s + t * t);
        }

        bool outOfRange(const std::tuple<float, float> &uRange, const std::tuple<float, float> &vRange,
                        const std::tuple<float, float> &sRange, const std::tuple<float, float> &tRange,
                        bool wrapU = false, bool wrapV = false, bool wrapS = false, bool wrapT = false) {
            auto[startU, endU] = uRange;
            if ((u > endU || u < startU) && !wrapU) return true;

            auto[startV, endV] = vRange;
            if ((v > endV || v < startV) && !wrapV) return true;

            auto[startS, endS] = sRange;
            if ((s > endS || s < startS) && !wrapS) return true;

            auto[startT, endT] = tRange;
            if ((t > endT || t < startT) && !wrapT) return true;

            return false;
        }

        void clampToRange(const std::tuple<float, float> &uRange, const std::tuple<float, float> &vRange,
                        const std::tuple<float, float> &sRange, const std::tuple<float, float> &tRange) {
            auto[startU, endU] = uRange;
            u = std::clamp(u, startU, endU);

            auto[startV, endV] = vRange;
            v = std::clamp(v, startV, endV);

            auto[startS, endS] = sRange;
            s = std::clamp(s, startS, endS);

            auto[startT, endT] = tRange;
            t = std::clamp(t, startT, endT);
        }

        void wrap(const std::tuple<float, float> &uRange, const std::tuple<float, float> &vRange,
                  const std::tuple<float, float> &sRange, const std::tuple<float, float> &tRange,
                  bool wrapU, bool wrapV, bool wrapS, bool wrapT) {
            if (wrapU) {
                auto[startU, endU] = uRange;
                u = wrap(u, startU, endU);
            }
            if (wrapV) {
                auto[startV, endV] = vRange;
                v = wrap(v, startV, endV);
            }
            if (wrapS) {
                auto[startS, endS] = sRange;
                s = wrap(s, startS, endS);
            }
            if (wrapT) {
                auto[startT, endT] = tRange;
                t = wrap(t, startT, endT);
            }
        }

    private:
        static float wrap(float value, float start, float end) {
            auto v = value - start;
            auto l = end - start;
            return v - l * floor(v / l) + start;
        }
    };

    enum PointResult {
        Found,
        NoResult,
        End
    };

    IntersectPoint probeStartingPoint() const;

    IntersectPoint probeStartingPoint(DirectX::XMFLOAT3 hint) const;

    bool findIntersectPoint(IntersectPoint starting, IntersectPoint &intersect) const;

    std::shared_ptr<Object> findIntersectCurve(IntersectPoint starting);

    PointResult calculateNextIntersectPoint(IntersectPoint start, IntersectPoint &next,
                                               DirectX::XMVECTOR startValue, DirectX::XMVECTOR t) const;

    std::vector<std::pair<std::pair<float, float>, DirectX::XMVECTOR>>
    generatePoints(std::shared_ptr<ParametricObject<2>> surface, int uPoints, int vPoints) const;

    //TODO: self intersection
};

#endif //MG1_INTERSECTHANDLER_H
