//
// Created by Molom on 28/04/2022.
//

#ifndef MG1_SCREENTRANSFORM_H
#define MG1_SCREENTRANSFORM_H

#include "Objects/object.h"
#include "Utils/utils3D.h"
#include "camera.h"

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

    ScreenTransform(std::shared_ptr<Object> object, std::shared_ptr<Camera> camera, Transform mode, Axis axis);

    void transform(QPoint screenPosition, QPointF delta);

    void changeMode(Transform mode) { this->mode = mode; }

    void changeLockAxis(Axis axis) { this->axis = axis;}

private:
    std::shared_ptr<Object> object;
    std::shared_ptr<Camera> camera;

    Transform mode;
    Axis axis;

    bool movable{false};
    bool transformable{false};

    DirectX::XMFLOAT3 startScale;
    DirectX::XMFLOAT3 startRotation;

    void move(QPoint screenPosition);

    void rotate(QPointF delta);

    void scale(QPointF delta);
};


#endif //MG1_SCREENTRANSFORM_H
