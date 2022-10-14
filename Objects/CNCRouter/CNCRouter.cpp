//
// Created by Molom on 14/10/2022.
//

#include "CNCRouter.h"

CNCRouter::CNCRouter(uint id, DirectX::XMFLOAT3 position)
 : Object(id, "Router", position, D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST){

}

Type CNCRouter::type() const {
    return CNCROUTER;
}

void CNCRouter::draw(Renderer &renderer, DrawType drawType) {
    // TODO: implement
}

void CNCRouter::update(float frameTime) {
    // TODO: implement
}