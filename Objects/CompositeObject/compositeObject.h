//
// Created by Molom on 2022-03-19.
//

#ifndef MG1_COMPOSITEOBJECT_H
#define MG1_COMPOSITEOBJECT_H

#include <DirectXMath.h>
#include "Objects/Object/object.h"

class CompositeObject : public Object{
public:
    explicit CompositeObject(std::list<std::shared_ptr<Object>> &&objects);

    ~CompositeObject();

    bool contains(std::shared_ptr<Object> object);

    std::list<std::shared_ptr<Object>> &&release();

    void draw(Renderer &renderer, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, DrawType drawType) const override;

    Type type() const override;

    DirectX::BoundingOrientedBox boundingBox() const override;

    void setPosition(DirectX::XMFLOAT3 position) override;

    void setRotation(DirectX::XMFLOAT3 rotation) override;

    void setScale(DirectX::XMFLOAT3 scale) override;

private:
    std::list<std::shared_ptr<Object>> objects;

    DirectX::XMFLOAT3 startingPosition;
    DirectX::XMFLOAT4X4 modifyMatrix;

    void calculateCenter();

    void updateMatrix();

    DirectX::XMFLOAT3 quaternionToEuler(DirectX::XMFLOAT4 quaternion);
};


#endif //MG1_COMPOSITEOBJECT_H
