//
// Created by Molom on 11/12/2022.
//

#include <DirectXMath.h>
#include "toolDistantSurface.h"

using namespace std;
using namespace DirectX;

ToolDistantSurface::ToolDistantSurface(const std::shared_ptr<Patch>& patch, float distant)
        : ParametricObject<TOOL_DISTANT_DIM>(0, "", {}, patch->density(), patch->range(),
                                             D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST),
          patch(patch),
          distant(distant) {

}

array<bool, 2> ToolDistantSurface::looped() const {
    return patch->looped();
}

XMVECTOR ToolDistantSurface::value(const array<float, 2> &parameters) const {
    return XMVectorAdd(patch->value(parameters), XMVectorScale(normal(parameters), distant));
}

XMVECTOR ToolDistantSurface::tangent(const array<float, 2> &parameters) const {
    static const float DELTA = 0.00001f;

    auto parametersBefore = parameters;
    auto parametersAfter = parameters;
    parametersBefore[0] -= DELTA;
    parametersAfter[0] += DELTA;
    return XMVectorScale(XMVectorSubtract(value(parametersAfter), value(parametersBefore)), 1.f / DELTA);
}

XMVECTOR ToolDistantSurface::bitangent(const array<float, 2> &parameters) const {
    static const float DELTA = 0.00001f;

    auto parametersBefore = parameters;
    auto parametersAfter = parameters;
    parametersBefore[1] -= DELTA;
    parametersAfter[1] += DELTA;
    return XMVectorScale(XMVectorSubtract(value(parametersAfter), value(parametersBefore)), 1.f / DELTA);
}

Type ToolDistantSurface::type() const {
    return patch->type();
}

XMVECTOR ToolDistantSurface::normal(const array<float, 2> &parameters) const {
    return XMVector3Normalize(XMVector3Cross(patch->bitangent(parameters), patch->tangent(parameters)));
}
