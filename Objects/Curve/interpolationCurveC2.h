//
// Created by Molom on 2022-04-15.
//

#ifndef MG1_INTERPOLATIONCURVEC2_H
#define MG1_INTERPOLATIONCURVEC2_H

#include "curve.h"
#include "Objects/linestrip.h"

class InterpolationCurveC2 : public Curve {
public:
    explicit InterpolationCurveC2(std::vector<std::weak_ptr<Point>> &&points);

    Type type() const override;

protected:
    void drawPolygonal(Renderer &renderer, DrawType drawType) override;

    void drawCurve(Renderer &renderer, DrawType drawType) override;

    void preUpdate() override;

    void pointUpdate(const std::shared_ptr<Point> &point, int index) override;

    void postUpdate() override;

    void pointMoved(const std::weak_ptr<Point> &point) override;

private:
    std::vector<float> knotDistances;
    std::vector<float> alpha;
    std::vector<float> beta;
    std::vector<DirectX::XMFLOAT3> R;

    Linestrip controlPoints;

    void calculateControlPoints(const std::vector<DirectX::XMFLOAT3> &c, int index);
};


#endif //MG1_INTERPOLATIONCURVEC2_H
