//
// Created by Molom on 2022-03-19.
//

#ifndef MG1_POINT_H
#define MG1_POINT_H

#include <DirectXMath.h>
#include "Objects/Object/object.h"

class Point : public Object {
public:
    Point(DirectX::XMFLOAT3 position);

    void draw(Renderer &renderer, const Camera &camera, DrawType drawType) const override;

    Type type() const override;

    DirectX::BoundingOrientedBox boundingBox() const override;

private:
    DirectX::XMFLOAT3 size {0.5f, 0.5f, 0.5f};
    DirectX::XMFLOAT4 rot {0.0f, 0.0f, 0.0f, 1.0f};
};


#endif //MG1_POINT_H
