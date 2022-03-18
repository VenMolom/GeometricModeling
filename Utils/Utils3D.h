//
// Created by Molom on 2022-03-18.
//

#ifndef MG1_UTILS3D_H
#define MG1_UTILS3D_H

#include <DirectXMath.h>

namespace Utils3D {
    struct XMFLOAT3RAY {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 direction;
    };

    XMFLOAT3RAY
    getRayFromScreen(DirectX::XMINT2 screenPosition, DirectX::XMFLOAT2 screenSize, float nearZ, float farZ,
                     DirectX::XMMATRIX projection, DirectX::XMMATRIX view);

    DirectX::XMFLOAT4 getPerpendicularPlaneThroughPoint(DirectX::XMFLOAT3 direction, DirectX::XMFLOAT3 point);

    DirectX::XMFLOAT3 getRayCrossWithPlane(XMFLOAT3RAY ray, DirectX::XMFLOAT4 plane);
}

#endif //MG1_UTILS3D_H
