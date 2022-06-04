//
// Created by Molom on 2022-03-18.
//

#ifndef MG1_UTILS3D_H
#define MG1_UTILS3D_H

#include "camera.h"

namespace Utils3D {
    struct XMFLOAT3RAY {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 direction;
    };

    XMFLOAT3RAY getRayFromScreen(DirectX::XMINT2 screenPosition, const std::shared_ptr<Camera>& camera);

    DirectX::XMFLOAT4 getPerpendicularPlaneThroughPoint(DirectX::XMFLOAT3 direction, DirectX::XMFLOAT3 point);

    DirectX::XMFLOAT3 getRayCrossWithPlane(XMFLOAT3RAY ray, DirectX::XMFLOAT4 plane);

    void storeFloat3Lerp(DirectX::XMFLOAT3 &target, const DirectX::XMFLOAT3 &v1, const DirectX::XMFLOAT3 &v2, float t);
}

#endif //MG1_UTILS3D_H
