//
// Created by Molom on 2022-06-19.
//

#include "intersectHandler.h"

using namespace std;
using namespace DirectX;

IntersectHandler::IntersectHandler(bool cursorExists, ObjectFactory &factory)
        : hasCursor(cursorExists),
          factory(factory) {

}

shared_ptr<Object> IntersectHandler::calculateIntersection(Renderer &renderer) {
    auto starting = probeStartingPoint();
    return findIntersectCurve(starting, renderer);
}

shared_ptr<Object> IntersectHandler::calculateIntersection(Renderer &renderer, XMFLOAT3 hint) {
    auto starting = _useCursor ? probeStartingPoint(hint) : probeStartingPoint();
    return findIntersectCurve(starting, renderer);
}

IntersectHandler::IntersectPoint IntersectHandler::probeStartingPoint() const {
    auto uPoints = static_cast<int>(floor(sqrt(_maxPoints)));
    auto vPoints = static_cast<int>(round(_maxPoints / uPoints));

    auto pPoints = generatePoints(surfaces[0], uPoints, vPoints);
    auto qPoints = generatePoints(surfaces[1], uPoints, vPoints);

    IntersectPoint closest{};
    float minDistance = INFINITY;
    auto pointsCount = uPoints * vPoints;
    for (int i = 0; i < pointsCount; ++i) {
        for (int j = 0; j < pointsCount; ++j) {
            auto p1 = pPoints[i];
            auto p2 = qPoints[j];

            auto distance = XMVector3Length(XMVectorSubtract(p1.second, p2.second)).m128_f32[0];
            if (distance < minDistance) {
                minDistance = distance;
                closest = {p1.first.first, p1.first.second, p2.first.first, p2.first.second};
            }
        }
    }

    return closest;
}

IntersectHandler::IntersectPoint IntersectHandler::probeStartingPoint(XMFLOAT3 hint) const {
    auto uPoints = static_cast<int>(floor(sqrt(_maxPoints)));
    auto vPoints = static_cast<int>(round(_maxPoints / uPoints));

    auto pPoints = generatePoints(surfaces[0], uPoints, vPoints);
    auto qPoints = generatePoints(surfaces[1], uPoints, vPoints);

    IntersectPoint closest{};
    auto hintPos = XMLoadFloat3(&hint);
    auto pointsCount = uPoints * vPoints;
    float minDistance = INFINITY;
    for (int i = 0; i < pointsCount; ++i) {
        auto p = pPoints[i];

        auto distance = XMVector3Length(XMVectorSubtract(p.second, hintPos)).m128_f32[0];
        if (distance < minDistance) {
            minDistance = distance;
            closest.u = p.first.first;
            closest.v = p.first.second;
        }
    }

    minDistance = INFINITY;
    for (int i = 0; i < pointsCount; ++i) {
        auto p = qPoints[i];

        auto distance = XMVector3Length(XMVectorSubtract(p.second, hintPos)).m128_f32[0];
        if (distance < minDistance) {
            minDistance = distance;
            closest.s = p.first.first;
            closest.t = p.first.second;
        }
    }

    return closest;
}

shared_ptr<Object> IntersectHandler::findIntersectCurve(IntersectPoint starting, Renderer &renderer) {
    IntersectPoint firstIntersect{};
    if (!findIntersectPoint(starting, firstIntersect)) return {};

    list<pair<IntersectPoint, XMVECTOR>> intersections;
    intersections.emplace_back(firstIntersect, surfaces[0]->value({firstIntersect.u, firstIntersect.v}));
    bool closed = false;
    while (true) {
        auto[i0, p0] = intersections.back();
        auto np = XMVector3Cross(surfaces[0]->tangent({i0.u, i0.v}),
                                 surfaces[0]->bitangent({i0.u, i0.v}));
        auto nq = XMVector3Cross(surfaces[1]->tangent({i0.s, i0.t}),
                                 surfaces[1]->bitangent({i0.s, i0.t}));
        auto t = XMVector3Normalize(XMVector3Cross(np, nq));
        IntersectPoint next{};

        auto result = calculateNextIntersectPoint(i0, next, p0, t);
        if (result == NoResult) break;

        intersections.emplace_back(next, surfaces[0]->value({next.u, next.v}));

        if (result == End) break;
        if (XMVector3Length(XMVectorSubtract(intersections.back().second,
                                             intersections.front().second)).m128_f32[0] < _step / 2) {
            closed = true;
            break;
        }
    }

    if (!closed) {
        while (true) {
            auto[i0, p0] = intersections.front();
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
                                                 intersections.back().second)).m128_f32[0] < _step / 2) {
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
        //TODO: if wrap happens add additional points at both edges
        firstParams.emplace_back(i.first.u, i.first.v);
        secondParams.emplace_back(i.first.s, i.first.t);
    }

    return factory.createIntersection(surfaces, firstParams, secondParams, points, closed, renderer);
}

