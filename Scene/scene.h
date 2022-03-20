//
// Created by Molom on 2022-02-27.
//

#ifndef MG1_SCENE_H
#define MG1_SCENE_H

#include <list>
#include "Objects/Cursor/cursor.h"
#include "Objects/CompositeObject/compositeObject.h"

class Scene {
public:
    explicit Scene() = default;

    void addObject(std::shared_ptr<Object> &&object);

    void removeObject(const std::shared_ptr<Object>& object);

    void selectOrAddCursor(QPoint screenPosition, bool multiple);

    void draw(Renderer &renderer) const;

    Camera &camera() { return _camera; }

    std::weak_ptr<Object> selected() const { return _selected; }

    void setSelected (std::shared_ptr<Object> object);

    QBindable<std::weak_ptr<Object>> bindableSelected() { return &_selected; }

private:
    QProperty<std::weak_ptr<Object>> _selected;
    std::list<std::shared_ptr<Object>> objects;
    std::shared_ptr<Cursor> cursor;
    std::shared_ptr<Object> composite;
    Camera _camera;

    std::shared_ptr<Object> findIntersectingObject(Utils3D::XMFLOAT3RAY ray);

    void addCursor(Utils3D::XMFLOAT3RAY ray, DirectX::XMINT2 screenPos);
};


#endif //MG1_SCENE_H
