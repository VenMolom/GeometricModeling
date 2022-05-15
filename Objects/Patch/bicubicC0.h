//
// Created by Molom on 2022-05-13.
//

#ifndef MG1_BICUBICC0_H
#define MG1_BICUBICC0_H

#include "patch.h"

class BicubicC0 : public Patch {
public:
    BicubicC0(uint id, QString name, DirectX::XMFLOAT3 position, std::array<int, PATCH_DIM> segments,
              std::array<float, PATCH_DIM> size, bool cylinder,
              QBindable<std::weak_ptr<Object>> bindableSelected);

    Type type() const override;

protected:
    void createSegments(std::array<int, PATCH_DIM> segments, std::array<float, PATCH_DIM> size);

    void calculateMeshIndices(std::array<int, PATCH_DIM> segments, Linelist &linelist) override;

private:
    void createCylinderSegments(std::array<int, PATCH_DIM> segments, std::array<float, PATCH_DIM> size);

    void createPlaneSegments(std::array<int, PATCH_DIM> segments, std::array<float, PATCH_DIM> size);
};


#endif //MG1_BICUBICC0_H
