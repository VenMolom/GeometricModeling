//
// Created by Molom on 2022-06-04.
//

#include "gregoryUtils.h"

using namespace GregoryUtils;
using namespace std;

GregoryInfo GregoryUtils::checkForGregoryFillIn(const array<shared_ptr<BicubicC0>, 3> &patches) {
    vector<vector<shared_ptr<VirtualPoint>>> points = {
            patches[0]->virtualPoints(), patches[1]->virtualPoints(), patches[2]->virtualPoints()
    };

    array<array<int, 4>, 3> cornerIndex{};
    for (int i = 0; i < 3; ++i) {
        auto size = patches[i]->size();

        if (size[0] != 1 && size[1] != 1) return {false, {}};

        cornerIndex[i][0] = 0;
        cornerIndex[i][1] = size[0] * 3;
        cornerIndex[i][2] = size[1] * 3 * (cornerIndex[i][1] + 1);
        cornerIndex[i][3] = cornerIndex[i][2] + cornerIndex[i][1];
    }

    array<array<shared_ptr<VirtualPoint>, 4>, 3> corners;
    corners[0] = {points[0][cornerIndex[0][0]], points[0][cornerIndex[0][1]],
                  points[0][cornerIndex[0][2]], points[0][cornerIndex[0][3]]};
    corners[1] = {points[1][cornerIndex[1][0]], points[1][cornerIndex[1][1]],
                  points[1][cornerIndex[1][2]], points[1][cornerIndex[1][3]]};
    corners[2] = {points[2][cornerIndex[2][0]], points[2][cornerIndex[2][1]],
                  points[2][cornerIndex[2][2]], points[2][cornerIndex[2][3]]};

    //look for corners
    vector<pair<int, int>> cornersFound;
    for (int i = 0; i < 3; ++i) {
        auto firstCorners = corners[i];
        auto secondCorners = corners[(i + 1) % 3];
        for (int j = 0; j < 4; ++j) {
            auto firstCorner = firstCorners[j];
            for (int k = 0; k < 4; ++k) {
                auto secondCorner = secondCorners[k];

                if (firstCorner->equals(secondCorner)) {
                    cornersFound.emplace_back(cornerIndex[i][j], cornerIndex[(i + 1) % 3][k]);
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

        auto diff = abs(firstCorner - secondCorner);

        if (diff != 3 && diff != cornerIndex[i][2]) return {false, {}};
    }

    return {true, {cornersFound[0], cornersFound[1], cornersFound[2]}};
}

GregoryInfo::GregoryInfo(bool canFill, const array<std::pair<int, int>, 3> &corners)
        : canFill(canFill), corners(corners) {}

GregoryInfo::GregoryInfo() : canFill(false), corners() {

}
