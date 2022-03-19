//
// Created by Molom on 2022-03-19.
//

#include "objectListItem.h"

ObjectListItem::ObjectListItem(std::shared_ptr<Object> object, std::shared_ptr<Scene> scene)
    : object(object), scene(scene) {
    updateText();
    nameHandler = object->bindableName().addNotifier([&] { updateText(); });
}

void ObjectListItem::select() {
    scene->setSelected(object);
}

void ObjectListItem::remove() {
    scene->removeObject(object);
}

ObjectListItem::~ObjectListItem() {
    remove();
}

void ObjectListItem::updateText() {
    setText(object->name());
}