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
    virtual void render(const mini::dx_ptr<ID3D11DeviceContext> &context);

protected:
    std::vector<VertexPositionColor> vertices;
    std::vector<Index> indices;

    Renderable(D3D11_PRIMITIVE_TOPOLOGY topology);

    void updateBuffers();

    void setBuffers(std::vector<VertexPositionColor> vertices, std::vector<Index> indices);

private:
    mini::dx_ptr<ID3D11Buffer> indexBuffer{};
    mini::dx_ptr<ID3D11Buffer> vertexBuffer{};
    unsigned int stride = sizeof(VertexPositionColor);
    unsigned int offset = 0;
    unsigned int indexCount = 0;
    D3D11_PRIMITIVE_TOPOLOGY topology;
};


#endif //MG1_RENDERABLE_H
