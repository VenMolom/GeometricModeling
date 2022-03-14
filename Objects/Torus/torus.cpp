//
// Created by Molom on 2022-03-11.
//

#include "torus.h"
#include <algorithm>

using namespace std;
using namespace DirectX;

Torus::Torus(XMFLOAT3 position, XMFLOAT3 color, float minorRadius, float majorRadius, array<int, DIM> density)
        : ParametricObject<DIM>(position, color, density, {make_tuple(0, XM_2PI), make_tuple(0, XM_2PI)}),
          minorRadius(minorRadius),
          majorRadius(majorRadius) {

    calculateVerticesAndIndices();
}

void Torus::setMajorRadius(float radius) {
    majorRadius = radius;
    calculateVerticesAndIndices();
}

void Torus::setMinorRadius(float radius) {
    minorRadius = radius;
    calculateVerticesAndIndices();
}

vector<VertexPositionColor>
Torus::calculateVertices(const array<int, DIM> &density, const array<tuple<float, float>, DIM> &range) const {
    auto[startU, endU] = range[0];
    auto deltaU = (endU - startU) / static_cast<float>(density[0]);

    auto[startV, endV] = range[1];
    auto deltaV = (endV - startV) / static_cast<float>(density[1]);

    auto u = startU;
    vector<VertexPositionColor> vertices;
    for (auto i = 0; i < density[0]; ++i, u += deltaU) { // minor rotation
        auto v = startV;
        for (auto j = 0; j < density[1]; ++j, v += deltaV) { // major rotation
            vertices.push_back({
                                     {
                                             (majorRadius + minorRadius * cos(u)) * cos(v),
                                             minorRadius * sin(u),
                                             (majorRadius + minorRadius * cos(u)) * sin(v)
                                     }, color
                             });
        }
    }
    // major rotation first
    return vertices;
}

vector<Index> Torus::calculateIndices(const array<int, DIM> &density) const {
    vector<Index> indices;
    auto verticesSize = density[0] * density[1];
    for (auto i = 0; i < density[1]; ++i) { // major rotation
        for (auto j = 0; j < density[0]; ++j) { // minor rotation
            // lines around minor rotation
            auto index = j * density[1] + i;
            indices.push_back(index);
            indices.push_back((index + density[1]) % verticesSize);

            indices.push_back(index);
            indices.push_back((index + 1) % density[1] + j * density[1]);
        }
    }
    return indices;
}
