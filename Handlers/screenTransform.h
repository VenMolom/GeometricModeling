//
// Created by Molom on 28/04/2022.
//

#ifndef MG1_SCREENTRANSFORM_H
#define MG1_SCREENTRANSFORM_H

#include "Objects/object.h"
#include "Utils/utils3D.h"
#include "camera.h"

#define SCALE_MODIFIER 0.5f

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

    ScreenTransform(std::shared_ptr<Object> object, QPoint screenPosition,
                    std::shared_ptr<Camera> camera, Transform mode, Axis axis);

    void transform(QPoint screenPosition);

    void setAxis(Axis axis) { this->axis = axis; }

private:
    std::shared_ptr<Object> object;
    std::shared_ptr<Camera> camera;

    Transform mode;
    Axis axis;

    bool movable{false};
    bool transformable{false};

    DirectX::XMFLOAT3 startScale;
    DirectX::XMFLOAT3 startRotation;
    DirectX::XMFLOAT3 startingScenePosition;
    QPoint startingScreenPosition;

    void move(QPoint screenPosition);

    void rotate(QPoint screenPosition);

    void scale(QPoint screenPosition);

    DirectX::XMFLOAT3 getPositionFromScreen(QPoint screenPosition);

    // TODO: ? implement axis lock
};


#endif //MG1_SCREENTRANSFORM_H
