//
// Created by Molom on 2022-02-27.
//

#ifndef MG1_SCENE_H
#define MG1_SCENE_H

#include <list>
#include "Camera/camera.h"
#include "Objects/Cursor/cursor.h"

class Scene {
public:
    explicit Scene() = default;

    void addObject(std::shared_ptr<Object> &&object);

    void draw(Renderer &renderer) const;

    Camera &camera() { return _camera; }

    std::weak_ptr<Object> selected() const { return _selected; }
    QBindable<std::weak_ptr<Object>> bindableSelected() { return &_selected; }

    void addCursor(QPoint screenPosition);

private:
    std::vector<std::shared_ptr<Object>> objects;
    std::shared_ptr<Cursor> cursor;
    QProperty<std::weak_ptr<Object>> _selected;
    Camera _camera;
};


#endif //MG1_SCENE_H
