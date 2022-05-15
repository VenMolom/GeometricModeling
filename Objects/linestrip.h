//
// Created by Molom on 2022-04-14.
//

#ifndef MG1_LINESTRIP_H
#define MG1_LINESTRIP_H

#include "object.h"

class Linestrip : public Object {
public:
    Linestrip();

    std::vector<VertexPositionColor> &vertices() { return Renderable::vertices; };

    void update() { updateBuffers(); }

    void draw(Renderer &renderer, DrawType drawType) override;

    Type type() const override;
};


#endif //MG1_LINESTRIP_H
