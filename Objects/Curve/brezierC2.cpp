//
// Created by Molom on 2022-04-01.
//

#include "brezierC2.h"

using namespace std;
using namespace DirectX;
using namespace Utils3D;

BrezierC2::BrezierC2(uint id, vector<weak_ptr<Point>> &&points, QBindable<std::weak_ptr<Object>> bindableSelected)
        : BrezierCurve(id, "Brezier C2", std::move(points)),
          VirtualPointsHolder(bindableSelected),
          deBoorPoints() {
    updatePoints();
}

Type BrezierC2::type() const {
    return BREZIERC2;
}

void BrezierC2::draw(Renderer &renderer, DrawType drawType) {
    BrezierCurve::draw(renderer, drawType);

    if (_bernsteinBase && !bernsteinPoints.empty()) {
        for (auto &point: bernsteinPoints) {
            auto isSelected = !selected.expired() && point->equals(selected.lock());
            point->draw(renderer, isSelected ? SELECTED : DEFAULT);
        }
    }
}

void BrezierC2::drawPolygonal(Renderer &renderer, DrawType drawType) {
    if (_bernsteinBase || _bothPolygonals) {
        BrezierCurve::drawPolygonal(renderer, drawType);
    }
    if ((!_bernsteinBase || _bothPolygonals) && !deBoorPoints.vertices().empty()) {
        deBoorPoints.draw(renderer, drawType);
    }
}

void BrezierC2::pointUpdate(const shared_ptr<Point> &point, int index) {
    deBoorPoints.vertices().push_back({point->position(), {1, 1, 1}});

    shared_ptr<Point> right;
    // get next de Boor point
    while (index + 1 < _points.size() && !(right = _points[index + 1].lock())) {
        _points.erase(next(_points.begin(), index + 1));
    }
    if (!right) return;

    XMFLOAT3 b1{}, b2{};
    Utils3D::storeFloat3Lerp(b1, point->position(), right->position(), 1.0f / 3.0f);
    Utils3D::storeFloat3Lerp(b2, point->position(), right->position(), 2.0f / 3.0f);

    if (index == 0) {
        vertices.push_back({b2, {1, 1, 1}});
        addBernsteinPoint(b2);
        return;
    }

    XMFLOAT3 b0{};
    Utils3D::storeFloat3Lerp(b0, vertices.back().position, b1, 0.5f);
    if (index == 1) {
        vertices[0].position = b0;
        bernsteinPoints[0]->setPositionSilently(b0);
    } else {
        vertices.push_back({b0, {1, 1, 1}});
        addBernsteinPoint(b0);
    }
    vertices.push_back({b1, {1, 1, 1}});
    addBernsteinPoint(b1);
    vertices.push_back({b2, {1, 1, 1}});
    addBernsteinPoint(b2);

    auto size = vertices.size();
    indices.push_back(size - 3);
    indices.push_back(size - 2);
    indices.push_back(size - 1);
    indices.push_back(size);
}

void BrezierC2::preUpdate() {
    BrezierCurve::preUpdate();
    bernsteinPoints.clear();
    deBoorPoints.vertices().clear();
    bernsteinPointsHandlers.clear();
}

void BrezierC2::postUpdate() {
    _lastPatchSize = 4;
    if (vertices.size() > 2) {
        vertices.resize(vertices.size() - 2);
        bernsteinPoints.resize(vertices.size());
        indices.resize(indices.size() - 4);
    }

    canDraw = _points.size() >= 4;
    updateBuffers();
    deBoorPoints.update();
}

void BrezierC2::pointMoved(const weak_ptr<Point> &point) {
    if (_points.size() < 4) return;

    shared_ptr<Point> moved;
    if (!(moved = point.lock())) {
        updatePoints();
        return;
    }

    auto it = _points.begin();
    while ((it = std::find_if(it, _points.end(), [&moved](const weak_ptr<Point> &p) {
        return moved.get() == p.lock().get();
    })) != _points.end()) {
        int index = it - _points.begin();
        deBoorMoved(index, moved);
        std::advance(it, 1);
    }
}

const vector<shared_ptr<VirtualPoint>> &BrezierC2::virtualPoints() {
    return bernsteinPoints;
}

void BrezierC2::synchroniseBernsteinPositions(int start, int end) {
    for (; start <= end; ++start) {
        bernsteinPoints[start]->setPositionSilently(vertices[start].position);
    }
}

void BrezierC2::addBernsteinPoint(const DirectX::XMFLOAT3 &position) {
    auto point = make_shared<VirtualPoint>(position);
    weak_ptr<VirtualPoint> weakPoint = point;
    bernsteinPointsHandlers.push_back(point->bindablePosition().addNotifier([this, weakPoint] {
        bernsteinMoved(weakPoint);
    }));
    bernsteinPoints.push_back(std::move(point));
}

