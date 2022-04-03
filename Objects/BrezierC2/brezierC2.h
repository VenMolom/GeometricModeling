//
// Created by Molom on 2022-04-01.
//

#ifndef MG1_BREZIERC2_H
#define MG1_BREZIERC2_H

#include <DirectXMath.h>
#include "Objects/BrezierCurve/brezierCurve.h"

class BrezierC2 : public BrezierCurve {
public:
    explicit BrezierC2(std::vector<std::weak_ptr<Point>> &&points);

    boolean bothPolygonals() const { return _bothPolygonals; }

    void setBothPolygonals(bool both) { _bothPolygonals = both; }

    boolean bernsteinBase() const { return _bernsteinBase; }

    void setBernsteinBase(bool use) { _bernsteinBase = use; }

    Type type() const override;

protected:
    void drawPolygonal(Renderer &renderer, DirectX::XMMATRIX mvp, DrawType drawType) override;

    void preUpdate() override;

    void pointUpdate(const std::shared_ptr<Point> &point, int index) override;

    void postUpdate() override;

    void pointMoved(const std::weak_ptr<Point> &point) override;

private:
    std::vector<VertexPositionColor> bSplineVertices;
    bool _bothPolygonals{false};
    bool _bernsteinBase{false};
};


#endif //MG1_BREZIERC2_H
