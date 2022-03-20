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

    void addComposite(std::list<std::shared_ptr<Object>>&& objects);

    void removeSelected();

    void selectOrAddCursor(QPoint screenPosition, bool multiple);

    void draw(Renderer &renderer) const;

    Camera &camera() { return _camera; }

    std::weak_ptr<Object> selected() const { return _selected; }

    void setSelected (std::shared_ptr<Object> object);

    QBindable<std::weak_ptr<Object>> bindableSelected() { return &_selected; }

private:
    QProperty<std::weak_ptr<Object>> _selected;
    std::list<std::shared_ptr<Object>> _objects;
    std::shared_ptr<Cursor> cursor;
    std::shared_ptr<Object> composite;
    // TODO: look into removing composite to reduce complexity
    Camera _camera;

    std::shared_ptr<Object> findIntersectingObject(Utils3D::XMFLOAT3RAY ray);

    void addCursor(Utils3D::XMFLOAT3RAY ray, DirectX::XMINT2 screenPos);

    void removeComposite();
};


#endif //MG1_SCENE_H
