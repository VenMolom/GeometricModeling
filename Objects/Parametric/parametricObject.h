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
    ParametricObject(uint id,
                     QString name,
                     DirectX::XMFLOAT3 position,
                     std::array<int, Dim> density,
                     std::array<std::tuple<float, float>, Dim> range,
                     D3D11_PRIMITIVE_TOPOLOGY topology);

public:
    const std::array<int, Dim> &density() const { return _density; }

    void setDensity(const std::array<int, Dim> &density);

    const std::array<std::tuple<float, float>, Dim> &range() const { return _range; }

    virtual std::array<bool, Dim> looped() const = 0;

private:
    std::array<int, Dim> _density;
    std::array<std::tuple<float, float>, Dim> _range;

    virtual void densityUpdated() = 0;
};

template<size_t Dim>
ParametricObject<Dim>::ParametricObject(uint id,
                                        QString name,
                                        DirectX::XMFLOAT3 position,
                                        std::array<int, Dim> density,
                                        std::array<std::tuple<float, float>, Dim> range,
                                        D3D11_PRIMITIVE_TOPOLOGY topology)
        : Object(id, name, position, topology),
          _density(density),
          _range(range) {
}

template<size_t Dim>
void ParametricObject<Dim>::setDensity(const std::array<int, Dim> &density) {
    _density = density;
    densityUpdated();
}

#endif //MG1_PARAMETRICOBJECT_H
