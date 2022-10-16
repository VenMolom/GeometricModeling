//
// Created by Molom on 2022-03-11.
//

#ifndef MG1_RENDERER_H
#define MG1_RENDERER_H

#include <DirectXMath.h>
#include "DirectX/DXStructures/dxStructures.h"

class Object;

class Torus;

class BrezierCurve;

class InterpolationCurveC2;

class Grid;

class Point;

class Patch;

class BicubicC2;

class GregoryPatch;

class IntersectionInstance;

class Mesh;

const DirectX::XMFLOAT4 SELECTED_COLOR{1.0f, 0.4f, 0.0f, 1.0f};
const DirectX::XMFLOAT4 DEFAULT_COLOR{0.0f, 0.0f, 0.0f, 0.0f};
const DirectX::XMFLOAT4 POLYGONAL_COLOR{0.0f, 1.0f, 1.0f, 1.0f};

class Renderer {
public:
    virtual void draw(const Object &object, DirectX::XMFLOAT4 color) = 0;

    virtual void draw(const Torus &torus, DirectX::XMFLOAT4 color) = 0;

    virtual void draw(const BrezierCurve &curve, DirectX::XMFLOAT4 color) = 0;

    virtual void draw(const InterpolationCurveC2 &curve, DirectX::XMFLOAT4 color) = 0;

    virtual void draw(const Grid &grid, DirectX::XMFLOAT4 color) = 0;

    virtual void draw(const Point &point, DirectX::XMFLOAT4 color) = 0;

    virtual void draw(const Patch &patch, DirectX::XMFLOAT4 color) = 0;

    virtual void draw(const BicubicC2 &patch, DirectX::XMFLOAT4 color) = 0;

    virtual void draw(const GregoryPatch &patch, DirectX::XMFLOAT4 color) = 0;

    virtual void draw(const IntersectionInstance &instance, bool clear = true) = 0;

    virtual void draw(const Mesh &mesh, DirectX::XMMATRIX modelMatrix) = 0;
};

#endif //MG1_RENDERER_H