//
// Created by Molom on 2022-06-04.
//

#include "gregoryPatch.h"

using namespace std;
using namespace GregoryUtils;

GregoryPatch::GregoryPatch(uint id, const array<shared_ptr<BicubicC0>, 3> &patches, GregoryInfo fillInInfo)
        : Patch(id, "GregoryPatch", {0, 0, 0}, {3, 3}, {1, 1}, false, {}) {
    // TODO: create
}

Type GregoryPatch::type() const {
    return GREGORY;
}

void GregoryPatch::draw(Renderer &renderer, DrawType drawType) {
    // TODO: draw
}
