//
// Created by Molom on 2022-03-11.
//

#ifndef MG1_RENDERER_H
#define MG1_RENDERER_H

#include <vector>

enum Topology {
    LineList = D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
    LineStrip = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
};

const DirectX::XMFLOAT4 SELECTED_COLOR{1.0f, 0.4f, 0.0f, 1.0f};
const DirectX::XMFLOAT4 DEFAULT_COLOR{0.0f, 0.0f, 0.0f, 0.0f};
const DirectX::XMFLOAT4 POLYGONAL_COLOR{0.0f, 1.0f, 1.0f, 1.0f};

class Renderer {
public:
    virtual void drawIndexed(const std::vector<VertexPositionColor> &vertices,
                             const std::vector<Index> &indices,
                             Topology topology,
                             const DirectX::XMMATRIX &mvp,
                             DirectX::XMFLOAT4 colorOverride) = 0;

    virtual void draw(const std::vector<VertexPositionColor> &points,
                      Topology topology,
                      const DirectX::XMMATRIX &mvp,
                      DirectX::XMFLOAT4 colorOverride) = 0;

    virtual void drawGrid(const std::vector<VertexPositionColor> &points, const DirectX::XMMATRIX &mvp) = 0;

    virtual void drawCurve4(const std::vector<VertexPositionColor> &controlPoints,
                            const std::vector<Index> &indices, int lastPatchSize,
                            DirectX::XMVECTOR min, DirectX::XMVECTOR max,
                            const DirectX::XMMATRIX &mvp,
                            DirectX::XMFLOAT4 colorOverride) = 0;
};

#endif //MG1_RENDERER_H