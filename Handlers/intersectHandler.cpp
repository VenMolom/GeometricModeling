//
// Created by Molom on 2022-06-19.
//

#include "intersectHandler.h"
#include <random>

using namespace std;
using namespace DirectX;

IntersectHandler::IntersectHandler(bool cursorExists, ObjectFactory &factory)
        : hasCursor(cursorExists),
          factory(factory) {

}

shared_ptr<Object> IntersectHandler::calculateIntersection(Renderer &renderer) {
    auto starting = probeStartingPoint();
    auto data = findIntersectCurve(starting);
    if (data.points.empty()) {
        return {};
    }
    return selfIntersection
           ? factory.createIntersection(surfaces[0], data.firstParams, data.secondParams, data.points, data.closed,
                                        renderer)
           : factory.createIntersection(surfaces, data.firstParams, data.secondParams, data.points, data.closed,
                                        renderer);
}

shared_ptr<Object> IntersectHandler::calculateIntersection(Renderer &renderer, XMFLOAT3 hint) {
    auto starting = _useCursor ? probeStartingPoint(hint) : probeStartingPoint();

    auto data = findIntersectCurve(starting);
    if (data.points.empty()) {
        return {};
    }
    return selfIntersection
           ? factory.createIntersection(surfaces[0], data.firstParams, data.secondParams, data.points, data.closed,
                                        renderer)
           : factory.createIntersection(surfaces, data.firstParams, data.secondParams, data.points, data.closed,
                                        renderer);
}

IntersectHandler::IntersectionData
IntersectHandler::calculateIntersection(Renderer &renderer, array<float, 4> starting) {
    IntersectPoint startingPoint = {starting[0], starting[1], starting[2], starting[3]};
    auto data = findIntersectCurve(startingPoint);
    return {std::move(data)};
}

IntersectHandler::IntersectPoint IntersectHandler::probeStartingPoint() const {
    auto pPoints = generatePoints(surfaces[0], maxPoints);
    auto qPoints = generatePoints(surfaces[1], maxPoints);

    IntersectPoint closest{};
    float minDistance = INFINITY;
    for (auto p1: pPoints) {
        for (auto p2: qPoints) {
            auto distance = XMVector3Length(XMVectorSubtract(p1.second, p2.second)).m128_f32[0];
            if (distance < minDistance &&
                (!selfIntersection || parameterDistance(p1.first, p2.first) > parameterEpsilon)) {
                minDistance = distance;
                closest = {p1.first.first, p1.first.second, p2.first.first, p2.first.second};
            }
        }
    }

    return closest;
}

IntersectHandler::IntersectPoint IntersectHandler::probeStartingPoint(XMFLOAT3 hint) const {
    auto pPoints = generatePoints(surfaces[0], maxPoints);
    auto qPoints = generatePoints(surfaces[1], maxPoints);

    IntersectPoint closest{};
    auto hintPos = XMLoadFloat3(&hint);
    float minDistance = INFINITY;
    for (auto p: pPoints) {
        auto distance = XMVector3Length(XMVectorSubtract(p.second, hintPos)).m128_f32[0];
        if (distance < minDistance) {
            minDistance = distance;
            closest.u = p.first.first;
            closest.v = p.first.second;
        }
    }

    minDistance = INFINITY;
    pair<float, float> p1 = {closest.u, closest.v};
    for (auto p: qPoints) {
        auto distance = XMVector3Length(XMVectorSubtract(p.second, hintPos)).m128_f32[0];
        if (distance < minDistance && (!selfIntersection || parameterDistance(p1, p.first) > parameterEpsilon)) {
            minDistance = distance;
            closest.s = p.first.first;
            closest.t = p.first.second;
        }
    }

    return closest;
}

