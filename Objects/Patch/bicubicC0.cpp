//
// Created by Molom on 2022-05-13.
//

#include "bicubicC0.h"

#include <utility>

using namespace std;
using namespace DirectX;

BicubicC0::BicubicC0(uint id, QString name, XMFLOAT3 position, array<int, PATCH_DIM> segments,
                     array<float, PATCH_DIM> size, bool cylinder,
                     QBindable<weak_ptr<Object>> bindableSelected)
        : Patch(id, std::move(name), position, {3, 3}, segments, cylinder, bindableSelected) {
    createSegments(segments, size);
}

void BicubicC0::createPlaneSegments(array<int, PATCH_DIM> segments, array<float, PATCH_DIM> size) {
    auto uDiff = size[0] / 3;
    auto vDiff = size[1] / 3;

    XMFLOAT3 startPos = _position;
    startPos.x -= uDiff * segments[0] * 1.5f;
    startPos.z -= vDiff * segments[1] * 1.5f;

    auto pos = startPos;
    auto uPoints = segments[0] * 3 + 1;
    auto vPoints = segments[1] * 3 + 1;
    // vertices
    for (int i = 0; i < vPoints; ++i) {
        for (int j = 0; j < uPoints; ++j) {
            addPoint(pos);
            pos.x += uDiff;
        }
        pos.x = startPos.x;
        pos.z += vDiff;
    }

    //indices
    for (int i = 0; i < segments[1]; ++i) {
        for (int j = 0; j < segments[0]; ++j) {
            auto index = i * 3 * uPoints + 3 * j;

            for (int k = 0; k < 4; ++k) {
                indices.push_back(index);
                indices.push_back(index + uPoints);
                indices.push_back(index + 2 * uPoints);
                indices.push_back(index + 3 * uPoints);
                index += 1;
            }
        }
    }
}

void BicubicC0::createCylinderSegments(array<int, PATCH_DIM> segments, array<float, PATCH_DIM> size) {
    auto radius = size[0];
    auto vDiff = size[1] / 3;

    XMFLOAT3 startPos = _position;
    startPos.z -= vDiff * segments[1] * 1.5f;

    auto uPoints = segments[0] * 3;
    auto vPoints = segments[1] * 3 + 1;
    auto uAngle = XM_2PI / uPoints;
    // vertices
    for (int i = 0; i < vPoints; ++i) {
        for (int j = 0; j < uPoints; ++j) {
            auto angle = j * uAngle;
            auto pos = XMFLOAT3(startPos.x + radius * sin(angle), startPos.y + radius * cos(angle), startPos.z);
            addPoint(pos);
        }
        startPos.z += vDiff;
    }

    //indices
    for (int i = 0; i < segments[1]; ++i) {
        for (int j = 0; j < segments[0]; ++j) {
            auto index = i * 3 * uPoints + 3 * j;

            for (int k = 0; k < 4; ++k) {
                if (j == segments[0] - 1 && k == 3) {
                    index = i * 3 * uPoints;
                }

                indices.push_back(index);
                indices.push_back(index + uPoints);
                indices.push_back(index + 2 * uPoints);
                indices.push_back(index + 3 * uPoints);
                index += 1;
            }
        }
    }
}

Type BicubicC0::type() const {
    return PATCHC0;
}

void BicubicC0::calculateMeshIndices(array<int, PATCH_DIM> segments, Linelist &linelist) {
    auto uPoints = segments[0] * 3 + (loopedU ? 0 : 1);
    auto vPoints = segments[1] * 3 + (loopedV ? 0 : 1);

    for (int i = 0; i < uPoints; ++i) {
        for (int j = 0; j < vPoints; ++j) {
            auto index = j * uPoints + i;
            auto nextLine = (index + uPoints) % linelist.vertices().size();
            auto nextIndex = j * uPoints + (i + 1) % uPoints;

            if (loopedU || i != uPoints - 1) linelist.addLine(index, nextIndex);
            if (loopedV || j != vPoints - 1) linelist.addLine(index, nextLine);
        }
    }
}

MG1::BezierSurfaceC0 BicubicC0::serialize(vector<MG1::Point> &serializedPoints) {
    return Patch::serialize<MG1::BezierSurfaceC0>(serializedPoints);
}

BicubicC0::BicubicC0(const MG1::BezierSurfaceC0 &surface, vector<MG1::Point> &serializedPoints,
                     QBindable<std::weak_ptr<Object>> bindableSelected)
        : Patch(surface, serializedPoints, bindableSelected) {
    calculateMeshIndices(segments, bezierMesh);
    updatePoints();
}

std::shared_ptr<VirtualPoint> BicubicC0::pointAt(std::pair<int, int> index) const {
    auto uPoints = segments[0] * 3 + (loopedU ? 0 : 1);
    auto idx = index.second * uPoints + index.first;
    return points[idx];
}

DirectX::XMVECTOR BicubicC0::value(const array<float, 2> &parameters) {
    auto[u, v] = parameters;
    auto control = getControlPoints(u, v);

    vector<XMVECTOR> p{4};
    for (int i = 0; i < 4; ++i) {
        p[i] = Utils3D::bernsteinPolynomial(
                {
                        XMLoadFloat3(&control[4 * i]),
                        XMLoadFloat3(&control[4 * i + 1]),
                        XMLoadFloat3(&control[4 * i + 2]),
                        XMLoadFloat3(&control[4 * i + 3]),
                }, v
        );
    }
    return Utils3D::bernsteinPolynomial(p, u);
}

DirectX::XMVECTOR BicubicC0::tangent(const array<float, 2> &parameters) {
    auto[u, v] = parameters;
    auto control = getControlPoints(u, v);

    vector<XMVECTOR> p{4};
    for (int i = 0; i < 4; ++i) {
        p[i] = Utils3D::bernsteinPolynomial(
                {
                        XMLoadFloat3(&control[4 * i]),
                        XMLoadFloat3(&control[4 * i + 1]),
                        XMLoadFloat3(&control[4 * i + 2]),
                        XMLoadFloat3(&control[4 * i + 3]),
                }, v
        );
    }

    vector<XMVECTOR> pp{
            XMVectorScale(XMVectorSubtract(p[1], p[0]), 3.f),
            XMVectorScale(XMVectorSubtract(p[2], p[1]), 3.f),
            XMVectorScale(XMVectorSubtract(p[3], p[2]), 3.f)
    };
    return Utils3D::bernsteinPolynomial(pp, u);
}

DirectX::XMVECTOR BicubicC0::bitangent(const array<float, 2> &parameters) {
    auto[u, v] = parameters;
    auto control = getControlPoints(u, v);

    vector<XMVECTOR> p{4};
    for (int i = 0; i < 4; ++i) {
        p[i] = Utils3D::bernsteinPolynomial(
                {
                        XMLoadFloat3(&control[i]),
                        XMLoadFloat3(&control[4 + i]),
                        XMLoadFloat3(&control[8 + i]),
                        XMLoadFloat3(&control[12 + i]),
                }, u
        );
    }

    vector<XMVECTOR> pp{
            XMVectorScale(XMVectorSubtract(p[1], p[0]), 3.f),
            XMVectorScale(XMVectorSubtract(p[2], p[1]), 3.f),
            XMVectorScale(XMVectorSubtract(p[3], p[2]), 3.f)
    };
    return Utils3D::bernsteinPolynomial(pp, v);
}
