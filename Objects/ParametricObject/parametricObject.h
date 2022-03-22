//
// Created by Molom on 2022-03-11.
//

#ifndef MG1_PARAMETRICOBJECT_H
#define MG1_PARAMETRICOBJECT_H

#include <vector>
#include <array>
#include <functional>
#include <DirectXMath.h>
#include "Objects/Object/object.h"

template<size_t Dim>
class ParametricObject : public Object {
protected:
    ParametricObject(QString name,
                     DirectX::XMFLOAT3 position,
                     std::array<int, Dim> density,
                     std::array<std::tuple<float, float>, Dim> range);

    // must be called by derived class constructor
    void calculateVerticesAndIndices();

    virtual std::vector<VertexPositionColor>
    calculateVertices(const std::array<int, Dim> &density,
                      const std::array<std::tuple<float, float>, Dim> &range) const = 0;

    virtual std::vector<Index> calculateIndices(const std::array<int, Dim> &density) const = 0;

public:
    void draw(Renderer &renderer, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, DrawType drawType) const final;

    const std::array<int, Dim> &density() const { return _density; }

    void setDensity(const std::array<int, Dim> &density);

    const std::array<std::tuple<float, float>, Dim> &range() const { return _range; }

    virtual std::array<bool, 2> looped() const = 0;

private:
    std::array<int, Dim> _density;
    std::array<std::tuple<float, float>, Dim> _range;

    std::vector<VertexPositionColor> vertices;
    std::vector<Index> indices;
};

template<size_t Dim>
ParametricObject<Dim>::ParametricObject(QString name,
                                        DirectX::XMFLOAT3 position,
                                        std::array<int, Dim> density,
                                        std::array<std::tuple<float, float>, Dim> range)
        : Object(name, position),
          _density(density),
          _range(range) {
}

template<size_t Dim>
void ParametricObject<Dim>::draw(Renderer &renderer, DirectX::XMMATRIX view, DirectX::XMMATRIX projection,
                                 DrawType drawType) const {
    auto mvp = modelMatrix() * view * projection;
    renderer.drawLines(vertices, indices, mvp, drawType != DEFAULT);
    if (drawType == SELECTED) renderer.drawCursor(DirectX::XMLoadFloat4x4(&noScaleMatrix) * view * projection);
}

template<size_t Dim>
void ParametricObject<Dim>::calculateVerticesAndIndices() {
    vertices = calculateVertices(_density, _range);
    indices = calculateIndices(_density);
}

template<size_t Dim>
void ParametricObject<Dim>::setDensity(const std::array<int, Dim> &density) {
    _density = density;
    calculateVerticesAndIndices();
}

#endif //MG1_PARAMETRICOBJECT_H
