//
// Created by Molom on 04/04/2022.
//

#include "virtualPoint.h"

using namespace DirectX;

VirtualPoint::VirtualPoint(const XMFLOAT3 &position) : Point(0, position) {}

Type VirtualPoint::type() const {
    return VIRTUALPOINT3D;
}

void VirtualPoint::setPositionSilently(XMFLOAT3 position) {
    _position.setValueBypassingBindings(position);
    calculateModel();
}