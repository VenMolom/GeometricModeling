//
// Created by Molom on 2022-02-27.
//

#ifndef MG1_OBJECT_H
#define MG1_OBJECT_H

#include "DirectX/DXDevice/dxptr.h"
#include "DirectX/DXStructures/dxStructures.h"
#include "Renderer/Renderer.h"


enum Type {
    TORUS
};

class Object {
protected:
    DirectX::XMFLOAT3 position;

    DirectX::XMFLOAT3 color;

    DirectX::XMFLOAT4X4 model;

    Object(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 color);

public:
    Object(Object &object) = default;

    Object(Object &&object) = default;

    DirectX::XMMATRIX modelMatrix() const;

    virtual void draw(Renderer &renderer, const DirectX::XMMATRIX &camera) const = 0;

    virtual Type type() const = 0;
};


#endif //MG1_OBJECT_H
