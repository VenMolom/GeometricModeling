//
// Created by Molom on 2022-02-27.
//

#ifndef MG1_ELLIPSOID_H
#define MG1_ELLIPSOID_H


#include "Object/object.h"

class Ellipsoid : public Object {
    QVector3D radius;
    QMatrix4x4 implicitMatrix;

public:
    Ellipsoid();

    Ellipsoid(Ellipsoid &ellipsoid) = default;

    Ellipsoid(Ellipsoid &&ellipsoid) = default;

    void draw(QImage &target, const QMatrix4x4 &camera, const QVector3D &lightPosition) const override;

    QVector4D getNormal(QVector4D &worldPosition) const override;

    void setRadius(QVector3D &radius);

    void setRadiusX(float x);

    void setRadiusY(float y);

    void setRadiusZ(float z);

private:
    [[nodiscard]] QMatrix4x4 ellipsoidMatrix() const { return implicitMatrix; };

    static float solveForPoint(const QVector2D &point, const QMatrix4x4 &scene);

    void recalculateMatrix();
};


#endif //MG1_ELLIPSOID_H
