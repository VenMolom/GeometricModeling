//
// Created by Molom on 2022-02-27.
//

#ifndef MG1_OBJECT_H
#define MG1_OBJECT_H

#include <qstring.h>

#include <utility>
#include <QProperty>
#include "DirectX/DXDevice/dxptr.h"
#include "DirectX/DXStructures/dxStructures.h"
#include "Renderer/Renderer.h"

enum Type {
    CURSOR,
    TORUS
};

class Object {
protected:
    QProperty<DirectX::XMFLOAT3> _position;
    QProperty<DirectX::XMFLOAT3> _rotation{{0, 0, 0}};
    QProperty<DirectX::XMFLOAT3> _scale{{1, 1, 1}};
    QProperty<DirectX::XMFLOAT3> _color;

    Object(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 color);

public:
    Object(Object &object) = default;

    Object(Object &&object) = default;

    DirectX::XMMATRIX modelMatrix() const;

    DirectX::XMFLOAT3 position() const { return _position; }

    void setPosition(DirectX::XMFLOAT3 position);

    QBindable<DirectX::XMFLOAT3> bindablePosition() { return &_position; }

    DirectX::XMFLOAT3 rotation() const { return _rotation; }

    void setRotation(DirectX::XMFLOAT3 rotation);

    QBindable<DirectX::XMFLOAT3> bindableRotation() { return &_rotation; }

    DirectX::XMFLOAT3 scale() const { return _scale; }

    void setScale(DirectX::XMFLOAT3 scale);

    QBindable<DirectX::XMFLOAT3> bindableScale() { return &_scale; }

    DirectX::XMFLOAT3 color() const { return _color; }

    void setColor(DirectX::XMFLOAT3 color);

    QBindable<DirectX::XMFLOAT3> bindableColor() { return &_color; }

    virtual void draw(Renderer &renderer, const DirectX::XMMATRIX &camera) const = 0;

    virtual Type type() const = 0;

private:
    DirectX::XMFLOAT4X4 model;

    void calculateModel();
};


#endif //MG1_OBJECT_H
