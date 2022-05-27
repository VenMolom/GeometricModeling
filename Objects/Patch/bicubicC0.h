//
// Created by Molom on 2022-05-13.
//

#ifndef MG1_BICUBICC0_H
#define MG1_BICUBICC0_H

#include <Models/Surfaces/BezierSurfaceC0.h>
#include "patch.h"

class BicubicC0 : public Patch {
public:
    BicubicC0(uint id, QString name, DirectX::XMFLOAT3 position, std::array<int, PATCH_DIM> segments,
              std::array<float, PATCH_DIM> size, bool cylinder,
              QBindable<std::weak_ptr<Object>> bindableSelected);

    BicubicC0(const MG1::BezierSurfaceC0 &surface, std::vector<MG1::Point> &serializedPoints,
          QBindable<std::weak_ptr<Object>> bindableSelected);

    Type type() const override;

    MG1::BezierSurfaceC0 serialize(std::vector<MG1::Point> &serializedPoints);

protected:
    void calculateMeshIndices(std::array<int, PATCH_DIM> segments, Linelist &linelist) override;

    void createCylinderSegments(std::array<int, PATCH_DIM> segments, std::array<float, PATCH_DIM> size) override;

    void createPlaneSegments(std::array<int, PATCH_DIM> segments, std::array<float, PATCH_DIM> size) override;
};


#endif //MG1_BICUBICC0_H
