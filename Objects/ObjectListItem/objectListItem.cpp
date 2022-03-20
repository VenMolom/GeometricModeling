//
// Created by Molom on 2022-03-19.
//

#include "objectListItem.h"

ObjectListItem::ObjectListItem(std::shared_ptr<Object> object, std::shared_ptr<Scene> scene)
    : _object(object), scene(scene) {
    updateText();
    nameHandler = _object->bindableName().addNotifier([&] { updateText(); });
}

void ObjectListItem::select() {
    scene->setSelected(_object);
}

void ObjectListItem::updateText() {
    setText(_object->name());
}