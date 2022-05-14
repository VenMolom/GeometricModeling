//
// Created by Molom on 2022-05-13.
//

#include "bicubicC0.h"
#include <algorithm>

using namespace std;
using namespace DirectX;

BicubicC0::BicubicC0(uint id, XMFLOAT3 position, array<int, PATCH_DIM> segments,
                     array<float, PATCH_DIM> size, bool cylinder,
                     QBindable<weak_ptr<Object>> bindableSelected)
        : Patch(id, "BicubicC0", position, {3, 3}, cylinder, bindableSelected) {
    if (cylinder) {
        createCylinderSegments(segments, size);
    } else {
        createPlaneSegments(segments, size);
    }
}

void BicubicC0::createPlaneSegments(array<int, PATCH_DIM> segments, array<float, PATCH_DIM> size) {
    auto uDiff = size[0] / 3;
    auto vDiff = size[1] / 3;

    XMFLOAT3 startPos = _position;
    startPos.x -= uDiff * segments[0] * 1.5f;
    startPos.z -= vDiff * segments[1] * 1.5f;

    auto pos = startPos;
    auto uPoints = (segments[0] * 4 - (segments[0] - 1));
    auto vPoints = (segments[1] * 4 - (segments[1] - 1));
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
            auto index = j * 3 * uPoints + 3 * i;

            for (int k = 0; k < 4; ++k) {
                indices.push_back(index);
                indices.push_back(index + 1);
                indices.push_back(index + 2);
                indices.push_back(index + 3);
                index += uPoints;
            }
        }
    }

    updateBuffers();
}

void BicubicC0::createCylinderSegments(std::array<int, PATCH_DIM> segments, std::array<float, PATCH_DIM> size) {
    auto uDiff = size[0] / 3;
    auto radius = size[1];

    XMFLOAT3 startPos = _position;
    startPos.x -= uDiff * segments[0] * 1.5f;

    auto uPoints = (segments[0] * 4 - (segments[0] - 1));
    auto vPoints = segments[1] * 3;
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
            auto index = j * 3 * uPoints + 3 * i;

            for (int k = 0; k < 4; ++k) {
                if (j == segments[1] - 1 && k == 3) {
                    index = 3 * i;
                }

                indices.push_back(index);
                indices.push_back(index + 1);
                indices.push_back(index + 2);
                indices.push_back(index + 3);
                index += uPoints;
            }
        }
    }

    updateBuffers();
}

Type BicubicC0::type() const {
    return PATCHC0;
}