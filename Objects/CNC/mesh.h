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
         D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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

    static Mesh cylinder(float radius, float height, unsigned int stacks, unsigned int slices) {
        return {cylinderVerts(radius, height, stacks, slices), cylinderIdx(stacks, slices)};
    }

    static Mesh disk(float radius, unsigned int slices, DirectX::XMFLOAT3 color, DirectX::XMMATRIX modelMatrix) {
        return {diskVerts(radius, slices), diskIdx(slices), color, modelMatrix};
    }

    static Mesh disk(float radius, unsigned int slices) {
        return {diskVerts(radius, slices), diskIdx(slices)};
    }

    static Mesh dome(float radius, unsigned int stacks, unsigned int slices, DirectX::XMFLOAT3 color,
                     DirectX::XMMATRIX modelMatrix) {
        return {domeVerts(radius, stacks, slices, 1.f, 0.f), domeIdx(stacks, slices), color, modelMatrix};
    }

    static Mesh dome(float radius, unsigned int stacks, unsigned int slices, float zSign = 1.f, float zOffset = 0.f) {
        return {domeVerts(radius, stacks, slices, zSign, zOffset), domeIdx(stacks, slices)};
    }

    static Mesh square(float side) {
        return {squareVerts(side), squareIdx()};
    }

    static Mesh halfCylinder(float radius, float height, unsigned int stacks, unsigned int slices) {
        return {halfCylinderVerts(radius, height, stacks, slices), halfCylinderIdx(stacks, slices)};
    }

private:
    DirectX::XMFLOAT4X4 model;
    DirectX::XMFLOAT3 _color;

    static std::vector<VertexPositionColor>
    cylinderVerts(float radius, float height, unsigned int stacks, unsigned int slices);

    static std::vector<Index> cylinderIdx(unsigned int stacks, unsigned int slices);

    static std::vector<VertexPositionColor> diskVerts(float radius, unsigned int slices);

    static std::vector<Index> diskIdx(unsigned int slices);

    static std::vector<VertexPositionColor>
    domeVerts(float radius, unsigned int stacks, unsigned int slices, float zSign, float zOffset);

    static std::vector<Index> domeIdx(unsigned int stacks, unsigned int slices);

    static std::vector<VertexPositionColor> squareVerts(float side);

    static std::vector<Index> squareIdx();

    static std::vector<VertexPositionColor>
    halfCylinderVerts(float radius, float height, unsigned int stacks, unsigned int slices);

    static std::vector<Index> halfCylinderIdx(unsigned int stacks, unsigned int slices);
};


#endif //MG1_MESH_H
