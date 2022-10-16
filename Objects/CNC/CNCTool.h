//
// Created by Molom on 15/10/2022.
//

#ifndef MG1_CNCTOOL_H
#define MG1_CNCTOOL_H

#include "Utils/fileParser.h"
#include "Objects/CNC/mesh.h"
#include <DirectXMath.h>

class CNCTool {
    static constexpr float HEIGHT = 10.f;
    static const DirectX::XMFLOAT3 COLOR;

public:
    explicit CNCTool(DirectX::XMFLOAT3 position);

    CNCType endType() const { return _type; }

    void setType(CNCType type);

    int size() const { return _size * 10.f; }

    void setSize(int size);

    void draw(Renderer &renderer, DirectX::XMMATRIX parentMatrix);

    DirectX::XMFLOAT3 position() { return _position; }

    void setPosition(DirectX::XMFLOAT3 position);

private:
    Mesh cylinder, topCap, bottomCap, sphericalCap;
    DirectX::XMFLOAT3 _position;
    DirectX::XMFLOAT4X4 positionMatrix;
    DirectX::XMFLOAT4X4 scaleMatrix, sphericalScaleMatrix;
    CNCType _type{static_cast<CNCType>(0)};
    float _size{1.f};
};


#endif //MG1_CNCTOOL_H
