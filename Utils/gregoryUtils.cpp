//
// Created by Molom on 2022-06-04.
//

#include "gregoryUtils.h"

using namespace GregoryUtils;
using namespace std;

GregoryInfo GregoryUtils::checkForGregoryFillIn(const array<shared_ptr<BicubicC0>, 3> &patches) {
    array<array<pair<int, int>, 4>, 3> cornerIndex{};
    for (int i = 0; i < 3; ++i) {
        auto size = patches[i]->size();

        if (size[0] != 1 && size[1] != 1) return {false, {}};

        cornerIndex[i][0] = make_pair<int, int>(0, 0);
        cornerIndex[i][1] = make_pair<int, int>(size[0] * 3, 0);
        cornerIndex[i][2] = make_pair<int, int>(0, size[1] * 3);
        cornerIndex[i][3] = make_pair<int, int>(size[0] * 3, size[1] * 3);
    }

    //look for corners
    vector<pair<pair<int, int>, pair<int, int>>> cornersFound;
    for (int i = 0; i < 3; ++i) {
        auto ii = (i + 1) % 3;
        for (int j = 0; j < 4; ++j) {
            auto firstCorner = patches[i]->pointAt(cornerIndex[i][j]);
            for (int k = 0; k < 4; ++k) {
                auto secondCorner = patches[ii]->pointAt(cornerIndex[ii][k]);
                if (firstCorner->equals(secondCorner)) {
                    cornersFound.emplace_back(make_pair(cornerIndex[i][j], cornerIndex[ii][k]));
                }
            }
        }
    }

    // not exactly 3 corners found
    if (cornersFound.size() != 3) return {false, {}};

    // distance between corners is not one segment
    for (int i = 0; i < 3; ++i) {
        auto firstCorner = cornersFound[i].first;
        auto secondCorner = cornersFound[(i + 2) % 3].second;

        auto uDiff = abs(firstCorner.first - secondCorner.first);
        auto vDiff = abs(firstCorner.second - secondCorner.second);

        if (!((uDiff == 0 && vDiff == 3) || (uDiff == 3 && vDiff == 0))) return {false, {}};
    }

    return {true, {cornersFound[0], cornersFound[1], cornersFound[2]}};
}

GregoryInfo::GregoryInfo(bool canFill, const array<pair<pair<int, int>, pair<int, int>>, 3> &corners)
        : canFill(canFill), corners(corners) {}

GregoryInfo::GregoryInfo() : canFill(false), corners() {

}
