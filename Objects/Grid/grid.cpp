//
// Created by Molom on 2022-04-08.
//

#include <DirectXMath.h>
#include "grid.h"

using namespace DirectX;

const XMFLOAT3 Grid::color = {0.5f, 0.5f, 0.7f};

Grid::Grid(int n) : Object(0, "Grid", {0, 0, 0}, D3D11_PRIMITIVE_TOPOLOGY_LINELIST) {
    vertices.clear();

    for (int i = 0; i < n; ++i) {
        float z = 1.0f / static_cast<float>(n - 1) * static_cast<float>(i) - 0.5f;
        vertices.push_back({{-0.5f, 0.0f, z}, color});
        vertices.push_back({{0.5f, 0.0f, z}, color});
    }

    for (int i = 0; i < n; ++i) {
        float x = 1.0f / static_cast<float>(n - 1) * static_cast<float>(i) - 0.5f;
        vertices.push_back({{x, 0.0f, -0.5f}, color});
        vertices.push_back({{x, 0.0f, 0.5f}, color});
    }
    auto size = static_cast<float>(n);
    Object::setScale({size, 1, size});
    updateBuffers();
}

void Grid::draw(Renderer &renderer, DrawType drawType) {
    renderer.draw(*this, DEFAULT_COLOR);
}

Type Grid::type() const {
    return GRID;
}

BoundingOrientedBox Grid::boundingBox() const {
    return {{},
            {},
            {0, 0, 0, 1.f}};
}
