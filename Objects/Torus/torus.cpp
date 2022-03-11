//
// Created by Molom on 2022-03-11.
//

#include <DirectXMath.h>
#include "torus.h"

using namespace std;
using namespace DirectX;

Torus::Torus(XMFLOAT3 position, XMFLOAT3 color, float minorRadius, float majorRadius, array<int, 2> density)
        : ParametricObject<2>(position, color, density, {make_tuple(0, XM_2PI), make_tuple(0, XM_2PI)}),
          minorRadius(minorRadius),
          majorRadius(majorRadius) {

    calculateVerticesAndIndices();
}

VertexPositionColor Torus::parametricFunction(std::array<float, 2> parameters) const {
    return {
            {
                    (majorRadius + minorRadius * cos(parameters[0])) * cos(parameters[1]),
                    minorRadius * sin(parameters[0]),
                    (majorRadius + minorRadius * cos(parameters[0])) * sin(parameters[1])
            }, color
    };
}

void Torus::setMajorRadius(float radius) {
    majorRadius = radius;
    calculateVerticesAndIndices();
}

void Torus::setMinorRadius(float radius) {
    minorRadius = radius;
    calculateVerticesAndIndices();
}



