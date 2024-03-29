//
// Created by Molom on 2022-02-27.
//

#ifndef MG1_OBJECT_H
#define MG1_OBJECT_H

#define NOMINMAX

#include <d3d11.h>
#include <qstring.h>
#include <utility>
#include <QProperty>
#include <DirectXCollision.h>
#include <DirectXMath.h>
#include "DirectX/renderer.h"
#include "Objects/renderable.h"

enum Type {
    CURSOR = 1 << 0,
    POINT3D = 1 << 1,
    TORUS = 1 << 2,
    COMPOSITE = 1 << 3,
    BREZIERC0 = 1 << 4,
    BREZIERC2 = 1 << 5,
    VIRTUALPOINT3D = 1 << 6,
    GRID = 1 << 7,
    INTERPOLATIONC2 = 1 << 8,
    PATCHC0 = 1 << 9,
    PATCHC0CREATOR = 1 << 10,
    COMPOSABLEVIRTUALPOINT3D = 1 << 11,
    PATCHC2 = 1 << 12,
    PATCHC2CREATOR = 1 << 13,
    GREGORY = 1 << 14,
    INTERSECTION = 1 << 15,
    CNCROUTER = 1 << 16,
    CURVE = BREZIERC0 | BREZIERC2 | INTERPOLATIONC2,
    PATCH = PATCHC0 | PATCHC2 | GREGORY,
    PATCHCREATOR = PATCHC0CREATOR | PATCHC2CREATOR,
    MOVABLE = POINT3D | TORUS | COMPOSITE | CURSOR | VIRTUALPOINT3D | PATCHC0 | PATCHC2
            | PATCHCREATOR | COMPOSABLEVIRTUALPOINT3D | CNCROUTER,
    NAMEABLE = POINT3D | TORUS | CURVE | PATCH | PATCHCREATOR | INTERSECTION | CNCROUTER,
    TRANSFORMABLE = TORUS | COMPOSITE | PATCHC0 | PATCHC2 | PATCHCREATOR,
    SCREENMOVABLE = CURSOR,
    VIRTUALPOINTSHOLDER = BREZIERC2 | PATCHC0 | PATCHC2,
    COMPOSABLE = POINT3D | TORUS | COMPOSITE | COMPOSABLEVIRTUALPOINT3D | PATCHC0 | PATCHC2,
    VIRTUAL = CURSOR | VIRTUALPOINT3D | COMPOSABLEVIRTUALPOINT3D | PATCHCREATOR,
    PARAMETRIC = TORUS | PATCH | PATCHCREATOR,
    SCREENSELECTABLE = POINT3D | VIRTUALPOINT3D | TORUS | COMPOSITE | COMPOSABLEVIRTUALPOINT3D,
    UPDATABLE = CNCROUTER,
};

inline Type operator|(Type a, Type b)
{
    return static_cast<Type>(static_cast<int>(a) | static_cast<int>(b));
}

inline Type operator&(Type a, Type b)
{
    return static_cast<Type>(static_cast<int>(a) & static_cast<int>(b));
}

enum
DrawType {
    SELECTED,
    NO_CURSOR,
    DEFAULT
};

class Object : public Renderable {
public:
    bool noDepth{false};

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

    QString name() const { return _name; }

    QString nameId() const;

    void setName(QString name);

    uint id() const { return _id; }

    QBindable<QString> bindableName() { return &_name; }

    virtual bool equals(const std::shared_ptr<Object> &other) const;

    virtual void draw(Renderer &renderer, DrawType drawType) = 0;

    virtual Type type() const = 0;

    virtual bool intersects(DirectX::XMFLOAT3 origin, DirectX::XMFLOAT3 direction, DirectX::XMMATRIX viewMatrix,
                            float nearZ, float farZ, float &distance) const { return false; };

protected:
    QProperty<DirectX::XMFLOAT3> _position;
    QProperty<DirectX::XMFLOAT3> _rotation{{0, 0, 0}};
    QProperty<DirectX::XMFLOAT3> _scale{{1, 1, 1}};

    DirectX::XMFLOAT4X4 noScaleMatrix;
    DirectX::XMFLOAT4X4 rotationMatrix;
    DirectX::XMFLOAT4X4 scaleMatrix;

    Object(uint id, QString name, DirectX::XMFLOAT3 position, D3D11_PRIMITIVE_TOPOLOGY topology);

    void calculateModel();
private:
    DirectX::XMFLOAT4X4 model;
    QProperty<QString> _name;
    uint _id;
};

#endif //MG1_OB