//
// Created by Molom on 31/03/2022.
//

#ifndef MG1_BREZIERCURVE_H
#define MG1_BREZIERCURVE_H

#include <DirectXMath.h>
#include "Objects/Curve/curve.h"



class BrezierCurve : public Curve {
public:
    int patchesCount() const { return static_cast<int>(indices.size() + 3) / 4; }

    int lastPatchSize() const { return _lastPatchSize; }
protected:
    int _lastPatchSize{0};
    bool canDraw{false};

    BrezierCurve(QString name, std::vector<std::weak_ptr<Point>> &&points);

    void drawPolygonal(Renderer &renderer, DrawType drawType) override;

    void drawCurve(Renderer &renderer, DrawType drawType) override;

    void pointUpdate(const std::shared_ptr<Point> &point, int index) override;

    void postUpdate() override;

    void pointMoved(const std::weak_ptr<Point> &point) override;

    static DirectX::XMFLOAT3 newMin(DirectX::XMFLOAT3 oldMin, DirectX::XMFLOAT3 candidate);

    static DirectX::XMFLOAT3 newMax(DirectX::XMFLOAT3 oldMax, DirectX::XMFLOAT3 candidate);
};


#endif //MG1_BREZIERCURVE_H
