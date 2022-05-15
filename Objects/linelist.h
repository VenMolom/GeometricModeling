//
// Created by Molom on 2022-05-15.
//

#ifndef MG1_LINELIST_H
#define MG1_LINELIST_H

#include "object.h"

class Linelist : public Object {
public:
    Linelist();

    std::vector<VertexPositionColor> &vertices() { return Renderable::vertices; };

    std::vector<Index> &indices() { return Renderable::indices; };

    void addLine(Index start, Index end);

    void update() { updateBuffers(); }

    void draw(Renderer &renderer, DrawType drawType) override;

    Type type() const override;
};


#endif //MG1_LINELIST_H
