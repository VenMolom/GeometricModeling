//
// Created by Molom on 2022-04-30.
//

#ifndef MG1_STEREOSCOPICRENDERER_H
#define MG1_STEREOSCOPICRENDERER_H

#include <DirectXMath.h>

class StereoscopicRenderer {
public:
    virtual void enableStereoscopy(bool enable) = 0;

    virtual void setLeftEyeColor(DirectX::XMFLOAT3 color) = 0;

    virtual void setRightEyeColor(DirectX::XMFLOAT3 color) = 0;
};

#endif //MG1_STEREOSCOPICRENDERER_H
