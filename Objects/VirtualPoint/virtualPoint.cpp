//
// Created by Molom on 04/04/2022.
//

#include "virtualPoint.h"

VirtualPoint::VirtualPoint(const DirectX::XMFLOAT3 &position) : Point(position) {}

Type VirtualPoint::type() const {
    return VIRTUALPOINT3D;
}