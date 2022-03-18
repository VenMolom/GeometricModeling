//
// Created by Molom on 2022-03-18.
//

#ifndef MG1_CURSOR_H
#define MG1_CURSOR_H


#include <DirectXMath.h>
#include "Objects/Object/object.h"

class Cursor : public Object {
public:
    Cursor(DirectX::XMFLOAT3 position, DirectX::XMFLOAT2 screenPosition);

    void draw(Renderer &renderer, const DirectX::XMMATRIX &camera) const override;

    Type type() const override;

    DirectX::XMFLOAT2 screenPosition() const { return _screenPosition; }

    void setScreenPosition(DirectX::XMFLOAT2 position);

private:
    static const std::vector<VertexPositionColor> vertices;
    DirectX::XMFLOAT2 _screenPosition;
    QPropertyNotifier positionHandler;

    void updatePosition();
};


#endif //MG1_CURSOR_H
