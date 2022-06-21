//
// Created by Molom on 2022-06-20.
//

#ifndef MG1_INTERSECTION_H
#define MG1_INTERSECTION_H


#include "Objects/object.h"

class Intersection : public Object {
public:
    Intersection(uint id, const std::vector<DirectX::XMFLOAT3> &points, bool closed);

    void setPosition(DirectX::XMFLOAT3 position) override {}

    void setRotation(DirectX::XMFLOAT3 rotation) override {}

    void setScale(DirectX::XMFLOAT3 scale) override {}

    void draw(Renderer &renderer, DrawType drawType) override;

    Type type() const override;
};


#endif //MG1_INTERSECTION_H
