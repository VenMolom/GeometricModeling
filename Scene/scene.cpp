//
// Created by Molom on 2022-02-27.
//

#include "scene.h"

using namespace DirectX;
using namespace std;

void Scene::draw(Renderer &renderer) const {
    for (auto &object: objects) {
        object->draw(renderer, _camera);
    }
    if (cursor) {
        cursor->draw(renderer, _camera);
    }
}

void Scene::addObject(shared_ptr<Object> &&object) {
    if (cursor) {
        object->setPosition(cursor->position());
        cursor.reset();
    }

    _selected = object;
    objects.push_back(std::move(object));
}

void Scene::addCursor(QPoint screenPosition) {
    auto screenPos = XMINT2(screenPosition.x(), screenPosition.y());
    auto screenSize = XMFLOAT2(_camera.viewport().width(), _camera.viewport().height());
    auto ray = Utils3D::getRayFromScreen(screenPos, screenSize, _camera.nearZ(), _camera.farZ(),
                                         _camera.projectionMatrix(), _camera.viewMatrix());

    auto plane = Utils3D::getPerpendicularPlaneThroughPoint(_camera.direction(), _camera.center());
    auto position = Utils3D::getRayCrossWithPlane(ray, plane);

    cursor = make_shared<Cursor>(position, screenPos, _camera);
    _selected = cursor;
}
