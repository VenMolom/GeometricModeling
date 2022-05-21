//
// Created by Molom on 2022-05-21.
//

#include "bicubicC2.h"

using namespace std;
using namespace DirectX;

BicubicC2::BicubicC2(uint id, QString name, XMFLOAT3 position, array<int, PATCH_DIM> segments,
                     array<float, PATCH_DIM> size, bool cylinder,
                     QBindable<weak_ptr<Object>> bindableSelected)
        : Patch(id, name, position, {3, 3}, cylinder, bindableSelected) {
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
                indices.push_back(index + 1);
                indices.push_back(index + 2);
                indices.push_back(index + 3);
                index += uPoints;
            }
        }
    }
}

void BicubicC2::createCylinderSegments(array<int, PATCH_DIM> segments, array<float, PATCH_DIM> size) {
    auto uDiff = size[0];
    auto radius = size[1];

    XMFLOAT3 startPos = _position;
    startPos.x -= uDiff * (segments[0] * 0.5f + 1.f);

    auto uPoints = segments[0] + 3;
    auto vPoints = segments[1];
    auto vAngle = XM_2PI / vPoints;
    // vertices
    for (int i = 0; i < vPoints; ++i) {
        auto angle = i * vAngle;
        auto pos = XMFLOAT3(startPos.x, startPos.y + radius * cos(angle), startPos.z + radius * sin(angle));
        for (int j = 0; j < uPoints; ++j) {
            addPoint(pos);
            pos.x += uDiff;
        }
    }

    //indices
    for (int i = 0; i < segments[0]; ++i) {
        for (int j = 0; j < segments[1]; ++j) {
            auto index = j * uPoints + i;

            for (int k = 0; k < 4; ++k) {
                if (j == segments[1] - k) {
                    index = i;
                }

                indices.push_back(index);
                indices.push_back(index + 1);
                indices.push_back(index + 2);
                indices.push_back(index + 3);
                index += uPoints;
            }
        }
    }
}

Type BicubicC2::type() const {
    return PATCHC2;
}

void BicubicC2::calculateMeshIndices(array<int, PATCH_DIM> segments, Linelist &linelist) {
    return;
    auto uPoints = (segments[0] * 4 - (segments[0] - 1));
    for (int i = 0; i < segments[0]; ++i) {
        for (int j = 0; j < segments[1]; ++j) {
            auto index = j * 3 * uPoints + 3 * i;

            for (int k = 0; k < 3; ++k) {
                if (cylinder && j == segments[1] - 1 && k == 3) {
                    return;
                }

                auto nextLine = (index + uPoints) % linelist.vertices().size();

                linelist.addLine(index, index + 1);
                linelist.addLine(index + 1, index + 2);
                linelist.addLine(index + 2, index + 3);

                linelist.addLine(index, nextLine);
                linelist.addLine(index + 1, nextLine + 1);
                linelist.addLine(index + 2, nextLine + 2);
                linelist.addLine(index + 3, nextLine + 3);

                index += uPoints;
            }

            linelist.addLine(index, index + 1);
            linelist.addLine(index + 1, index + 2);
            linelist.addLine(index + 2, index + 3);
        }
    }
}

void BicubicC2::drawMesh(Renderer &renderer, DrawType drawType) {
    renderer.draw(*this, drawType != DEFAULT ? SELECTED_COLOR : DEFAULT_COLOR);
}
