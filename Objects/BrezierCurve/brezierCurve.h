//
// Created by Molom on 31/03/2022.
//

#ifndef MG1_BREZIERCURVE_H
#define MG1_BREZIERCURVE_H

#include <DirectXMath.h>
#include "Objects/Object/object.h"
#include "Objects/Point/point.h"

enum Direction {
    UP,
    DOWN
};

class BrezierCurve : public Object {
public:
    explicit BrezierCurve(QString name, std::vector<std::weak_ptr<Point>> &&points);

    void addPoint(std::weak_ptr<Point> point);

    void movePoint(int index, Direction direction);

    void removePoint(int index);

    std::vector<std::weak_ptr<Point>> points() const { return _points; }

    QBindable<int> bindablePoints() { return &pointsChanged; }

    boolean drawPolygonal() { return polygonal; }

    void setDrawPolygonal(bool draw) { polygonal = draw; }

    void draw(Renderer &renderer, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, DrawType drawType) override;

    DirectX::BoundingOrientedBox boundingBox() const override;

    void setPosition(DirectX::XMFLOAT3 position) override {};

    void setRotation(DirectX::XMFLOAT3 rotation) override {};

    void setScale(DirectX::XMFLOAT3 scale) override {};
protected:
    std::vector<std::weak_ptr<Point>> _points;
    std::vector<QPropertyNotifier> pointsHandlers{};
    QProperty<int> pointsChanged{};
    bool polygonal{false};

    std::vector<VertexPositionColor> vertices;
    std::vector<Index> indices;
    DirectX::XMFLOAT3 min{INFINITY, INFINITY, INFINITY};
    DirectX::XMFLOAT3 max{-INFINITY, -INFINITY, -INFINITY};
    int lastPatchSize;

    void updatePoints();

    void virtual preUpdate();

    void virtual pointUpdate(const std::shared_ptr<Point> &point, int index);

    void virtual postUpdate();

    static DirectX::XMFLOAT3 newMin(DirectX::XMFLOAT3 oldMin, DirectX::XMFLOAT3 candidate);

    static DirectX::XMFLOAT3 newMax(DirectX::XMFLOAT3 oldMax, DirectX::XMFLOAT3 candidate);
};


#endif //MG1_BREZIERCURVE_H
