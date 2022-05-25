//
// Created by Molom on 2022-04-01.
//

#ifndef MG1_BREZIERC2_H
#define MG1_BREZIERC2_H

#include <DirectXMath.h>
#include <Models/Curves/BezierC2.h>
#include "Utils/utils3D.h"
#include "brezierCurve.h"
#include "Objects/Point/virtualPoint.h"
#include "Objects/linestrip.h"
#include "Objects/Point/virtualPointsHolder.h"

class BrezierC2 : public BrezierCurve, public VirtualPointsHolder {
public:
    BrezierC2(uint id, std::vector<std::weak_ptr<Point>> &&points, QBindable<std::weak_ptr<Object>> bindableSelected);

    BrezierC2(const MG1::Bezier &curve, const std::list<std::shared_ptr<Object>> &sceneObjects,
              QBindable<std::weak_ptr<Object>> bindableSelected);

    boolean bothPolygonals() const { return _bothPolygonals; }

    void setBothPolygonals(bool both) { _bothPolygonals = both; }

    boolean bernsteinBase() const { return _bernsteinBase; }

    void setBernsteinBase(bool use) { _bernsteinBase = use; }

    Type type() const override;

    const std::vector<std::shared_ptr<VirtualPoint>> &virtualPoints() override;

    MG1::BezierC2 serialize();

protected:
public:
    void draw(Renderer &renderer, DrawType drawType) override;

protected:

    void drawPolygonal(Renderer &renderer, DrawType drawType) override;

    void preUpdate() override;

    void pointUpdate(const std::shared_ptr<Point> &point, int index) override;

    void postUpdate() override;

    void pointMoved(const std::weak_ptr<Point> &point) override;

private:
    std::vector<std::shared_ptr<VirtualPoint>> bernsteinPoints;
    Linestrip deBoorPoints;
    std::vector<QPropertyNotifier> bernsteinPointsHandlers{};
    bool _bothPolygonals{false};
    bool _bernsteinBase{false};

    void synchroniseBernsteinPositions(int start, int end);

    void addBernsteinPoint(const DirectX::XMFLOAT3 &position);

    void bernsteinMoved(const std::weak_ptr<VirtualPoint> &point);

    void deBoorMoved(int index, const std::shared_ptr<Point> &moved);
};


#endif //MG1_BREZIERC2_H
