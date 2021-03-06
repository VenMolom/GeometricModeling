//
// Created by Molom on 2022-03-25.
//

#include "brezierC0.h"

using namespace std;
using namespace DirectX;

BrezierC0::BrezierC0(uint id, vector<weak_ptr<Point>> &&points)
        : BrezierCurve(id, "Brezier C0", std::move(points)) {
    updatePoints();
}

Type BrezierC0::type() const {
    return BREZIERC0;
}

BrezierC0::BrezierC0(const MG1::Bezier &curve, const list<std::shared_ptr<Object>> &sceneObjects)
        : BrezierCurve(curve, sceneObjects) {
    updatePoints();
}

MG1::BezierC0 BrezierC0::serialize() {
    return MG1::BezierC0(Curve::serialize());
}