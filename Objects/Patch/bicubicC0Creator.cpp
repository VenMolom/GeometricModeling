//
// Created by Molom on 2022-05-14.
//

#include "bicubicC0Creator.h"

using namespace std;
using namespace DirectX;

BicubicC0Creator::BicubicC0Creator(const XMFLOAT3 &position, const QBindable<weak_ptr<Object>> &bindableSelected)
        : BicubicC0(0, "BicubicC0", position, {1, 1}, {1, 1}, false, bindableSelected) {}

void BicubicC0Creator::draw(Renderer &renderer, DrawType drawType) {
    renderer.draw(*this, SELECTED_COLOR);
    Cursor::drawCursor(renderer, position(), rotation());
}

Type BicubicC0Creator::type() const {
    return PATCHC0CREATOR;
}

void BicubicC0Creator::setSize(const array<float, PATCH_DIM> &size) {
    this->size = size;
    createSegments(segments, size);
}

void BicubicC0Creator::setSegments(const array<int, PATCH_DIM> &segments) {
    this->segments = segments;
    createSegments(segments, size);
}

void BicubicC0Creator::setCylinder(bool cylinder) {
    this->cylinder = cylinder;
    createSegments(segments, size);
}

std::shared_ptr<Object> BicubicC0Creator::create(uint id) {
    auto patch = make_shared<BicubicC0>(id, name(), _position.value(), segments, size, cylinder, bindableSelected);
    patch->setScale(_scale.value());
    patch->setRotation(_rotation.value());
    patch->setDensity(density());
    return patch;
}

