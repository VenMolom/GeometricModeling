//
// Created by Molom on 2022-06-04.
//

#ifndef MG1_GREGORYUTILS_H
#define MG1_GREGORYUTILS_H


#include "Objects/Patch/bicubicC0.h"

namespace GregoryUtils {
    struct GregoryInfo {
        bool canFill;
        std::array<std::pair<std::pair<int, int>, std::pair<int, int>>, 3> corners;

        GregoryInfo();
        GregoryInfo(bool canFill, const std::array<std::pair<std::pair<int, int>, std::pair<int, int>>, 3> &corners);
    };

    GregoryInfo checkForGregoryFillIn(const std::array<std::shared_ptr<BicubicC0>, 3> &patches);
}

#endif //MG1_GREGORYUTILS_H
