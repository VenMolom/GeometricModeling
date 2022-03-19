//
// Created by Molom on 2022-03-19.
//

#ifndef MG1_POINT_H
#define MG1_POINT_H

#include <DirectXMath.h>
#include "Objects/Object/object.h"

class Point : public Object {
public:
    Point(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 color);

    void draw(Renderer &renderer, const Camera &camera) const override;

    Type type() const override;
};


#endif //MG1_POINT_H
