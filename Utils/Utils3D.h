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

    XMFLOAT3RAY getRayFromScreen(DirectX::XMFLOAT2 screenPosition, DirectX::XMFLOAT2 screenSize, float near, float far,
                                 DirectX::XMMATRIX projection, DirectX::XMMATRIX view) {
        auto mouseNear = DirectX::XMVectorSet(screenPosition.x, screenPosition.y, 0.0f, 0.0f);
        auto mouseFar = DirectX::XMVectorSet(screenPosition.x, screenPosition.y, 1.0f, 0.0f);
        auto worldNear = DirectX::XMVector3Unproject(mouseNear, 0, 0,
                                                     screenSize.x, screenSize.y,
                                                     near, far, projection,
                                                     view, DirectX::XMMatrixIdentity());
        auto worldFar = DirectX::XMVector3Unproject(mouseFar, 0, 0,
                                                    screenSize.x, screenSize.y,
                                                    near, far, projection,
                                                    view, DirectX::XMMatrixIdentity());
        XMFLOAT3RAY result{};
        DirectX::XMStoreFloat3(&result.position, mouseNear);
        DirectX::XMStoreFloat3(&result.direction, DirectX::XMVector3Normalize(
                DirectX::XMVectorSubtract(worldFar, worldNear)));
        return result;
    }
}

#endif //MG1_UTILS3D_H
