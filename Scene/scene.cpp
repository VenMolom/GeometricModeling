//
// Created by Molom on 2022-02-27.
//

#include "scene.h"

void Scene::draw(Renderer &renderer) const {
    for (auto &object: objects) {
        object->draw(renderer, camera.viewMatrix());
    }
}

void Scene::addObject(std::unique_ptr<Object> &&object) {
    objects.push_back(std::move(object));
}
