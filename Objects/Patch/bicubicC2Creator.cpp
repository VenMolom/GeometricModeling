//
// Created by Molom on 2022-05-21.
//

#include "bicubicC2Creator.h"

using namespace std;
using namespace DirectX;

BicubicC2Creator::BicubicC2Creator(const XMFLOAT3 &position, const QBindable<weak_ptr<Object>> &bindableSelected)
        : BicubicC2(0, "BicubicC2", position, {1, 1}, {1, 1}, false, bindableSelected) {}

void BicubicC2Creator::draw(Renderer &renderer, DrawType drawType) {
    renderer.draw(*this, SELECTED_COLOR);
    Cursor::drawCursor(renderer, position(), rotation());
}

Type BicubicC2Creator::type() const {
    return PATCHC2CREATOR;
}

void BicubicC2Creator::setSize(const array<float, PATCH_DIM> &size) {
    this->size = size;
    createSegments(segments, size);
}

void BicubicC2Creator::setSegments(const array<int, PATCH_DIM> &segments) {
    this->segments = segments;
    createSegments(segments, size);
}

void BicubicC2Creator::setCylinder(bool cylinder) {
    this->cylinder = cylinder;
    createSegments(segments, size);
}

std::shared_ptr<Object> BicubicC2Creator::create(uint id) {
    auto patch = make_shared<BicubicC2>(id, name(), _position.value(), segments, size, cylinder, bindableSelected);
    patch->setScale(_scale.value());
    patch->setRotation(_rotation.value());
    patch->setDensity(density());
    return patch;
}