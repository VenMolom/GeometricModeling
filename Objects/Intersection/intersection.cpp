//
// Created by Molom on 2022-06-20.
//

#include "intersection.h"

using namespace std;
using namespace DirectX;

Intersection::Intersection(uint id, const vector<XMFLOAT3> &points)
        : Object(id, "Intersection", {0, 0, 0}, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP) {
    for (auto &point : points) {
        vertices.push_back({point, {1, 1, 1}});
    }
    updateBuffers();
}

void Intersection::draw(Renderer &renderer, DrawType drawType) {
    renderer.draw(*this, drawType != DEFAULT ? SELECTED_COLOR : DEFAULT_COLOR);
}

Type Intersection::type() const {
    return INTERSECTION;
}