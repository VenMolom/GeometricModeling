//
// Created by Molom on 2022-06-04.
//

#ifndef MG1_GREGORYPATCH_H
#define MG1_GREGORYPATCH_H

#include "bicubicC0.h"
#include "Utils/gregoryUtils.h"

class GregoryPatch : public Patch {
public:
    GregoryPatch(uint id, const std::array<std::shared_ptr<BicubicC0>, 3> &patches,
                 GregoryUtils::GregoryInfo fillInInfo);

    Type type() const override;

    void setPosition(DirectX::XMFLOAT3 position) override {}

    void setRotation(DirectX::XMFLOAT3 rotation) override {}

    void setScale(DirectX::XMFLOAT3 scale) override {}

    void replacePoint(std::shared_ptr<VirtualPoint> point, std::shared_ptr<VirtualPoint> newPoint) override {}

protected:
    void calculateMeshIndices(std::array<int, 2> segments, Linelist &linelist) override;

    void drawMesh(Renderer &renderer, DrawType drawType) override;

    void drawCursor(Renderer &renderer, DrawType drawType) override {}

    void drawPoints(Renderer &renderer, DrawType drawType) override {}

private:
    std::array<std::shared_ptr<BicubicC0>, 3> patches;
    GregoryUtils::GregoryInfo fillInInfo;

    void clear();
    void createPoints();
    void setHandlers();
};


#endif //MG1_GREGORYPATCH_H
