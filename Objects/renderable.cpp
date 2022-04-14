//
// Created by Molom on 2022-04-14.
//

#include "renderable.h"

using namespace std;
using namespace mini;

Renderable::Renderable(D3D11_PRIMITIVE_TOPOLOGY topology) : topology(topology) {

}

void Renderable::updateBuffers() {
    if (vertices.empty()) {
        indexBuffer.reset();
        vertexBuffer.reset();
        return;
    }

    vertexBuffer = DxDevice::Instance().CreateVertexBuffer(vertices);
    indexCount = vertices.size();
    indexBuffer.reset();

    if (!indices.empty()) {
        indexBuffer = DxDevice::Instance().CreateIndexBuffer(indices);
        indexCount = indices.size();
    }
}

void Renderable::setBuffers(vector<VertexPositionColor> vertices,
                            vector<Index> indices) {
    if (vertices.empty()) {
        indexBuffer.reset();
        vertexBuffer.reset();
        return;
    }

    vertexBuffer = DxDevice::Instance().CreateVertexBuffer(vertices);
    indexCount = vertices.size();
    indexBuffer.reset();

    if (!indices.empty()) {
        indexBuffer = DxDevice::Instance().CreateIndexBuffer(indices);
        indexCount = indices.size();
    }
}

void Renderable::render(const dx_ptr<ID3D11DeviceContext> &context) {
    if (!vertexBuffer) return;

    context->IASetPrimitiveTopology(topology);
    ID3D11Buffer *vb = vertexBuffer.get();
    context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

    if (indexBuffer) {
        context->IASetIndexBuffer(indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
        context->DrawIndexed(indexCount, 0, 0);
    } else {
        context->Draw(indexCount, 0);
    }
}