//
// Created by Molom on 2022-03-18.
//

#ifndef MG1_CURSOR_H
#define MG1_CURSOR_H

#include "Objects/Object/object.h"
#include "Camera/camera.h"
#include "Utils/Utils3D.h"

class Cursor : public Object {
public:
    Cursor(DirectX::XMFLOAT3 position, DirectX::XMINT2 screenPosition, Camera &camera);

    void draw(Renderer &renderer, const DirectX::XMMATRIX &camera) const override;

    Type type() const override;

    DirectX::XMINT2 screenPosition() const { return _screenPosition; }

    void setScreenPosition(DirectX::XMINT2 position);

    QBindable<DirectX::XMINT2> bindableScreenPosition() { return &_screenPosition; }

private:
    Camera &camera;

    static const std::vector<VertexPositionColor> vertices;
    QProperty<DirectX::XMINT2> _screenPosition;
    QPropertyNotifier positionHandler;
    QPropertyNotifier viewHandler;
    QPropertyNotifier projectionHandler;

    void updateScreenPosition();
};


#endif //MG1_CURSOR_H
