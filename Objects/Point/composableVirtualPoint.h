//
// Created by Molom on 2022-05-16.
//

#ifndef MG1_COMPOSABLEVIRTUALPOINT_H
#define MG1_COMPOSABLEVIRTUALPOINT_H

#include "virtualPoint.h"

class ComposableVirtualPoint : public VirtualPoint {
public:
    explicit ComposableVirtualPoint(const DirectX::XMFLOAT3 &position);

    Type type() const override;
};


#endif //MG1_COMPOSABLEVIRTUALPOINT_H
