//
// Created by Molom on 2022-02-27.
//

#ifndef MG1_CAMERA_H
#define MG1_CAMERA_H


#include <QMatrix4x4>

#define STEP 1.2
#define SENSITIVITY 0.005f
#define SPEED 0.01f
#define LIMIT M_PI_2 - 0.1f

class Camera {
    QMatrix4x4 perspective;
    QMatrix4x4 view;

    QVector3D position;
    QVector3D center;
    QVector3D front;
    QVector3D up;
    QVector3D worldUp;
    QVector3D right;

    QRectF viewPlane;
    float viewDepth;
    float distance;
    float zoom;
    float yaw;
    float pitch;

public:
    Camera();

    Camera(Camera &&camera) = default;

    [[nodiscard]] QMatrix4x4 viewMatrix() const;

    void resize(QSizeF newSize);

    void changeZoom(float delta, QSizeF viewportSize);

    [[nodiscard]] QVector3D getPosition() const { return position; }

    void rotate(QPointF angle);

    void move(QPointF direction);
};


#endif //MG1_CAMERA_H
