//
// Created by Molom on 2022-02-27.
//

#include "ellipsoid.h"
#include <cmath>

Ellipsoid::Ellipsoid::Ellipsoid(QVector3D position, QVector3D radius, QColor color, int specular)
        : Object(position, color, specular),
          radius(radius),
          implicitMatrix(1 / powf(radius.x(), 2), 0, 0, 0,
                         0, 1 / powf(radius.y(), 2), 0, 0,
                         0, 0, 1 / powf(radius.z(), 2), 0,
                         0, 0, 0, -1) {
}

void Ellipsoid::draw(QImage &target, const QMatrix4x4 &camera, const QVector3D &lightPosition) const {
    auto diag = ellipsoidMatrix();
    auto scene = (camera * modelMatrix()).inverted();

    auto diagScene = scene.transposed() * diag * scene;

    auto size = target.size();
    for (int x = 0; x < size.width(); ++x) {
        for (int y = 0; y < size.height(); ++y) {
            auto pos = convertFromScreen(x, y, size);
            auto z = solveForPoint(pos, diagScene);
            if (z <= 1 && z >= -1) {
                auto worldPosition = camera.inverted() * QVector4D(pos.x(), pos.y(), z, 1);
                auto normal = getNormal(worldPosition);
                auto pixelColor = calculateColor(worldPosition.toVector3DAffine(), lightPosition,
                                                 normal.toVector3D());
                target.setPixelColor(x, y, pixelColor);
            }
        }
    }
}

float Ellipsoid::solveForPoint(const QVector2D &point, const QMatrix4x4 &scene) {
    QVector4D p(point.x(), point.y(), 0, 1);
    QVector4D v(0, 0, 1, 0);

    auto a = QVector4D::dotProduct(v, scene * v);
    auto b = 2 * QVector4D::dotProduct(v, scene * p);
    auto c = QVector4D::dotProduct(p, scene * p);

    auto deltaQuad = b * b - 4 * a * c;

    if (deltaQuad < 0) {
        return NAN;
    }

    auto delta = sqrtf(deltaQuad);
    auto z1 = (-b + delta) / (2 * a);
    auto z2 = (-b - delta) / (2 * a);

    return z1 < z2 && z1 >= -1 ? z1 : z2;
}

QVector4D Ellipsoid::getNormal(QVector4D &worldPosition) const {
    auto modelInverted = modelMatrix().inverted();
    auto implicit = modelInverted.transposed() * ellipsoidMatrix() * modelInverted;
    return (2 * implicit.transposed() * worldPosition);
}

void Ellipsoid::setRadius(QVector3D &radius) {
    this->radius = radius;
    recalculateMatrix();
}

void Ellipsoid::setRadiusX(float x) {
    this->radius.setX(x);
    recalculateMatrix();
}

void Ellipsoid::setRadiusY(float y) {
    this->radius.setY(y);
    recalculateMatrix();
}

void Ellipsoid::setRadiusZ(float z) {
    this->radius.setZ(z);
    recalculateMatrix();
}

void Ellipsoid::recalculateMatrix() {
    implicitMatrix = QMatrix4x4(1 / powf(radius.x(), 2), 0, 0, 0,
                                0, 1 / powf(radius.y(), 2), 0, 0,
                                0, 0, 1 / powf(radius.z(), 2), 0,
                                0, 0, 0, -1);
}
