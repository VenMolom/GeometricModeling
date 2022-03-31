//
// Created by Molom on 2022-03-25.
//

#include <DirectXMath.h>

#include <utility>
#include "brezierC0.h"

using namespace std;
using namespace DirectX;

BrezierC0::BrezierC0(vector<weak_ptr<Point>> points)
        : BrezierCurve("Brezier C0", std::move(points)) {
    updatePoints();
}

Type BrezierC0::type() const {
    return BREZIERC0;
}

void BrezierC0::updatePoints() {
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

XMFLOAT3 BrezierC0::newMax(XMFLOAT3 oldMax, XMFLOAT3 candidate) {
    return {max(oldMax.x, candidate.x), max(oldMax.y, candidate.y), max(oldMax.z, candidate.z)};
}

XMFLOAT3 BrezierC0::newMin(XMFLOAT3 oldMin, XMFLOAT3 candidate) {
    return {min(oldMin.x, candidate.x), min(oldMin.y, candidate.y), min(oldMin.z, candidate.z)};
}
