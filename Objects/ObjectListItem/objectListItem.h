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

    ~ObjectListItem();

    void select();

    void remove();

    bool hasObject(std::shared_ptr<Object> object) { return this->object.get() == object.get(); }
private:
    QPropertyNotifier nameHandler;

    std::shared_ptr<Object> object;
    std::shared_ptr<Scene> scene;

    void updateText();
};


#endif //MG1_OBJECTLISTITEM_H
