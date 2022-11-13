//
// Created by Molom on 2022-06-20.
//

#include "intersection.h"

using namespace std;
using namespace DirectX;

Intersection::Intersection(uint id, const array<shared_ptr<ParametricObject<2>>, 2> &surfaces,
                           const vector<pair<float, float>> &firstParameters,
                           const vector<pair<float, float>> &secondParameters,
                           const vector<XMFLOAT3> &points, bool closed, Renderer &renderer)
        : Object(id, "Intersection", {0, 0, 0}, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP),
          _closed(closed),
          _secondParameters(secondParameters),
          _points(points) {
    uint index = 0;
    for (auto &point: points) {
        vertices.push_back({point, {1, 1, 1}});
        indices.push_back(index++);
    }
    if (closed) {
        indices.push_back(0);
    }
    updateBuffers();

    instances[0] = make_shared<IntersectionInstance>(firstParameters, surfaces[0]->range(),
                                                     surfaces[0]->looped(), closed, renderer);
    surfaces[0]->setIntersectionInstance(instances[0]);
    instances[1] = make_shared<IntersectionInstance>(secondParameters, surfaces[1]->range(),
                                                     surfaces[1]->looped(), closed, renderer);
    surfaces[1]->setIntersectionInstance(instances[1]);
}

Intersection::Intersection(uint id, const shared_ptr<ParametricObject<2>> &surface,
                           const vector<pair<float, float>> &firstParameters,
                           const vector<pair<float, float>> &secondParameters,
                           const vector<XMFLOAT3> &points, bool closed, Renderer &renderer)
        : Object(id, "Intersection", {0, 0, 0}, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP),
          _closed(closed),
          _secondParameters(secondParameters),
          _points(points) {
    uint index = 0;
    for (auto &point: points) {
        vertices.push_back({point, {1, 1, 1}});
        indices.push_back(index++);
    }
    if (closed) {
        indices.push_back(0);
    }
    updateBuffers();

    instances[0] = make_shared<IntersectionInstance>(firstParameters, secondParameters, surface->range(),
                                                     surface->looped(), closed, renderer);
    surface->setIntersectionInstance(instances[0]);
}

void Intersection::draw(Renderer &renderer, DrawType drawType) {
    renderer.draw(*this, drawType != DEFAULT ? SELECTED_COLOR : DEFAULT_COLOR);
}

Type Intersection::type() const {
    return INTERSECTION;
}