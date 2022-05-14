//
// Created by Molom on 2022-05-14.
//

#ifndef MG1_PATCHCREATOR_H
#define MG1_PATCHCREATOR_H

#include "Objects/creator.h"

class PatchCreator : public Creator {
public:
    virtual void setSegments(const std::array<int, 2> &segments) = 0;

    virtual void setSize(const std::array<float, 2> &size) = 0;

    virtual void setCylinder(bool cylinder) = 0;
};


#endif //MG1_PATCHCREATOR_H
