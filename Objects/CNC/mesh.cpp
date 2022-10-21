//
// Created by Molom on 15/10/2022.
//

#include "mesh.h"
#include <utility>

using namespace std;
using namespace DirectX;

Mesh::Mesh() : Renderable(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST) {

}

Mesh::Mesh(std::vector<VertexPositionColor> vertices, std::vector<Index> indices, D3D11_PRIMITIVE_TOPOLOGY topology)
        : Renderable(topology) {
    setBuffers(std::move(vertices), std::move(indices));
}

Mesh::Mesh(vector<VertexPositionColor> vertices, vector<Index> indices, XMFLOAT3 color, XMMATRIX modelMatrix,
           D3D11_PRIMITIVE_TOPOLOGY topology) : Mesh(vertices, indices, topology) {
    _color = color;
    XMStoreFloat4x4(&model, modelMatrix);
}

std::vector<VertexPositionColor>
Mesh::cylinderVerts(float radius, float height, unsigned int stacks, unsigned int slices) {
    assert(stacks > 0 && slices > 1);
    auto n = (stacks + 1) * slices;
    vector<VertexPositionColor> vertices(n);
    auto y = height / 2;
    auto dy = height / stacks;
    auto dp = XM_2PI / slices;
    auto k = 0U;
    for (auto i = 0U; i <= stacks; ++i, y -= dy) {
        auto phi = 0.0f;
        for (auto j = 0U; j < slices; ++j, phi += dp) {
            float sinp, cosp;
            XMScalarSinCos(&sinp, &cosp, phi);
            vertices[k].position = XMFLOAT3(radius * cosp, radius * sinp, y);
            vertices[k++].color = XMFLOAT3(cosp, sinp, 0);
        }
    }
    return vertices;
}

std::vector<Index> Mesh::cylinderIdx(unsigned int stacks, unsigned int slices) {
    assert(stacks > 0 && slices > 1);
    auto in = 6 * stacks * slices;
    vector<Index> indices(in);
    auto k = 0U;
    for (auto i = 0U; i < stacks; ++i) {
        auto j = 0U;
        for (; j < slices - 1; ++j) {
            indices[k++] = i * slices + j;
            indices[k++] = i * slices + j + 1;
            indices[k++] = (i + 1) * slices + j + 1;
            indices[k++] = i * slices + j;
            indices[k++] = (i + 1) * slices + j + 1;
            indices[k++] = (i + 1) * slices + j;
        }
        indices[k++] = i * slices + j;
        indices[k++] = i * slices;
        indices[k++] = (i + 1) * slices;
        indices[k++] = i * slices + j;
        indices[k++] = (i + 1) * slices;
        indices[k++] = (i + 1) * slices + j;
    }
    return indices;
}

std::vector<VertexPositionColor> Mesh::diskVerts(float radius, unsigned int slices) {
    assert(slices > 1);
    auto n = slices + 1;
    vector<VertexPositionColor> vertices(n);
    vertices[0].position = XMFLOAT3(0.0f, 0.0f, 0.0f);
    vertices[0].color = XMFLOAT3(0.0f, 0.0f, 1.0f);
    auto phi = 0.0f;
    auto dp = XM_2PI / slices;
    auto k = 1;
    for (auto i = 1U; i <= slices; ++i, phi += dp) {
        float cosp, sinp;
        XMScalarSinCos(&sinp, &cosp, phi);
        vertices[k].position = XMFLOAT3(radius * cosp, radius * sinp, 0.f);
        vertices[k++].color = XMFLOAT3(0.0f, 0.0f, 1.0f);
    }
    return vertices;
}

std::vector<Index> Mesh::diskIdx(unsigned int slices) {
    assert(slices > 1);
    auto in = slices * 3;
    vector<Index> indices(in);
    auto k = 0U;
    for (auto i = 0U; i < slices - 1; ++i) {
        indices[k++] = 0;
        indices[k++] = i + 2;
        indices[k++] = i + 1;
    }
    indices[k++] = 0;
    indices[k++] = 1;
    indices[k] = slices;
    return indices;
}

