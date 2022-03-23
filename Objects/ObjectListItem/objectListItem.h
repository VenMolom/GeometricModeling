//
// Created by Molom on 2022-03-19.
//

#ifndef MG1_OBJECTLISTITEM_H
#define MG1_OBJECTLISTITEM_H


#include <qlistwidget.h>
#include "Objects/Object/object.h"
#include "Scene/scene.h"

class ObjectListItem : public QListWidgetItem {
public:
    ObjectListItem(std::shared_ptr<Object> object, std::shared_ptr<Scene> scene);

    void select();

    bool hasObject(const std::shared_ptr<Object> &object) { return _object->equals(object); }

    std::shared_ptr<Object> object() const { return _object; }

private:
    QPropertyNotifier nameHandler;

    std::shared_ptr<Object> _object;
    std::shared_ptr<Scene> scene;

    void updateText();
};


#endif //MG1_OBJECTLISTITEM_H
