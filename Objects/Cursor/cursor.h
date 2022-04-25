//
// Created by Molom on 2022-03-18.
//

#ifndef MG1_CURSOR_H
#define MG1_CURSOR_H

#include "Objects/object.h"
#include "camera.h"
#include "Utils/Utils3D.h"

class Cursor : public Object {
public:
    static void drawCursor(Renderer &renderer, const DirectX::XMFLOAT3 &position, const DirectX::XMFLOAT3 &rotation);

    Cursor(DirectX::XMFLOAT3 position, DirectX::XMINT2 screenPosition, std::shared_ptr<Camera> camera);

    void draw(Renderer &renderer, DrawType drawType) override;

    Type type() const override;

    DirectX::XMINT2 screenPosition() const { return _screenPosition; }

    void setScreenPosition(DirectX::XMINT2 position);

    QBindable<DirectX::XMINT2> bindableScreenPosition() { return &_screenPosition; }

    void setScale(DirectX::XMFLOAT3 scale) override {}

private:
    const static std::vector<VertexPositionColor> cursorVertices;

    static std::unique_ptr<Cursor> instance;

    std::shared_ptr<Camera> camera;

    QProperty<DirectX::XMINT2> _screenPosition;
    QPropertyNotifier positionHandler;
    QPropertyNotifier viewHandler;
    QPropertyNotifier projectionHandler;

    void updateScreenPosition();
};


#endif //MG1_CURSOR_H
