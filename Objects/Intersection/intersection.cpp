//
// Created by Molom on 2022-06-20.
//

#include "intersection.h"

using namespace std;
using namespace DirectX;

Intersection::Intersection(uint id, const vector<XMFLOAT3> &points, bool closed)
        : Object(id, "Intersection", {0, 0, 0}, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP) {
    uint index = 0;
    for (auto &point : points) {
        vertices.push_back({point, {1, 1, 1}});
        indices.push_back(index++);
    }
    if (closed) {
        indices.push_back(0);
    }
    updateBuffers();
}

void Intersection::draw(Renderer &renderer, DrawType drawType) {
    renderer.draw(*this, drawType != DEFAULT ? SELECTED_COLOR : DEFAULT_COLOR);
}

Type Intersection::type() const {
    return INTERSECTION;
}