//
// Created by Molom on 14/10/2022.
//

#ifndef MG1_CNCROUTER_H
#define MG1_CNCROUTER_H

#include "Objects/object.h"
#include "Objects/updatable.h"
#include "Utils/fileParser.h"
#include "Objects/linestrip.h"

class CNCRouter: public Object, public Updatable {
public:
    CNCRouter(uint id, DirectX::XMFLOAT3 position);

    void setPosition(DirectX::XMFLOAT3 position) override;

    void setRotation(DirectX::XMFLOAT3 rotation) override {}

    void setScale(DirectX::XMFLOAT3 scale) override {}

    void draw(Renderer &renderer, DrawType drawType) override;

    Type type() const override;

    void update(float frameTime) override;

    void loadPath(CNCPath&& path);

private:
    CNCPath routerPath;
    Linestrip drawPaths;

    void fillDrawPaths();
};


#endif //MG1_CNCROUTER_H
