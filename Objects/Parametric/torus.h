//
// Created by Molom on 2022-03-11.
//

#ifndef MG1_TORUS_H
#define MG1_TORUS_H

#include "parametricObject.h"

#define DIM 2

class Torus : public ParametricObject<DIM> {
public:
    Torus(DirectX::XMFLOAT3 position);

    float majorRadius() const { return _majorRadius; }

    void setMajorRadius(float radius);

    float minorRadius() const { return _minorRadius; }

    void setMinorRadius(float radius);

    Type type() const override { return TORUS; }

    std::array<bool, 2> looped() const override;

    DirectX::BoundingOrientedBox boundingBox() const override;

protected:
    std::vector<VertexPositionColor> &&calculateVertices(const std::array<int, DIM> &density,
                                                       const std::array<std::tuple<float, float>, DIM> &range) const override;

    std::vector<Index> &&calculateIndices(const std::array<int, DIM> &density) const override;

private:
    float _minorRadius{1};
    float _majorRadius{3};
};


#endif //MG1_TORUS_H
