//
// Created by Molom on 2022-05-21.
//

#ifndef MG1_BICUBICC2CREATOR_H
#define MG1_BICUBICC2CREATOR_H

#include "bicubicC2.h"
#include "patchCreator.h"

class BicubicC2Creator : public BicubicC2, public PatchCreator {
public:
    BicubicC2Creator(const DirectX::XMFLOAT3 &position, const QBindable<std::weak_ptr<Object>> &bindableSelected);

    void draw(Renderer &renderer, DrawType drawType) override;

    Type type() const override;

    void setSegments(const std::array<int, PATCH_DIM> &segments) override;

    void setSize(const std::array<float, PATCH_DIM> &size) override;

    void setCylinder(bool cylinder) override;

    std::shared_ptr<Object> create(uint id) override;

private:
    std::array<int, PATCH_DIM> segments{1,1};
    std::array<float, PATCH_DIM> size{1,1};
};


#endif //MG1_BICUBICC2CREATOR_H
