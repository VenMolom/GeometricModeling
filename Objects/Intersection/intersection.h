//
// Created by Molom on 2022-06-20.
//

#ifndef MG1_INTERSECTION_H
#define MG1_INTERSECTION_H


#include "Objects/object.h"
#include "Objects/Parametric/parametricObject.h"

class Intersection : public Object {
public:
    Intersection(uint id, const std::array<std::shared_ptr<ParametricObject<2>>, 2> &surfaces,
                 const std::vector<std::pair<float, float>> &firstParameters,
                 const std::vector<std::pair<float, float>> &secondParameters,
                 const std::vector<DirectX::XMFLOAT3> &points, bool closed, Renderer &renderer);

    void setPosition(DirectX::XMFLOAT3 position) override {}

    void setRotation(DirectX::XMFLOAT3 rotation) override {}

    void setScale(DirectX::XMFLOAT3 scale) override {}

    void draw(Renderer &renderer, DrawType drawType) override;

    Type type() const override;

    //TODO: convert to interpolation curve
    //TODO: display trimming textures

private:
    bool closed;
    std::array<std::shared_ptr<IntersectionInstance>, 2> instances;
};


#endif //MG1_INTERSECTION_H
