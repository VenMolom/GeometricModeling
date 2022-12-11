//
// Created by Molom on 11/12/2022.
//

#ifndef MG1_TOOLDISTANTSURFACE_H
#define MG1_TOOLDISTANTSURFACE_H

#include <DirectXMath.h>
#include "parametricObject.h"
#include "../Patch/patch.h"

#define TOOL_DISTANT_DIM 2

class ToolDistantSurface: public ParametricObject<TOOL_DISTANT_DIM> {
public:
    ToolDistantSurface(const std::shared_ptr<Patch>& patch, float distant);

    std::array<bool, 2> looped() const override;

    DirectX::XMVECTOR value(const std::array<float, 2> &parameters) const override;

    DirectX::XMVECTOR tangent(const std::array<float, 2> &parameters) const override;

    DirectX::XMVECTOR bitangent(const std::array<float, 2> &parameters) const override;

    void draw(Renderer &renderer, DrawType drawType) override {}

    Type type() const override;

private:
    const std::shared_ptr<Patch> patch;
    float distant;

    DirectX::XMVECTOR normal(const std::array<float, 2> &parameters) const;
};


#endif //MG1_TOOLDISTANTSURFACE_H
