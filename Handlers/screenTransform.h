//
// Created by Molom on 28/04/2022.
//

#ifndef MG1_SCREENTRANSFORM_H
#define MG1_SCREENTRANSFORM_H

#include <DirectXMath.h>
#include "Objects/object.h"
#include "Utils/utils3D.h"

class ScreenTransform {
public:
    enum Transform {
        NONE,
        MOVE,
        ROTATE,
        SCALE
    };

    enum Axis {
        FREE,
        X,
        Y,
        Z,
    };

    ScreenTransform(std::shared_ptr<Object> object);

    ~ScreenTransform();

    void release();

    void transform(QPoint screenPosition, QPointF move);

    void changeMode(Transform mode);

    void changeLockAxis(Axis axis);

//    void Scene::moveSelected(QPoint screenPosition) {
//        shared_ptr<Object> selected;
//        if (!(selected = _selected.value().lock()) || !selected->type() & MOVEABLE) return;
//
//        auto screenPos = XMINT2(screenPosition.x(), screenPosition.y());
//        auto position = getPositionOnPlane(screenPos, _camera->direction(), selected->position());
//
//        selected->setPosition(position);
//    }
}


#endif //MG1_SCREENTRANSFORM_H
