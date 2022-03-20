//
// Created by Molom on 2022-03-19.
//

#ifndef MG1_COMPOSITEOBJECT_H
#define MG1_COMPOSITEOBJECT_H

#include "Objects/Object/object.h"

class CompositeObject : public Object{
public:
    CompositeObject(std::list<std::shared_ptr<Object>> &&objects);

    void draw(Renderer &renderer, const Camera &camera, DrawType drawType) const override;

    Type type() const override;

    DirectX::BoundingOrientedBox boundingBox() const override;

private:
    std::list<std::shared_ptr<Object>> objects;

    void calculateCenter();
};


#endif //MG1_COMPOSITEOBJECT_H
