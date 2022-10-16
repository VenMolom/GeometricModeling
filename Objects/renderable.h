//
// Created by Molom on 2022-04-14.
//

#ifndef MG1_RENDERABLE_H
#define MG1_RENDERABLE_H

#include "DirectX/DXDevice/dxptr.h"
#include "DirectX/DXDevice/dxDevice.h"
#include "DirectX/DXStructures/dxStructures.h"

class Renderable {
public:
    void render(const mini::dx_ptr<ID3D11DeviceContext> &context) const;

protected:
    std::vector<VertexPositionColor> vertices;
    std::vector<VertexPositionTexture> verticesTextured;
    std::vector<Index> indices;

    explicit Renderable(D3D11_PRIMITIVE_TOPOLOGY topology);

    void updateBuffers();

    void setBuffers(std::vector<VertexPositionColor>&& vertices, std::vector<Index>&& indices);

    void setBuffers(const std::vector<VertexPositionColor>& vertices, const std::vector<Index>& indices);

    void setTopology(D3D11_PRIMITIVE_TOPOLOGY topology) { this->topology = topology; }

    void switchIndexed(bool enabled) { indexed = enabled; }

    void convertToTextured();

private:
    mini::dx_ptr<ID3D11Buffer> indexBuffer{};
    mini::dx_ptr<ID3D11Buffer> vertexBuffer{};
    unsigned int stride = sizeof(VertexPositionColor);
    unsigned int offset = 0;
    unsigned int indexCount = 0;
    unsigned int vertexCount = 0;
    bool indexed = true;
    bool textured = false;
    D3D11_PRIMITIVE_TOPOLOGY topology;

    void updateBuffersTextured();
};


#endif //MG1_RENDERABLE_H
