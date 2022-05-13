//
// Created by Molom on 2022-03-11.
//

#ifndef MG1_TORUS_H
#define MG1_TORUS_H

#include "parametricObject.h"

#define TORUS_DIM 2

class Torus : public ParametricObject<TORUS_DIM> {
public:
    Torus(uint id, DirectX::XMFLOAT3 position);

    float majorRadius() const { return _majorRadius; }

    void setMajorRadius(float radius);

    float minorRadius() const { return _minorRadius; }

    void setMinorRadius(float radius);

    void draw(Renderer &renderer, DrawType drawType) override;

    Type type() const override { return TORUS; }

    std::array<bool, TORUS_DIM> looped() const override;

    bool intersects(DirectX::XMFLOAT3 origin, DirectX::XMFLOAT3 direction, DirectX::XMMATRIX viewMatrix,
                    float nearZ, float farZ, float &distance) const override;

protected:
    void densityUpdated() override;

private:
    float _minorRadius{1};
    float _majorRadius{3};

    void calculateVertices(const std::array<int, TORUS_DIM> &density,
                           const std::array<std::tuple<float, float>, TORUS_DIM> &range);

    void calculateIndices(const std::array<int, TORUS_DIM> &density);
};


#endif //MG1_TORUS_H