IntersectHandler::IntersectionDataInternal IntersectHandler::findIntersectCurve(IntersectPoint starting) {
    IntersectPoint firstIntersect{};
    if (!findIntersectPoint(starting, firstIntersect)) return {};

    list<pair<IntersectPoint, XMVECTOR>> intersections;
    intersections.emplace_back(firstIntersect, surfaces[0]->value({firstIntersect.u, firstIntersect.v}));
    bool closed = false;
    while (true) {
        auto [i0, p0] = intersections.back();
        auto np = XMVector3Cross(surfaces[0]->tangent({i0.u, i0.v}),
                                 surfaces[0]->bitangent({i0.u, i0.v}));
        auto nq = XMVector3Cross(surfaces[1]->tangent({i0.s, i0.t}),
                                 surfaces[1]->bitangent({i0.s, i0.t}));
        auto t = XMVector3Normalize(XMVector3Cross(np, nq));
        IntersectPoint next{};

        // intersections.size() == 17
        auto result = calculateNextIntersectPoint(i0, next, p0, t);
        if (result == NoResult) break;

        intersections.emplace_back(next, surfaces[0]->value({next.u, next.v}));

        if (result == End) break;
        if (XMVector3Length(XMVectorSubtract(intersections.back().second,
                                             intersections.front().second)).m128_f32[0] < step / 2) {
            closed = true;
            break;
        }
    }

    if (!closed) {
        while (true) {
            auto [i0, p0] = intersections.front();
            auto np = XMVector3Cross(surfaces[0]->tangent({i0.u, i0.v}),
                                     surfaces[0]->bitangent({i0.u, i0.v}));
            auto nq = XMVector3Cross(surfaces[1]->tangent({i0.s, i0.t}),
                                     surfaces[1]->bitangent({i0.s, i0.t}));
            auto t = XMVector3Normalize(XMVector3Cross(nq, np));
            IntersectPoint next{};

            auto result = calculateNextIntersectPoint(i0, next, p0, t);
            if (result == NoResult) break;

            intersections.emplace_front(next, surfaces[0]->value({next.u, next.v}));

            if (result == End) break;
            if (XMVector3Length(XMVectorSubtract(intersections.front().second,
                                                 intersections.back().second)).m128_f32[0] < step / 2) {
                closed = true;
                break;
            }
        }
    }

    vector<XMFLOAT3> points{};
    vector<pair<float, float>> firstParams{}, secondParams{};
    for (auto &i: intersections) {
        XMFLOAT3 p{};
        XMStoreFloat3(&p, i.second);
        points.push_back(p);
        firstParams.emplace_back(i.first.u, i.first.v);
        secondParams.emplace_back(i.first.s, i.first.t);
    }

    IntersectionDataInternal d;
    d.firstParams = std::move(firstParams);
    d.secondParams = std::move(secondParams);
    d.points = std::move(points);
    d.closed = closed;
    return d;
}

