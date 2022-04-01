//
// Created by Molom on 2022-04-01.
//

#include "brezierC2.h"

using namespace std;
using namespace DirectX;

BrezierC2::BrezierC2(vector<weak_ptr<Point>> &&points)
        : BrezierCurve("Brezier C2", std::move(points)) {
    updatePoints();
}

Type BrezierC2::type() const {
    return BREZIERC2;
}

void BrezierC2::updatePoints() {
    vertices.clear();
    indices.clear();
    pointsHandlers.clear();
    std::shared_ptr<Point> point;
    for (int i = 0; i < _points.size(); ++i) {
        if (!(point = _points[i].lock())) {
            _points.erase(next(_points.begin(), i));
            --i;
            continue;
        }

        pointsHandlers.push_back(point->bindablePosition().addNotifier([&] { updatePoints(); }));

        if (indices.size() > 0 && indices.size() % 4 == 0) {
            indices.push_back(i - 1);
        }

        vertices.push_back({point->position(), {1, 1, 1}});
        indices.push_back(i);

        min = newMin(min, point->position());
        max = newMax(max, point->position());
    }

    if (vertices.size() > 1) {
        lastPatchSize = indices.size() % 4;
        if (lastPatchSize == 0) lastPatchSize = 4;

        while (indices.size() % 4 != 0) indices.push_back(vertices.size() - 1);
    }

    pointsChanged.setValue(pointsChanged.value() + 1);
}