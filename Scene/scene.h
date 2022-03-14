//
// Created by Molom on 2022-02-27.
//

#ifndef MG1_SCENE_H
#define MG1_SCENE_H

#include <list>
#include "Objects/Object/object.h"
#include "Camera/camera.h"

class Scene {
public:
    explicit Scene() = default;

    void addObject(std::shared_ptr<Object> &&object);

    void draw(Renderer &renderer) const;

    Camera &camera() { return _camera; }

    std::weak_ptr<Object> selectedObject() { return objects[0]; }

private:
    std::vector<std::shared_ptr<Object>> objects;
    Camera _camera;
};


#endif //MG1_SCENE_H
