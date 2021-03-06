//
// Created by Molom on 2022-04-08.
//

#ifndef MG1_GRID_H
#define MG1_GRID_H

#include <DirectXMath.h>
#include "Objects/object.h"

class Grid : public Object {
public:
    explicit Grid(int n);

    void draw(Renderer &renderer, DrawType drawType) override;

    Type type() const override;

    void setPosition(DirectX::XMFLOAT3 position) override {}

    void setRotation(DirectX::XMFLOAT3 rotation) override {}

    void setScale(DirectX::XMFLOAT3 scale) override {}

private:
    const static DirectX::XMFLOAT3 color;
};


#endif //MG1_GRID_H
