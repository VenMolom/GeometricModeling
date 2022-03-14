//
// Created by Molom on 2022-03-11.
//

#ifndef MG1_TORUS_H
#define MG1_TORUS_H

#include "Objects/ParametricObject/parametricObject.h"

#define DIM 2

class Torus : public ParametricObject<DIM> {
public:
    Torus(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 color,
          float minorRadius, float majorRadius,
          std::array<int, DIM> density);

    void setMajorRadius(float radius);

    void setMinorRadius(float radius);

    Type type() const override { return TORUS; }

protected:
    std::vector<VertexPositionColor> calculateVertices(const std::array<int, DIM> &density,
                                                       const std::array<std::tuple<float, float>, DIM> &range) const override;

    std::vector<Index> calculateIndices(const std::array<int, DIM> &density) const override;

private:
    float minorRadius, majorRadius;
};


#endif //MG1_TORUS_H
