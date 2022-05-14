//
// Created by Molom on 04/04/2022.
//

#ifndef MG1_VIRTUALPOINT_H
#define MG1_VIRTUALPOINT_H


#include "Objects/Point/point.h"

class VirtualPoint : public Point {
public:
    explicit VirtualPoint(const DirectX::XMFLOAT3 &position);

    void setPositionSilently(DirectX::XMFLOAT3 position);

    Type type() const override;
};


#endif //MG1_VIRTUALPOINT_H
