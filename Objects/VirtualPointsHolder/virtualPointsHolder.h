//
// Created by Molom on 2022-04-06.
//

#ifndef MG1_VIRTUALPOINTSHOLDER_H
#define MG1_VIRTUALPOINTSHOLDER_H

#include "Objects/VirtualPoint/virtualPoint.h"

class VirtualPointsHolder {
public:
    virtual const std::vector<std::shared_ptr<VirtualPoint>> &virtualPoints() = 0;
};


#endif //MG1_VIRTUALPOINTSHOLDER_H
