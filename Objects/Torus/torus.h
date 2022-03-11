//
// Created by Molom on 2022-03-11.
//

#ifndef MG1_TORUS_H
#define MG1_TORUS_H


#include <DirectXMath.h>
#include "Objects/ParametricObject/parametricObject.h"

class Torus : public ParametricObject<2> {
public:
    Torus(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 color,
          float minorRadius, float majorRadius,
          std::array<int, 2> density);

    void setMajorRadius(float radius);

    void setMinorRadius(float radius);

protected:
    VertexPositionColor parametricFunction(std::array<float, 2> parameters) const override;

private:
    float minorRadius, majorRadius;
};


#endif //MG1_TORUS_H
