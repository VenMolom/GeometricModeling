//
// Created by Molom on 2022-03-25.
//

#include "brezierC0.h"

using namespace std;
using namespace DirectX;

BrezierC0::BrezierC0(vector<weak_ptr<Point>> &&points)
        : BrezierCurve("Brezier C0", std::move(points)) {
    updatePoints();
}

Type BrezierC0::type() const {
    return BREZIERC0;
}