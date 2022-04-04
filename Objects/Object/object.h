//
// Created by Molom on 2022-02-27.
//

#ifndef MG1_OBJECT_H
#define MG1_OBJECT_H

#include <qstring.h>
#include <utility>
#include <QProperty>
#include <DirectXCollision.h>
#include <DirectXMath.h>
#include "DirectX/DXDevice/dxptr.h"
#include "DirectX/DXStructures/dxStructures.h"
#include "Renderer/Renderer.h"
#include "Camera/camera.h"

enum Type {
    CURSOR = 1 << 0,
    POINT3D = 1 << 1,
    TORUS = 1 << 2,
    COMPOSITE = 1 << 3,
    BREZIERC0 = 1 << 4,
    BREZIERC2 = 1 << 5,
    VIRTUALPOINT3D = 1 << 6,
    BREZIERCURVE = BREZIERC0 | BREZIERC2,
    MOVEABLE = POINT3D | TORUS | COMPOSITE | CURSOR | VIRTUALPOINT3D,
    NAMEABLE = POINT3D | TORUS | BREZIERCURVE,
    TRANSFORMABLE = TORUS | COMPOSITE,
    SCREENMOVEABLE = CURSOR,
    VIRTUALPOINTSHOLDER = BREZIERC2
};

inline Type operator|(Type a, Type b)
{
    return static_cast<Type>(static_cast<int>(a) | static_cast<int>(b));
}

inline Type operator&(Type a, Type b)
{
    return static_cast<Type>(static_cast<int>(a) & static_cast<int>(b));
}

enum DrawType {
    SELECTED,
    NO_CURSOR,
    DEFAULT
};

class Object {
protected:
    QProperty<DirectX::XMFLOAT3> _position;
    QProperty<DirectX::XMFLOAT3> _rotation{{0, 0, 0}};
    QProperty<DirectX::XMFLOAT3> _scale{{1, 1, 1}};
    QProperty<QString> _name;

    DirectX::XMFLOAT4X4 noScaleMatrix;
    DirectX::XMFLOAT4X4 rotationMatrix;
    DirectX::XMFLOAT4X4 scaleMatrix;

    Object(QString name, DirectX::XMFLOAT3 position);

public:
    Object(Object &object) = default;

    Object(Object &&object) = default;

    DirectX::XMMATRIX modelMatrix() const;

    DirectX::XMFLOAT3 position() const { return _position; }

    virtual void setPosition(DirectX::XMFLOAT3 position);

    QBindable<DirectX::XMFLOAT3> bindablePosition() { return &_position; }

    DirectX::XMFLOAT3 rotation() const { return _rotation; }

    virtual void setRotation(DirectX::XMFLOAT3 rotation);

    QBindable<DirectX::XMFLOAT3> bindableRotation() { return &_rotation; }

    DirectX::XMFLOAT3 scale() const { return _scale; }

    virtual void setScale(DirectX::XMFLOAT3 scale);

    QBindable<DirectX::XMFLOAT3> bindableScale() { return &_scale; }

    QString name() { return _name; }

    void setName(QString name);

    QBindable<QString> bindableName() { return &_name; }

    virtual bool equals(const std::shared_ptr<Object> &other) const;

    virtual void draw(Renderer &renderer, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, DrawType drawType) = 0;

    virtual Type type() const = 0;

    virtual DirectX::BoundingOrientedBox boundingBox() const = 0;

private:
    DirectX::XMFLOAT4X4 model;

    void calculateModel();
};


#endif //MG1_OBJECT_H
