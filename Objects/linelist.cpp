//
// Created by Molom on 2022-05-15.
//

#include "linelist.h"

Linelist::Linelist() : Object(0, "", {0, 0, 0}, D3D11_PRIMITIVE_TOPOLOGY_LINELIST) {

}

void Linelist::draw(Renderer &renderer, DrawType drawType) {
    if (Renderable::vertices.empty()) return;

    renderer.draw(*this, drawType != DEFAULT ? POLYGONAL_COLOR : DEFAULT_COLOR);
}

Type Linelist::type() const {
    return VIRTUAL;
}

void Linelist::addLine(Index start, Index end) {
    indices().push_back(start);
    indices().push_back(end);
}
