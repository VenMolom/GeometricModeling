//
// Created by Molom on 2022-03-11.
//

#ifndef MG1_PARAMETRICOBJECT_H
#define MG1_PARAMETRICOBJECT_H

#include <DirectXMath.h>
#include <vector>
#include <array>
#include <functional>
#include "Objects/Object/object.h"

template <size_t Dim>
class ParametricObject : public Object {
protected:
    ParametricObject(DirectX::XMFLOAT3 position,
                     DirectX::XMFLOAT3 color,
                     std::array<int, Dim> density,
                     std::array<std::tuple<float, float>, Dim> range);

    // must be called by derived class constructor
    void calculateVerticesAndIndices();

    virtual VertexPositionColor parametricFunction(std::array<float, Dim> parameters) const = 0;

public:
    void draw(Renderer &renderer, const DirectX::XMMATRIX &camera) const final;

    void setDensity(std::array<int, Dim> density);

private:
    std::array<int, Dim> density;
    std::array<std::tuple<float, float>, Dim> range;

    std::vector<VertexPositionColor> vertices;
    std::vector<Index> indices;

    void calculateDimension(std::array<float, Dim> &value, int dimension);
};

template<size_t Dim>
ParametricObject<Dim>::ParametricObject(DirectX::XMFLOAT3 position,
                                        DirectX::XMFLOAT3 color,
                                        std::array<int, Dim> density,
                                        std::array<std::tuple<float, float>, Dim> range)
        : Object(position, color),
          density(density),
          range(range) {
}

template<size_t Dim>
void ParametricObject<Dim>::setDensity(std::array<int, Dim> density) {
    this->density = density;

    calculateVerticesAndIndices();
}

template<size_t Dim>
void ParametricObject<Dim>::draw(Renderer &renderer, const DirectX::XMMATRIX &camera) const {
    auto mvp = modelMatrix() * camera;
    renderer.drawLines(vertices, indices, mvp);
}

template<size_t Dim>
void ParametricObject<Dim>::calculateVerticesAndIndices() {
    vertices.clear();
    std::array<float, Dim> arr;
    calculateDimension(arr, 0);
}

template<size_t Dim>
void ParametricObject<Dim>::calculateDimension(std::array<float, Dim> &value, int dimension) {
    if (dimension == Dim) {
        vertices.push_back(parametricFunction(value));
        return;
    }

    auto [start, end] = range[dimension];
    auto delta = (end - start) / (density[dimension] - 1);

    for (auto i = 0; i < density[dimension]; ++i) {
        value[dimension] = start + i * delta;
        calculateDimension(value, dimension + 1);
    }
}

#endif //MG1_PARAMETRICOBJECT_H
