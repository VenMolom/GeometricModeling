//
// Created by Molom on 2022-05-21.
//

#ifndef MG1_BICUBICC2_H
#define MG1_BICUBICC2_H

#include <Models/Surfaces/BezierSurfaceC2.h>
#include "patch.h"

class BicubicC2 : public Patch {
public:
    BicubicC2(uint id, QString name, DirectX::XMFLOAT3 position, std::array<int, PATCH_DIM> segments,
            std::array<float, PATCH_DIM> size, bool cylinder,
            QBindable<std::weak_ptr<Object>> bindableSelected);

    Type type() const override;

    MG1::BezierSurfaceC2 serialize(std::vector<MG1::Point> &serializedPoints);

protected:
    void drawMesh(Renderer &renderer, DrawType drawType) override;

    void calculateMeshIndices(std::array<int, PATCH_DIM> segments, Linelist &linelist) override;

    void createCylinderSegments(std::array<int, PATCH_DIM> segments, std::array<float, PATCH_DIM> size) override;

    void createPlaneSegments(std::array<int, PATCH_DIM> segments, std::array<float, PATCH_DIM> size) override;
};


#endif //MG1_BICUBICC2_H
