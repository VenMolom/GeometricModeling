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

    void draw(Renderer &renderer, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, DrawType drawType) override;

    boolean drawBSplinePolygonal() { return bSplinePolygonal; }

    void setDrawBSplinePolygonal(bool draw) { bSplinePolygonal = draw; }

    Type type() const override;

protected:
    void preUpdate() override;

    void pointUpdate(const std::shared_ptr<Point> &point, int index) override;

    void postUpdate() override;

private:
    std::vector<VertexPositionColor> bSplineVertices;
    bool bSplinePolygonal{false};
};


#endif //MG1_BREZIERC2_H
