//
// Created by Molom on 2022-02-27.
//

#include "scene.h"
#include "Utils/Utils3D.h"

using namespace DirectX;

void Scene::draw(Renderer &renderer) const {
    for (auto &object: objects) {
        object->draw(renderer, _camera.viewMatrix());
    }
}

void Scene::addObject(std::shared_ptr<Object> &&object) {
    _selected = object;
    objects.push_back(std::move(object));
}

void Scene::addCursor(QPoint screenPosition) {
    auto position = XMFLOAT2(screenPosition.x(), screenPosition.y());
    auto ray = Utils3D::getRayFromScreen(position, )

}
