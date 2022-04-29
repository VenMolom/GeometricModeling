//
// Created by Molom on 2022-02-27.
//

#include "scene.h"

using namespace DirectX;
using namespace Utils3D;
using namespace std;

Scene::Scene()
        : grid(500), virtualPointsHolders(), _objects(), _selected(), factory() {
    _camera = make_shared<Camera>();
    _camera->rotate({0, 200.0f});
}

void Scene::draw(Renderer &renderer) {
    grid.draw(renderer, DEFAULT);
    if (composite) {
        composite->draw(renderer, SELECTED);
    }
    if (cursor) {
        cursor->draw(renderer, DEFAULT);
    }
    for (auto &object: _objects) {
        auto selected = _selected.value().lock().get();
        object->draw(renderer, selected == object.get() ? SELECTED : DEFAULT);
    }
}

void Scene::addObject(shared_ptr<Object> &&object, bool overrideCursor) {
    if (!overrideCursor && cursor) {
        object->setPosition(cursor->position());
        cursor.reset();
    }
    boolean select = true;

    // add point to selected curve
    shared_ptr<Object> sel;
    if ((sel = _selected.value().lock()) && sel->type() & CURVE && object->type() & POINT3D) {
        auto *c = dynamic_cast<Curve *>(sel.get());
        shared_ptr<Point> p = static_pointer_cast<Point>(object);
        c->addPoint(p);
        select = false;
    }

    if (select) {
        _selected = object;
    }

    if (object->type() & VIRTUALPOINTSHOLDER) {
        virtualPointsHolders.push_back(dynamic_pointer_cast<VirtualPointsHolder>(object));
    }

    _objects.push_back(std::move(object));
    emit objectAdded(_objects.back(), select);
}

void Scene::addComposite(list<shared_ptr<Object>> &&objects) {
    _objects.remove_if([&objects](const shared_ptr<Object> &ob) {
        return find_if(objects.begin(), objects.end(),
                       [&ob](const shared_ptr<Object> &obb) { return ob->equals(obb); }) != objects.end();
    });

    auto comp = make_shared<CompositeObject>(std::move(objects));
    setSelected(comp);
}

void Scene::removeSelected() {
    if (auto selected = _selected.value().lock()) {
        _objects.remove_if([&selected](const shared_ptr<Object> &ob) { return selected->equals(ob); });
        composite.reset();
        _selected.setValue({});
    }
}

void Scene::addPoint(QPoint screenPosition) {
    auto screenPos = XMINT2(screenPosition.x(), screenPosition.y());
    auto position = getPositionOnPlane(screenPos, _camera->direction(), _camera->center());
    addObject(factory.createPoint(position), true);
}

void Scene::centerSelected() {
    if (auto selected = _selected.value().lock()) {
        _camera->moveTo(selected->position());
    }
}

void Scene::selectOrAddCursor(QPoint screenPosition, bool multiple) {
    auto screenPos = XMINT2(screenPosition.x(), screenPosition.y());
    auto ray = getRayFromScreenPosition(screenPos);

    if (auto object = findIntersectingObject(ray)) {
        shared_ptr<Object> sel;

        if (multiple && (sel = _selected.value().lock()) && object->type() & COMPOSABLE) {
            if (sel->type() & CURVE && object->type() & POINT3D) {
                auto *c = dynamic_cast<Curve *>(sel.get());
                shared_ptr<Point> p = static_pointer_cast<Point>(object);
                c->addPoint(p);
            } else if (sel->type() & COMPOSABLE && sel.get() != object.get()) {
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
            } else {
                setSelected(object);
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

    if (object->type() & COMPOSITE) {
        composite = std::move(object);
        _selected = composite;
        cursor.reset();
    } else if ((composite && composite->equals(object))
               || object->type() & VIRTUALPOINT3D
               || find_if(_objects.begin(), _objects.end(),
                          [&object](const shared_ptr<Object> &ob) { return object->equals(ob); }) != _objects.end()) {
        _selected = object;
        removeComposite();
        cursor.reset();
    }
}

Utils3D::XMFLOAT3RAY Scene::getRayFromScreenPosition(XMINT2 screenPosition) const {
    return getRayFromScreen(screenPosition, _camera);
}

shared_ptr<Object> Scene::findIntersectingObject(XMFLOAT3RAY ray) {
    shared_ptr<Object> closest{nullptr};
    float closestDistance = INFINITY;

    for (auto &object: _objects) {
        float distance{};
        if (object->intersects(ray.position, ray.direction, _camera->viewMatrix(),
                               _camera->nearZ(), _camera->farZ(), distance)
            && distance >= 0 && distance < closestDistance) {
            closest = object;
            closestDistance = distance;
        }
    }

    virtualPointsHolders.remove_if([](weak_ptr<VirtualPointsHolder> &holder) { return holder.expired(); });
    for (auto &holder: virtualPointsHolders) {
        auto points = holder.lock()->virtualPoints();
        for (auto &point: points) {
            float distance{};
            if (point->intersects(ray.position, ray.direction, _camera->cameraMatrix(),
                                  _camera->nearZ(), _camera->farZ(), distance) &&
                distance < closestDistance) {
                closest = point;
                closestDistance = distance;
            }
        }
    }

    return closest;
}

XMFLOAT3 Scene::getPositionOnPlane(XMINT2 screenPosition, XMFLOAT3 normal, XMFLOAT3 point) const {
    auto ray = getRayFromScreenPosition(screenPosition);
    auto plane = getPerpendicularPlaneThroughPoint(normal, point);
    return getRayCrossWithPlane(ray, plane);
}

void Scene::addCursor(XMFLOAT3RAY ray, XMINT2 screenPos) {
    auto plane = getPerpendicularPlaneThroughPoint(_camera->direction(), _camera->center());
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
