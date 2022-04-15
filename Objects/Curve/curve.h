//
// Created by Molom on 2022-04-15.
//

#ifndef MG1_CURVE_H
#define MG1_CURVE_H

#include <DirectXMath.h>
#include "Objects/object.h"
#include "Objects/Point/point.h"

enum Direction {
    UP,
    DOWN
};

class Curve : public Object {
public:
    void addPoint(std::weak_ptr<Point> point);

    void movePoint(int index, Direction direction);

    void removePoint(int index);

    void draw(Renderer &renderer, DrawType drawType) override;

    const std::vector<std::weak_ptr<Point>> &points() const { return _points; }

    QBindable<int> bindablePoints() { return &pointsChanged; }

    boolean polygonal() const { return _polygonal; }

    void setPolygonal(bool draw) { _polygonal = draw; }

    DirectX::BoundingOrientedBox boundingBox() const override;

    void setPosition(DirectX::XMFLOAT3 position) final {};

    void setRotation(DirectX::XMFLOAT3 rotation) final {};

    void setScale(DirectX::XMFLOAT3 scale) final {};

    DirectX::XMVECTOR maxPosition() const { return DirectX::XMLoadFloat3(&maxPos); }

    DirectX::XMVECTOR minPosition() const { return DirectX::XMLoadFloat3(&minPos); }
protected:
    std::vector<std::weak_ptr<Point>> _points;
    std::vector<QPropertyNotifier> pointsHandlers{};

    DirectX::XMFLOAT3 minPos{INFINITY, INFINITY, INFINITY};
    DirectX::XMFLOAT3 maxPos{-INFINITY, -INFINITY, -INFINITY};

    Curve(QString name, std::vector<std::weak_ptr<Point>> &&points, D3D11_PRIMITIVE_TOPOLOGY topology);

    virtual void drawPolygonal(Renderer &renderer, DrawType drawType) = 0;

    virtual void drawCurve(Renderer &renderer, DrawType drawType) = 0;

    void updatePoints();

    virtual void preUpdate();

    virtual void pointUpdate(const std::shared_ptr<Point> &point, int index) = 0;

    virtual void postUpdate();

    virtual void pointMoved(const std::weak_ptr<Point> &point) = 0;

private:
    QProperty<int> pointsChanged{};
    bool _polygonal{false};
};


#endif //MG1_CURVE_H
