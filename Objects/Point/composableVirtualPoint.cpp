//
// Created by Molom on 2022-05-16.
//

#include "composableVirtualPoint.h"

ComposableVirtualPoint::ComposableVirtualPoint(const DirectX::XMFLOAT3 &position) : VirtualPoint(position) {
}

Type ComposableVirtualPoint::type() const {
    return COMPOSABLEVIRTUALPOINT3D;
}