IntersectHandler::PointResult IntersectHandler::calculateNextIntersectPoint(IntersectPoint start, IntersectPoint &next,
                                                                            XMVECTOR startValue, XMVECTOR t) const {
    auto funcValue = [this](const IntersectPoint &point, XMVECTOR v, XMVECTOR t) {
        auto value = surfaces[0]->value({point.u, point.v});
        auto vec = XMVectorSubtract(value, surfaces[1]->value({point.s, point.t}));

        vec.m128_f32[3] = XMVector3Dot(XMVectorSubtract(value, v), t).m128_f32[0] - _step;
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
    auto value = funcValue(point, startValue, t);
    int iterations = 0;
    while (iterations++ < _maxPoints) {
        auto matrix = funcMatrix(point, t);
        XMVECTOR det{};
        auto mInv = XMMatrixInverse(&det, matrix);
        if (abs(det.m128_f32[0]) < epsilon) {
            return NoResult;
        }
        auto sol = XMVector4Transform(value, mInv);

        IntersectPoint solution{sol.m128_f32[0], sol.m128_f32[1], sol.m128_f32[2], sol.m128_f32[3]};
        auto nextPoint = point - solution;
        auto nextValue = funcValue(nextPoint, startValue, t);

        if (solution.length() < epsilon || (nextPoint - point).length() < epsilon) {
            if (XMVector3Length(nextValue).m128_f32[0] >= epsilon) {
                return NoResult;
            }

            auto[wrapU, wrapV] = surfaces[0]->looped();
            auto[wrapS, wrapT] = surfaces[1]->looped();
            auto pRange = surfaces[0]->range();
            auto qRange = surfaces[1]->range();

            if (nextPoint.outOfRange(pRange[0], pRange[1], qRange[0], qRange[1], wrapU, wrapV, wrapS, wrapT)) {
                nextPoint.clampToRange(pRange[0], pRange[1], qRange[0], qRange[1]);
                next.u = nextPoint.u;
                next.v = nextPoint.v;
                next.s = nextPoint.s;
                next.t = nextPoint.t;
                return End;
            }

            nextPoint.wrap(pRange[0], pRange[1], qRange[0], qRange[1], wrapU, wrapV, wrapS, wrapT);

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

    int iterations = 0;
    while (iterations++ < _maxPoints) {
        grad = gradValue(point);

        auto nextPoint = point - grad * a;
        auto nextValue = funcValue(nextPoint);

        if (grad.length() < epsilon || (nextPoint - point).length() < epsilon) {
            if (nextValue >= epsilon) return false;

            auto pRange = surfaces[0]->range();
            auto qRange = surfaces[1]->range();
            if (nextPoint.outOfRange(pRange[0], pRange[1], qRange[0], qRange[1])) return false;
            // TODO: może wrap tutaj

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
                                                                            int uPoints, int vPoints) const {
    auto range = surface->range();

    auto[startU, endU] = range[0];
    auto deltaU = (endU - startU) / static_cast<float>(uPoints - 1);

    auto[startV, endV] = range[1];
    auto deltaV = (endV - startV) / static_cast<float>(vPoints - 1);

    vector<pair<pair<float, float>, XMVECTOR>> points{};
    points.reserve(uPoints * vPoints);

    pair<float, float> point{startU, startV};
    for (int i = 0; i < vPoints; ++i) {
        for (int j = 0; j < uPoints; ++j) {
            points.emplace_back(point, surface->value({point.first, point.second}));
            point.first += deltaU;
        }
        point.first = startU;
        point.second += deltaV;
    }

    return points;
}
