//
// Created by Molom on 2022-03-19.
//

#include "objectListItem.h"

#include <utility>

ObjectListItem::ObjectListItem(std::shared_ptr<Object> object, std::shared_ptr<Scene> scene)
    : _object(std::move(object)), scene(std::move(scene)) {
    updateText();
    nameHandler = _object->bindableName().addNotifier([this] { updateText(); });
}

void ObjectListItem::select() {
    scene->setSelected(_object);
}

void ObjectListItem::updateText() {
    setText(_object->nameId());
}