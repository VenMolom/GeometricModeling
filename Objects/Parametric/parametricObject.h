//
// Created by Molom on 2022-03-11.
//

#ifndef MG1_PARAMETRICOBJECT_H
#define MG1_PARAMETRICOBJECT_H

#include <vector>
#include <array>
#include <functional>
#include <DirectXMath.h>
#include "Objects/object.h"
#include "Objects/Cursor/cursor.h"

template<size_t Dim>
class ParametricObject : public Object {
protected:
    ParametricObject(QString name,
                     DirectX::XMFLOAT3 position,
                     std::array<int, Dim> density,
                     std::array<std::tuple<float, float>, Dim> range);

    // must be called by derived class constructor
    void calculateVerticesAndIndices();

    virtual void calculateVertices(const std::array<int, Dim> &density,
                                   const std::array<std::tuple<float, float>, Dim> &range) = 0;

    virtual void calculateIndices(const std::array<int, Dim> &density) = 0;

public:
    void draw(Renderer &renderer, DrawType drawType) final;

    const std::array<int, Dim> &density() const { return _density; }

    void setDensity(const std::array<int, Dim> &density);

    const std::array<std::tuple<float, float>, Dim> &range() const { return _range; }

    virtual std::array<bool, 2> looped() const = 0;

private:
    std::array<int, Dim> _density;
    std::array<std::tuple<float, float>, Dim> _range;
};

template<size_t Dim>
ParametricObject<Dim>::ParametricObject(QString name,
                                        DirectX::XMFLOAT3 position,
                                        std::array<int, Dim> density,
                                        std::array<std::tuple<float, float>, Dim> range)
        : Object(name, position, D3D11_PRIMITIVE_TOPOLOGY_LINELIST),
          _density(density),
          _range(range) {
}

template<size_t Dim>
void ParametricObject<Dim>::draw(Renderer &renderer, DrawType drawType) {
    renderer.draw(*this, drawType != DEFAULT ? SELECTED_COLOR : DEFAULT_COLOR);
    if (drawType == SELECTED) {
        Cursor::drawCursor(renderer, position(), rotation());
    }
}

template<size_t Dim>
void ParametricObject<Dim>::calculateVerticesAndIndices() {
    calculateVertices(_density, _range);
    calculateIndices(_density);
    updateBuffers();
}

template<size_t Dim>
void ParametricObject<Dim>::setDensity(const std::array<int, Dim> &density) {
    _density = density;
    calculateVerticesAndIndices();
}

#endif //MG1_PARAMETRICOBJECT_H
