//
// Created by Molom on 2022-03-25.
//

#ifndef MG1_BREZIERC0_H
#define MG1_BREZIERC0_H

#include <DirectXMath.h>
#include "Objects/Object/object.h"
#include "Objects/Point/point.h"

enum Direction {
    UP,
    DOWN
};

class BrezierC0 : public Object {
public:
    explicit BrezierC0(std::vector<std::weak_ptr<Point>> points);

    void addPoint(std::weak_ptr<Point> point);

    void movePoint(int index, Direction direction);

    void removePoint(int index);

    std::vector<std::weak_ptr<Point>> points() const { return _points; }

    QBindable<int> bindablePoints() { return &pointsChanged; }

    boolean drawPolygonal() { return polygonal; }

    void setDrawPolygonal(bool draw) { polygonal = draw; }

    void draw(Renderer &renderer, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, DrawType drawType) override;

    Type type() const override;

    DirectX::BoundingOrientedBox boundingBox() const override;

    void setPosition(DirectX::XMFLOAT3 position) override {};

    void setRotation(DirectX::XMFLOAT3 rotation) override {};

    void setScale(DirectX::XMFLOAT3 scale) override {};

private:
    std::vector<std::weak_ptr<Point>> _points;
    QProperty<int> pointsChanged{};
    bool polygonal{false};

    static DirectX::XMFLOAT3 newMin(DirectX::XMFLOAT3 oldMin, DirectX::XMFLOAT3 candidate);

    static DirectX::XMFLOAT3 newMax(DirectX::XMFLOAT3 oldMax, DirectX::XMFLOAT3 candidate);
};


#endif //MG1_BREZIERC0_H
