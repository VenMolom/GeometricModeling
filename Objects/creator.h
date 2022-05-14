//
// Created by Molom on 2022-05-14.
//

#ifndef MG1_CREATOR_H
#define MG1_CREATOR_H

#include "object.h"

class Creator {
public:
    virtual std::shared_ptr<Object> create(uint id) = 0;
};

#endif //MG1_CREATOR_H
