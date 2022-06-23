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
#include "Objects/Intersection/intersectionInstance.h"

template<size_t Dim>
class ParametricObject : public Object {
public:
    const std::array<int, Dim> &density() const { return _density; }

    void setDensity(const std::array<int, Dim> &density);

    const std::array<std::tuple<float, float>, Dim> &range() const { return _range; }

    virtual std::array<bool, Dim> looped() const = 0;

    virtual DirectX::XMVECTOR value(const std::array<float, Dim> &parameters) = 0;

    virtual DirectX::XMVECTOR tangent(const std::array<float, Dim> &parameters) = 0;

    virtual DirectX::XMVECTOR bitangent(const std::array<float, Dim> &parameters) = 0;

    void setIntersectionInstance(const std::shared_ptr<IntersectionInstance> &instance);

    std::shared_ptr<IntersectionInstance> intersectionInstance() const;

protected:
    ParametricObject(uint id,
                     QString name,
                     DirectX::XMFLOAT3 position,
                     std::array<int, Dim> density,
                     std::array<std::tuple<float, float>, Dim> range,
                     D3D11_PRIMITIVE_TOPOLOGY topology);

    std::weak_ptr<IntersectionInstance> intersection;

private:
    std::array<int, Dim> _density;
    std::array<std::tuple<float, float>, Dim> _range;

    virtual void densityUpdated() {}
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

template<size_t Dim>
void ParametricObject<Dim>::setIntersectionInstance(const std::shared_ptr<IntersectionInstance> &instance) {
    intersection = instance;
}

template<size_t Dim>
std::shared_ptr<IntersectionInstance> ParametricObject<Dim>::intersectionInstance() const {
    return intersection.lock();
}

#endif //MG1_PARAMETRICOBJECT_H
