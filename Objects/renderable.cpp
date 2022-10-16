//
// Created by Molom on 2022-04-14.
//

#include "renderable.h"

using namespace std;
using namespace mini;

Renderable::Renderable(D3D11_PRIMITIVE_TOPOLOGY topology) : topology(topology) {

}

void Renderable::updateBuffers() {
    if (textured) {
        updateBuffersTextured();
        return;
    }

    if (vertices.empty()) {
        indexBuffer.reset();
        vertexBuffer.reset();
        return;
    }

    vertexBuffer = DxDevice::Instance().CreateVertexBuffer(vertices);
    vertexCount = vertices.size();
    indexBuffer.reset();

    if (!indices.empty()) {
        indexBuffer = DxDevice::Instance().CreateIndexBuffer(indices);
        indexCount = indices.size();
    }
}

void Renderable::setBuffers(vector<VertexPositionColor>&& vertices, vector<Index>&& indices) {
    this->vertices = vertices;
    this->indices = indices;

    updateBuffers();
}

void Renderable::setBuffers(const vector<VertexPositionColor>& vertices, const vector<Index>& indices) {
    this->vertices = vertices;
    this->indices = indices;

    updateBuffers();
}

void Renderable::render(const dx_ptr<ID3D11DeviceContext> &context) const {
    if (!vertexBuffer) return;

    context->IASetPrimitiveTopology(topology);
    ID3D11Buffer *vb = vertexBuffer.get();
    context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

    if (indexBuffer && indexed) {
        context->IASetIndexBuffer(indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
        context->DrawIndexed(indexCount, 0, 0);
    } else {
        context->Draw(vertexCount, 0);
    }
}

void Renderable::convertToTextured() {
    textured = true;
    stride = sizeof(VertexPositionTexture);
    updateBuffersTextured();
}

void Renderable::updateBuffersTextured() {
    if (verticesTextured.empty()) {
        indexBuffer.reset();
        vertexBuffer.reset();
        return;
    }

    vertexBuffer = DxDevice::Instance().CreateVertexBuffer(verticesTextured);
    vertexCount = verticesTextured.size();
    indexBuffer.reset();

    if (!indices.empty()) {
        indexBuffer = DxDevice::Instance().CreateIndexBuffer(indices);
        indexCount = indices.size();
    }
}
