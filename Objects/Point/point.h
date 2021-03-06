//
// Created by Molom on 2022-03-19.
//

#ifndef MG1_POINT_H
#define MG1_POINT_H

#include "Objects/object.h"
#include <Models/Point.h>

class Point : public Object {
public:
    Point(uint id, DirectX::XMFLOAT3 position);

    explicit Point(const MG1::Point &point);

    void draw(Renderer &renderer, DrawType drawType) override;

    Type type() const override;

    bool intersects(DirectX::XMFLOAT3 origin, DirectX::XMFLOAT3 direction, DirectX::XMMATRIX viewMatrix,
                    float nearZ, float farZ, float &distance) const override;

    void setRotation(DirectX::XMFLOAT3 rotation) override {}

    void setScale(DirectX::XMFLOAT3 scale) override {}

    MG1::Point serialize();
};


#endif //MG1_POINT_H
