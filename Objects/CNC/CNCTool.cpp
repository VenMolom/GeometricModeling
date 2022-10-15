//
// Created by Molom on 15/10/2022.
//

#include "CNCTool.h"

using namespace DirectX;

CNCTool::CNCTool(XMFLOAT3 position)
        : Object(0, "", position, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST) {
    // TODO: calculate cylinder vertices
    // TODO: calculate flat cap vertices
    // TODO: calculate round cap vertices
}

void CNCTool::setType(CNCType type) {
    _type = type;
    // TODO: change model
}

void CNCTool::setSize(int size) {
    _size = size;
    setScale(XMFLOAT3(_size, _size, 1.f));

    // TODO: scale round cap
}

void CNCTool::draw(Renderer &renderer, DrawType drawType) {
    // TODO: add override to draw
    renderer.draw(*this, drawType != DEFAULT ? SELECTED_COLOR : DEFAULT_COLOR);
    // TODO: draw cylinder
    // TODO: draw top cap
    // TODO: draw bottom cap
}

Type CNCTool::type() const {
    return VIRTUAL;
}
