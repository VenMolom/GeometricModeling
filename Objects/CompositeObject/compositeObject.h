//
// Created by Molom on 2022-03-19.
//

#ifndef MG1_COMPOSITEOBJECT_H
#define MG1_COMPOSITEOBJECT_H

#include <DirectXMath.h>
#include "Objects/object.h"
#include "Objects/Point/composableVirtualPoint.h"
#include "Objects/Patch/gregoryPatch.h"
#include "Utils/gregoryUtils.h"

class CompositeObject : public Object{
public:
    explicit CompositeObject(std::list<std::shared_ptr<Object>> &&objects);

    ~CompositeObject();

    bool contains(const std::shared_ptr<Object> &object) const;

    std::list<std::shared_ptr<Object>> &&release();

    void draw(Renderer &renderer, DrawType drawType) override;

    Type type() const override;

    void setPosition(DirectX::XMFLOAT3 position) override;

    void setRotation(DirectX::XMFLOAT3 rotation) override;

    void setScale(DirectX::XMFLOAT3 scale) override;

    bool equals(const std::shared_ptr<Object> &other) const override;

    bool collapsable() const { return _collapsable; }

    bool fillable() const { return _fillable; }

    bool intersectable() const { return _intersectable; }

    bool empty() const { return objects.empty(); }

    std::shared_ptr<VirtualPoint> collapse();

    std::shared_ptr<GregoryPatch> fillIn(uint id);

private:
    std::list<std::shared_ptr<Object>> objects;
    std::vector<DirectX::XMFLOAT4X4> startingMatrices{};
    DirectX::XMFLOAT3 startingPosition;
    bool _collapsable = false;
    bool _fillable = false;
    bool _intersectable = false;
    GregoryUtils::GregoryInfo fillInInfo;

    void calculateCenter();

    void updateChildren();

    static DirectX::XMFLOAT3 rotationMatrixToEuler(DirectX::XMMATRIX rotationMatrix) ;
};


#endif //MG1_COMPOSITEOBJECT_H
