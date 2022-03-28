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

    bool contains(const std::shared_ptr<Object> &object) const;

    std::list<std::shared_ptr<Object>> &&release();

    void draw(Renderer &renderer, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, DrawType drawType) override;

    Type type() const override;

    DirectX::BoundingOrientedBox boundingBox() const override;

    void setPosition(DirectX::XMFLOAT3 position) override;

    void setRotation(DirectX::XMFLOAT3 rotation) override;

    void setScale(DirectX::XMFLOAT3 scale) override;

    bool equals(const std::shared_ptr<Object> &other) const override;

private:
    std::list<std::shared_ptr<Object>> objects;
    std::vector<DirectX::XMFLOAT4X4> startingMatrices{};
    DirectX::XMFLOAT3 startingPosition;

    void calculateCenter();

    void updateMatrix();

    void updateChildren();

    static DirectX::XMFLOAT3 rotationMatrixToEuler(DirectX::XMMATRIX rotationMatrix) ;
};


#endif //MG1_COMPOSITEOBJECT_H