void BrezierC2::bernsteinMoved(const weak_ptr<VirtualPoint> &point) {
    if (_points.size() < 4) return;

    shared_ptr<VirtualPoint> moved;
    if (!(moved = point.lock())) {
        updatePoints();
        return;
    }

    auto it = std::find_if(bernsteinPoints.begin(), bernsteinPoints.end(),
                           [&moved](const weak_ptr<VirtualPoint> &p) {
                               return moved.get() == p.lock().get();
                           });

    if (it == bernsteinPoints.end()) {
        updatePoints();
        return;
    }

    int index = it - bernsteinPoints.begin();
    int over = (index + 1) / 3 + 1;
    shared_ptr<Point> deBoorOver;
    if (!(deBoorOver = _points[over].lock())) {
        updatePoints();
        return;
    }

    auto oldPosition = vertices[index].position;
    auto newPosition = moved->position();
    auto value = index % 3 == 0 ? 3.0f / 2.0f : 54.0f / 31.0f;

    auto deBoorOverPosition = deBoorOver->position();
    XMStoreFloat3(&deBoorOverPosition,
                  XMVectorAdd(
                          XMLoadFloat3(&deBoorOverPosition),
                          XMVectorScale(
                                  XMVectorSubtract(XMLoadFloat3(&newPosition),
                                                   XMLoadFloat3(&oldPosition)
                                  ), value
                          )
                  )
    );
    deBoorOver->setPosition(deBoorOverPosition);
}

void BrezierC2::deBoorMoved(int index, const std::shared_ptr<Point> &moved) {
    int under = (index - 1) * 3;
    bool fixedLeft{false}, fixedRight{false};

    deBoorPoints.vertices()[index].position = moved->position();

    shared_ptr<Point> left{}, right{};
    // should have left
    if (index > 0) {
        left = _points[index - 1].lock();
        if (!left) {
            updatePoints();
            return;
        }
    }

    // should have _right
    if (index < _points.size() - 1) {
        right = _points[index + 1].lock();
        if (!right) {
            updatePoints();
            return;
        }
    }

    // fix between self and left neighbour
    if (index > 1 && index < _points.size() - 1) {
        Utils3D::storeFloat3Lerp(vertices[under - 2].position, left->position(), moved->position(), 1.0f / 3.0f);
        Utils3D::storeFloat3Lerp(vertices[under - 1].position, left->position(), moved->position(), 2.0f / 3.0f);
        fixedLeft = true;
    }

    // fix between self and _right neighbour
    if (index > 0 && index < _points.size() - 2) {
        Utils3D::storeFloat3Lerp(vertices[under + 1].position, moved->position(), right->position(), 1.0f / 3.0f);
        Utils3D::storeFloat3Lerp(vertices[under + 2].position, moved->position(), right->position(), 2.0f / 3.0f);
        fixedRight = true;
    }

    // fix under left neighbour
    if (index > 1) {
        XMFLOAT3 b1{}, b2{};

        // second left neighbour is not border
        if (index > 2) {
            b2 = vertices[under - 4].position;
        } else {
            shared_ptr<Point> leftLeft = _points[index - 2].lock();
            if (!leftLeft) {
                updatePoints();
                return;
            }
            Utils3D::storeFloat3Lerp(b2, leftLeft->position(), left->position(), 2.0f / 3.0f);
        }

        if (fixedLeft) {
            b1 = vertices[under - 2].position;
        } else {
            Utils3D::storeFloat3Lerp(b1, left->position(), moved->position(), 1.0f / 3.0f);
        }

        Utils3D::storeFloat3Lerp(vertices[under - 3].position, b1, b2, 0.5f);
    }

    // fix under _right neighbour
    if (index < _points.size() - 2) {
        XMFLOAT3 b1{}, b2{};

        // second _right neighbour is not border
        if (index < _points.size() - 3) {
            b2 = vertices[under + 4].position;
        } else {
            shared_ptr<Point> rightRight = _points[index + 2].lock();
            if (!rightRight) {
                updatePoints();
                return;
            }
            Utils3D::storeFloat3Lerp(b2, right->position(), rightRight->position(), 1.0f / 3.0f);
        }

        if (fixedRight) {
            b1 = vertices[under + 2].position;
        } else {
            Utils3D::storeFloat3Lerp(b1, moved->position(), right->position(), 2.0f / 3.0f);
        }

        Utils3D::storeFloat3Lerp(vertices[under + 3].position, b1, b2, 0.5f);
    }

    // fix under yourself
    if (index > 0 && index < _points.size() - 1) {
        XMFLOAT3 b1{}, b2{};
        if (fixedLeft) {
            b1 = vertices[under - 1].position;
        } else {
            Utils3D::storeFloat3Lerp(b1, left->position(), moved->position(), 2.0f / 3.0f);
        }

        if (fixedRight) {
            b2 = vertices[under + 1].position;
        } else {
            Utils3D::storeFloat3Lerp(b2, moved->position(), right->position(), 1.0f / 3.0f);
        }

        Utils3D::storeFloat3Lerp(vertices[under].position, b1, b2, 0.5f);
    }

    synchroniseBernsteinPositions(max(0, under - 3), min(static_cast<int>(vertices.size()) - 1, under + 3));
    updateBuffers();
    deBoorPoints.update();
}

BrezierC2::BrezierC2(const MG1::Bezier &curve, const list<std::shared_ptr<Object>> &sceneObjects,
                     QBindable<std::weak_ptr<Object>> bindableSelected)
        : BrezierCurve(curve, sceneObjects),
          VirtualPointsHolder(bindableSelected),
          deBoorPoints() {
    updatePoints();
}

MG1::BezierC2 BrezierC2::serialize() {
    MG1::BezierC2 curve{};
    curve.name = name().toStdString();
    curve.SetId(id());

    for (auto &point: _points) {
        if (point.lock())
            curve.controlPoints.emplace_back(point.lock()->id());
    }

    return curve;
}
