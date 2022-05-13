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
    // TODO: direction dependent on rotation
    // TODO: maybe size dependent on scale?
    auto dens = density();
    auto uDiff = size[0] / 3;
    auto vDiff = size[1] / 3;

    XMFLOAT3 startPos = _position;
    startPos.x -= (uDiff * segments[0]) / 2;
    startPos.z -= (vDiff * segments[1]) / 2;

    auto pos = startPos;
    auto uPoints = (segments[0] * 4 - (segments[0] - 1));
    auto vPoints = (segments[1] * 4 - (segments[1] - 1));
    // vertices
    for (int i = 0; i < uPoints; ++i) {
        for (int j = 0; j < vPoints; ++j) {
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
    auto dens = density();
    auto uDiff = size[0] / 3;
    auto vDiff = size[1] / 3;

    XMFLOAT3 startPos = _position;
    startPos.x -= (uDiff * segments[0]) / 2;
    startPos.z -= (vDiff * segments[1]) / 2;

    auto pos = startPos;
    auto uPoints = (segments[0] * 4 - (segments[0] - 1));
    auto vPoints = (segments[1] * 4 - (segments[1] - 1));
    // vertices
    for (int i = 0; i < uPoints; ++i) {
        for (int j = 0; j < vPoints; ++j) {
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

Type BicubicC0::type() const {
    return PATCHC0;
}