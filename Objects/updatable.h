//
// Created by Molom on 14/10/2022.
//

#ifndef MG1_UPDATABLE_H
#define MG1_UPDATABLE_H

#endif //MG1_UPDATABLE_H

#include "DIrectX/renderer.h"

class Updatable {
public:
    virtual void update(Renderer& renderer, float frameTime) = 0;
};