IntersectHandler::PointResult IntersectHandler::calculateNextIntersectPoint(IntersectPoint start, IntersectPoint &next,
                                                                            XMVECTOR startValue, XMVECTOR t) const {
    auto funcValue = [this](const IntersectPoint &point, XMVECTOR v, XMVECTOR t) {
        auto value = surfaces[0]->value({point.u, point.v});
        auto vec = XMVectorSubtract(value, surfaces[1]->value({point.s, point.t}));

        vec.m128_f32[3] = XMVector3Dot(XMVectorSubtract(value, v), t).m128_f32[0] - step;
        return vec;
    };

    auto funcMatrix = [this](const IntersectPoint &point, XMVECTOR t) -> XMMATRIX {
        auto pu = surfaces[0]->tangent({point.u, point.v});
        auto pv = surfaces[0]->bitangent({point.u, point.v});
        auto qs = XMVectorNegate(surfaces[1]->tangent({point.s, point.t}));
        auto qt = XMVectorNegate(surfaces[1]->bitangent({point.s, point.t}));

        XMMATRIX mat{
                pu, pv, qs, qt
        };
        mat = XMMatrixTranspose(mat);
        mat.r[3] = XMVectorSet(XMVector3Dot(pu, t).m128_f32[0], XMVector3Dot(pv, t).m128_f32[0], 0, 0);

        return XMMatrixTranspose(mat);
    };

    IntersectPoint point = start;
    auto [wrapU, wrapV] = surfaces[0]->looped();
    auto [wrapS, wrapT] = surfaces[1]->looped();
    auto pRange = surfaces[0]->range();
    auto qRange = surfaces[1]->range();
    auto value = funcValue(point, startValue, t);
    int iteration = 0;
    while (iteration++ < iterations) {
        auto matrix = funcMatrix(point, t);
        XMVECTOR det{};
        auto mInv = XMMatrixInverse(&det, matrix);
        if (abs(det.m128_f32[0]) < epsilon) return NoResult;
        auto sol = XMVector4Transform(value, mInv);

        IntersectPoint solution{sol.m128_f32[0], sol.m128_f32[1], sol.m128_f32[2], sol.m128_f32[3]};
        auto nextPoint = point - solution;
        auto nextValue = funcValue(nextPoint, startValue, t);

        if (nextPoint.outOfRange(pRange[0], pRange[1], qRange[0], qRange[1], wrapU, wrapV, wrapS, wrapT)) {
            nextPoint.clampToRange(point, pRange[0], pRange[1], qRange[0], qRange[1]);
            next.u = nextPoint.u;
            next.v = nextPoint.v;
            next.s = nextPoint.s;
            next.t = nextPoint.t;
            return End;
        }

        nextPoint.wrap(pRange[0], pRange[1], qRange[0], qRange[1], wrapU, wrapV, wrapS, wrapT);

        if (solution.length() < epsilon) {
            next.u = nextPoint.u;
            next.v = nextPoint.v;
            next.s = nextPoint.s;
            next.t = nextPoint.t;
            return Found;
        }

        point = nextPoint;
        value = nextValue;
    }

    return NoResult;
}

bool IntersectHandler::findIntersectPoint(IntersectPoint starting, IntersectPoint &intersect) const {
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

    auto [wrapU, wrapV] = surfaces[0]->looped();
    auto [wrapS, wrapT] = surfaces[1]->looped();
    auto pRange = surfaces[0]->range();
    auto qRange = surfaces[1]->range();
    int iteration = 0;
    while (iteration++ < iterations) {
        grad = gradValue(point);

        auto nextPoint = point - grad * a;
        auto nextValue = funcValue(nextPoint);

        if (nextPoint.outOfRange(pRange[0], pRange[1], qRange[0], qRange[1], wrapU, wrapV, wrapS, wrapT)) {
            return false;
        }

        nextPoint.wrap(pRange[0], pRange[1], qRange[0], qRange[1], wrapU, wrapV, wrapS, wrapT);

        if (grad.length() < epsilon || (nextPoint - point).length() < epsilon) {
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

vector<pair<pair<float, float>, XMVECTOR>> IntersectHandler::generatePoints(shared_ptr<ParametricObject<2>> surface,
                                                                            int points) {
    static default_random_engine m_random{};

    auto range = surface->range();
    auto [startU, endU] = range[0];
    auto [startV, endV] = range[1];
    uniform_real_distribution<float> uDistribution(startU, endU);
    uniform_real_distribution<float> vDistribution(startV, endV);

    vector<pair<pair<float, float>, XMVECTOR>> pointsVec{};
    pointsVec.reserve(points);
    for (int i = 0; i < points; ++i) {
        auto u = uDistribution(m_random);
        auto v = vDistribution(m_random);
        pair<float, float> p = {u, v};
        pointsVec.emplace_back(p, surface->value({u, v}));
    }

    return pointsVec;
}

void IntersectHandler::setSurfaces(std::array<std::shared_ptr<ParametricObject<2>>, 2> surfaces) {
    {
        this->surfaces = std::move(surfaces);
        selfIntersection = this->surfaces[0].get() == this->surfaces[1].get();
    }
}

float IntersectHandler::parameterDistance(std::pair<float, float> uv1, std::pair<float, float> uv2) {
    auto u = uv1.first - uv2.first;
    auto v = uv1.second - uv2.second;
    return u * u + v * v;
}
