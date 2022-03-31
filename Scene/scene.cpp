//
// Created by Molom on 2022-02-27.
//

#include <DirectXMath.h>
#include "scene.h"

using namespace DirectX;
using namespace Utils3D;
using namespace std;

void Scene::draw(Renderer &renderer) const {
    if (composite) {
        composite->draw(renderer, _camera.viewMatrix(), _camera.projectionMatrix(), SELECTED);
    }
    if (cursor) {
        cursor->draw(renderer, _camera.viewMatrix(), _camera.projectionMatrix(), DEFAULT);
    }
    for (auto &object: _objects) {
        auto a = _selected.value().lock().get();
        object->draw(renderer, _camera.viewMatrix(), _camera.projectionMatrix(),
                     a == object.get() ? SELECTED : DEFAULT);
    }

}

void Scene::addObject(shared_ptr<Object> &&object, bool overrideCursor) {
    if (!overrideCursor && cursor) {
        object->setPosition(cursor->position());
        cursor.reset();
    }
    boolean select = true;

    // add point to selected curve
    if (_selected.value().lock() && _selected.value().lock()->type() & BREZIERCURVE && object->type() == POINT3D) {
        auto *c = dynamic_cast<BrezierCurve *>(_selected.value().lock().get());
        shared_ptr<Point> p = dynamic_pointer_cast<Point>(object);

        c->addPoint(p);
        select = false;
    }

    if (select) {
        _selected = object;
    }

    _objects.push_back(std::move(object));
    emit objectAdded(_objects.back(), select);
}

void Scene::addComposite(list<shared_ptr<Object>> &&objects) {
    _objects.remove_if([&](const shared_ptr<Object> &ob) {
        return find_if(objects.begin(), objects.end(),
                       [&](const shared_ptr<Object> &obb) { return ob->equals(obb); }) != objects.end();
    });

    auto comp = make_shared<CompositeObject>(std::move(objects));
    setSelected(comp);
}

void Scene::removeSelected() {
    if (auto selected = _selected.value().lock()) {
        _objects.remove_if([&](const shared_ptr<Object> &ob) { return selected->equals(ob); });
        composite.reset();
        _selected.setValue({});
    }
}

void Scene::moveSelected(QPoint screenPosition) {
    shared_ptr<Object> selected;
    if (!(selected = _selected.value().lock()) || selected->type() == COMPOSITE) return;

    auto screenPos = XMINT2(screenPosition.x(), screenPosition.y());
    auto position = getPositionOnPlane(screenPos, _camera.direction(), selected->position());

    selected->setPosition(position);
}

void Scene::addPoint(QPoint screenPosition) {
    auto screenPos = XMINT2(screenPosition.x(), screenPosition.y());
    auto position = getPositionOnPlane(screenPos, _camera.direction(), _camera.center());
    addObject(make_shared<Point>(position), true);
}

void Scene::centerSelected() {
    if (auto selected = _selected.value().lock()) {
        _camera.moveTo(selected->position());
    }
}

void Scene::selectOrAddCursor(QPoint screenPosition, bool multiple) {
    auto screenPos = XMINT2(screenPosition.x(), screenPosition.y());
    auto ray = getRayFromScreenPosition(screenPos);

    if (auto object = findIntersectingObject(ray)) {
        shared_ptr<Object> sel;

        if (multiple && (sel = _selected.value().lock())) {
            if (sel->type() & BREZIERCURVE && object->type() == POINT3D) {
                auto *c = dynamic_cast<BrezierCurve *>(sel.get());
                shared_ptr<Point> p = dynamic_pointer_cast<Point>(object);

                c->addPoint(p);
            } else if (sel->type() != CURSOR && sel.get() != object.get()) {
                if (composite) {
                    auto comp = dynamic_cast<CompositeObject *>(composite.get());

                    if (comp->contains(object)) return;

                    auto obs = comp->release();
                    obs.push_back(object);
                    addComposite(std::move(obs));
                } else {
                    list<shared_ptr<Object>> obs = {sel, object};
                    addComposite(std::move(obs));
                }
            }
        } else {
            setSelected(object);
        }
    } else {
        addCursor(ray, screenPos);
    }
}

void Scene::setSelected(std::shared_ptr<Object> object) {
    if (!object && cursor) {
        return;
    }

    if (!object) {
        removeComposite();
        _selected.setValue({});
        return;
    }

    if (object->type() == COMPOSITE) {
        composite = std::move(object);
        _selected = composite;
        cursor.reset();
    } else if ((composite && composite->equals(object))
               || find_if(_objects.begin(), _objects.end(),
                          [&](const shared_ptr<Object> &ob) { return object->equals(ob); }) != _objects.end()) {
        _selected = object;
        removeComposite();
        cursor.reset();
    }
}

Utils3D::XMFLOAT3RAY Scene::getRayFromScreenPosition(XMINT2 screenPosition) const {
    auto screenSize = XMFLOAT2(_camera.viewport().width(), _camera.viewport().height());
    return getRayFromScreen(screenPosition, screenSize, _camera.nearZ(), _camera.farZ(),
                            _camera.projectionMatrix(), _camera.viewMatrix());
}

shared_ptr<Object> Scene::findIntersectingObject(XMFLOAT3RAY ray) const {
    shared_ptr<Object> closest{nullptr};
    float closestDistance = INFINITY;

    for (auto &object: _objects) {
        float distance{};
        if (object->boundingBox().Intersects(XMLoadFloat3(&ray.position), XMLoadFloat3(&ray.direction), distance) &&
            distance < closestDistance) {
            closest = object;
            closestDistance = distance;
        }
    }

    return closest;
}

XMFLOAT3
Scene::getPositionOnPlane(DirectX::XMINT2 screenPosition, DirectX::XMFLOAT3 normal, DirectX::XMFLOAT3 point) const {
    auto ray = getRayFromScreenPosition(screenPosition);
    auto plane = Utils3D::getPerpendicularPlaneThroughPoint(normal, point);
    return Utils3D::getRayCrossWithPlane(ray, plane);
}

void Scene::addCursor(XMFLOAT3RAY ray, XMINT2 screenPos) {
    auto plane = getPerpendicularPlaneThroughPoint(_camera.direction(), _camera.center());
    auto position = getRayCrossWithPlane(ray, plane);

    removeComposite();
    if (cursor) {
        cursor->setPosition(position);
        cursor->setScreenPosition(screenPos);
    } else {
        cursor = make_shared<Cursor>(position, screenPos, _camera);
        _selected = cursor;
    }
}

void Scene::removeComposite() {
    if (!composite) return;

    auto comp = dynamic_cast<CompositeObject *>(composite.get());
    _objects.splice(_objects.end(), comp->release());
    composite.reset();
}
