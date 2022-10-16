//
// Created by Molom on 15/10/2022.
//

#ifndef MG1_MESH_H
#define MG1_MESH_H

#include "DirectX/renderer.h"
#include "Objects/renderable.h"

class Mesh : public Renderable {
public:
    Mesh();

    Mesh(std::vector<VertexPositionColor> vertices,
         std::vector<Index> indices,
         DirectX::XMFLOAT3 color,
         DirectX::XMMATRIX modelMatrix,
         D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    DirectX::XMMATRIX modelMatrix() const { return DirectX::XMLoadFloat4x4(&model); }

    DirectX::XMFLOAT3 color() const { return _color; }

    static Mesh cylinder(float radius, float height, unsigned int stacks, unsigned int slices, DirectX::XMFLOAT3 color,
                         DirectX::XMMATRIX modelMatrix) {
        return {cylinderVerts(radius, height, stacks, slices), cylinderIdx(stacks, slices), color, modelMatrix};
    }

    static Mesh disk(float radius, unsigned int slices, DirectX::XMFLOAT3 color, DirectX::XMMATRIX modelMatrix) {
        return {diskVerts(radius, slices), diskIdx(slices), color, modelMatrix};
    }

    static Mesh dome(float radius, unsigned int stacks, unsigned int slices, DirectX::XMFLOAT3 color,
                     DirectX::XMMATRIX modelMatrix) {
        return {domeVerts(radius, stacks, slices), domeIdx(stacks, slices), color, modelMatrix};
    }

private:
    DirectX::XMFLOAT4X4 model;
    DirectX::XMFLOAT3 _color;

    static std::vector<VertexPositionColor>
    cylinderVerts(float radius, float height, unsigned int stacks, unsigned int slices);

    static std::vector<Index> cylinderIdx(unsigned int stacks, unsigned int slices);

    static std::vector<VertexPositionColor> diskVerts(float radius, unsigned int slices);

    static std::vector<Index> diskIdx(unsigned int slices);

    static std::vector<VertexPositionColor> domeVerts(float radius, unsigned int stacks, unsigned int slices);

    static std::vector<Index> domeIdx(unsigned int stacks, unsigned int slices);
};


#endif //MG1_MESH_H
