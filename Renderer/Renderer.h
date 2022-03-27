//
// Created by Molom on 2022-03-11.
//

#ifndef MG1_RENDERER_H
#define MG1_RENDERER_H

#include <vector>

class Renderer {
public:
    virtual void drawLines(const std::vector<VertexPositionColor> &vertices,
                           const std::vector<Index> &indices,
                           const DirectX::XMMATRIX &mvp, bool selected) = 0;

    virtual void drawCursor(const DirectX::XMMATRIX &mvp) = 0;

    virtual void drawPoint(const DirectX::XMMATRIX &mvp, bool selected) = 0;

    virtual void drawCurve4(const std::vector<VertexPositionColor> &controlPoints,
                            DirectX::XMVECTOR min, DirectX::XMVECTOR max,
                            const DirectX::XMMATRIX & mvp, bool selected) = 0;

    virtual void drawLineStrip(const std::vector<VertexPositionColor> & points,
                               const DirectX::XMMATRIX & mvp, bool selected) = 0;
};

#endif //MG1_RENDERER_H