std::vector<VertexPositionColor>
Mesh::domeVerts(float radius, unsigned int stacks, unsigned int slices, float zSign, float zOffset) {
    assert(stacks > 2 && slices > 1);
    auto n = stacks * slices + 1U;
    vector<VertexPositionColor> vertices(n);
    vertices[0].position = XMFLOAT3(0.f, 0.f, radius * zSign + zOffset);
    vertices[0].color = XMFLOAT3(0.f, 0.f, 1.f);
    auto dp = XM_PIDIV2 / stacks;
    auto phi = dp;
    auto k = 1U;
    for (auto i = 0U; i < stacks; ++i, phi += dp) {
        float cosp, sinp;
        XMScalarSinCos(&sinp, &cosp, phi);
        auto thau = 0.0f;
        auto dt = XM_2PI / slices;
        auto stackR = radius * sinp;
        auto stackY = radius * cosp;
        for (auto j = 0U; j < slices; ++j, thau += dt) {
            float cost, sint;
            XMScalarSinCos(&sint, &cost, thau);
            vertices[k].position = XMFLOAT3(stackR * cost, stackR * sint, stackY * zSign + zOffset);
            vertices[k++].color = XMFLOAT3(cost * sinp, sint * sinp, cosp);
        }
    }
    return vertices;
}

std::vector<Index> Mesh::domeIdx(unsigned int stacks, unsigned int slices) {
    assert(stacks > 2 && slices > 1);
    auto in = (stacks - 1U) * slices * 6U + slices * 3U;
    vector<Index> indices(in);
    auto k = 0U;
    for (auto j = 0U; j < slices - 1U; ++j) {
        indices[k++] = 0U;
        indices[k++] = j + 2;
        indices[k++] = j + 1;
    }
    indices[k++] = 0U;
    indices[k++] = 1U;
    indices[k++] = slices;
    auto i = 0U;
    for (; i < stacks - 1U; ++i) {
        auto j = 0U;
        for (; j < slices - 1U; ++j) {
            indices[k++] = i * slices + j + 1;
            indices[k++] = i * slices + j + 2;
            indices[k++] = (i + 1) * slices + j + 2;
            indices[k++] = i * slices + j + 1;
            indices[k++] = (i + 1) * slices + j + 2;
            indices[k++] = (i + 1) * slices + j + 1;
        }
        indices[k++] = i * slices + j + 1;
        indices[k++] = i * slices + 1;
        indices[k++] = (i + 1) * slices + 1;
        indices[k++] = i * slices + j + 1;
        indices[k++] = (i + 1) * slices + 1;
        indices[k++] = (i + 1) * slices + j + 1;
    }
    return indices;
}

std::vector<VertexPositionColor> Mesh::squareVerts(float side) {
    return {
            {{-0.5f * side, -0.5f * side, 0.0f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f * side, +0.5f * side, 0.0f}, {0.0f, 0.0f, 1.0f}},
            {{+0.5f * side, +0.5f * side, 0.0f}, {0.0f, 0.0f, 1.0f}},
            {{+0.5f * side, -0.5f * side, 0.0f}, {0.0f, 0.0f, 1.0f}}
    };
}

std::vector<Index> Mesh::squareIdx() {
    return {0, 1, 2, 0, 2, 3};
}

std::vector<VertexPositionColor>
Mesh::halfCylinderVerts(float radius, float height, unsigned int stacks, unsigned int slices) {
    assert(stacks > 0 && slices > 1);
    auto n = (stacks + 1) * (slices + 1);
    vector<VertexPositionColor> vertices(n);
    auto y = height / 2;
    auto dy = height / stacks;
    auto dp = XM_PI / slices;
    auto k = 0U;
    for (auto i = 0U; i <= stacks; ++i, y -= dy) {
        auto phi = 0.0f;
        for (auto j = 0U; j <= slices; ++j, phi += dp) {
            float sinp, cosp;
            XMScalarSinCos(&sinp, &cosp, phi);
            vertices[k].position = XMFLOAT3(y, radius * cosp, -radius * sinp + radius);
            vertices[k++].color = XMFLOAT3(0, cosp, sinp);
        }
    }
    return vertices;
}

std::vector<Index> Mesh::halfCylinderIdx(unsigned int stacks, unsigned int slices) {
    assert(stacks > 0 && slices > 1);
    auto in = 6 * stacks * slices;
    vector<Index> indices(in);
    auto k = 0U;
    for (auto i = 0U; i < stacks; ++i) {
        for (auto j = 0U; j < slices; ++j) {
            indices[k++] = i * (slices + 1) + j;
            indices[k++] = i * (slices + 1) + j + 1;
            indices[k++] = (i + 1) * (slices + 1) + j + 1;
            indices[k++] = i * (slices + 1) + j;
            indices[k++] = (i + 1) * (slices + 1) + j + 1;
            indices[k++] = (i + 1) * (slices + 1) + j;
        }
    }
    return indices;
}