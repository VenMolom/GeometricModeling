//
// Created by Molom on 2022-02-27.
//

#ifndef MG1_OBJECT_H
#define MG1_OBJECT_H

#include "DirectX/DXDevice/dxptr.h"
#include "DirectX/DXStructures/dxStructures.h"
#include "Renderer/Renderer.h"

class Object {
protected:
    DirectX::XMFLOAT3 position;

    DirectX::XMFLOAT3 color;

    DirectX::XMFLOAT4X4 model;

    Object(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 color);

public:
    Object(Object &object) = default;

    Object(Object &&object) = default;

    virtual void draw(Renderer &renderer, const DirectX::XMMATRIX &camera) const = 0;

    DirectX::XMMATRIX modelMatrix() const;
};


#endif //MG1_OBJECT_H
