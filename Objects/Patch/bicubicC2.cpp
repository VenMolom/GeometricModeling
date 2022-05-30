//
// Created by Molom on 2022-05-21.
//

#include "bicubicC2.h"

#include <utility>

using namespace std;
using namespace DirectX;

BicubicC2::BicubicC2(uint id, QString name, XMFLOAT3 position, array<int, PATCH_DIM> segments,
                     array<float, PATCH_DIM> size, bool cylinder,
                     QBindable<weak_ptr<Object>> bindableSelected)
        : Patch(id, std::move(name), position, {3, 3}, segments, cylinder, bindableSelected) {
    createSegments(segments, size);
}

void BicubicC2::createPlaneSegments(array<int, PATCH_DIM> segments, array<float, PATCH_DIM> size) {
    auto uDiff = size[0];
    auto vDiff = size[1];

    XMFLOAT3 startPos = _position;
    startPos.x -= uDiff * (segments[0] * 0.5f + 1.f);
    startPos.z -= vDiff * (segments[1] * 0.5f + 1.f);

    auto pos = startPos;
    auto uPoints = segments[0] + 3;
    auto vPoints = segments[1] + 3;
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
    for (int i = 0; i < segments[0]; ++i) {
        for (int j = 0; j < segments[1]; ++j) {
            auto index = j * uPoints + i;

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

void BicubicC2::createCylinderSegments(array<int, PATCH_DIM> segments, array<float, PATCH_DIM> size) {
    auto radius = size[0];
    auto vDiff = size[1];

    XMFLOAT3 startPos = _position;
    startPos.z -= vDiff * (segments[1] * 0.5f + 1.f);

    auto uPoints = segments[0];
    auto vPoints = segments[1] + 3;
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
    for (int i = 0; i < segments[0]; ++i) {
        for (int j = 0; j < segments[1]; ++j) {
            auto index = j * uPoints + i;

            for (int k = 0; k < 4; ++k) {
                if (i == segments[0] - k) {
                    index = j * uPoints;
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

Type BicubicC2::type() const {
    return PATCHC2;
}

void BicubicC2::calculateMeshIndices(array<int, PATCH_DIM> segments, Linelist &linelist) {
    auto uPoints = segments[0] + (loopedU ? 0 : 3);
    auto vPoints = segments[1] + (loopedV ? 0 : 3);

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

void BicubicC2::drawMesh(Renderer &renderer, DrawType drawType) {
    renderer.draw(*this, drawType != DEFAULT ? SELECTED_COLOR : DEFAULT_COLOR);
}

MG1::BezierSurfaceC2 BicubicC2::serialize(vector<MG1::Point> &serializedPoints) {
    return Patch::serialize<MG1::BezierSurfaceC2>(serializedPoints);
}

BicubicC2::BicubicC2(const MG1::BezierSurfaceC2 &surface, vector<MG1::Point> &serializedPoints,
                     QBindable<std::weak_ptr<Object>> bindableSelected)
                     : Patch(surface, serializedPoints, bindableSelected) {
    calculateMeshIndices(segments, bezierMesh);
    updatePoints();
}
