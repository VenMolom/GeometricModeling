//
// Created by Molom on 2022-06-04.
//

#include "gregoryPatch.h"
#include "Utils/utils3D.h"

using namespace std;
using namespace GregoryUtils;
using namespace Utils3D;
using namespace DirectX;

GregoryPatch::GregoryPatch(uint id, const array<shared_ptr<BicubicC0>, 3> &patches, GregoryInfo fillInInfo)
        : Patch(id, "GregoryPatch", {0, 0, 0}, {3, 3}, {1, 1}, false, {},
                D3D11_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST),
          patches(patches),
          fillInInfo(fillInInfo) {
    setHandlers();
    createPoints();
    calculateMeshIndices(segments, bezierMesh);
}

Type GregoryPatch::type() const {
    return GREGORY;
}

void GregoryPatch::calculateMeshIndices(std::array<int, 2> segments, Linelist &linelist) {
    // TODO: calculate vectors mesh
}

void GregoryPatch::drawMesh(Renderer &renderer, DrawType drawType) {
    //renderer.draw(*this, drawType != DEFAULT ? SELECTED_COLOR : DEFAULT_COLOR);
}

void GregoryPatch::createPoints() {
    // points - [patch][depth][bezier_index]
    array<array<array<shared_ptr<VirtualPoint>, 4>, 2>, 3> B;
    array<array<array<shared_ptr<VirtualPoint>, 3>, 2>, 3> R;
    array<array<array<shared_ptr<VirtualPoint>, 2>, 2>, 3> S;
    array<array<array<shared_ptr<VirtualPoint>, 1>, 2>, 3> T;
    // inside points [patch][bezier_index}
    array<array<shared_ptr<VirtualPoint>, 4>, 3> P;
    array<shared_ptr<VirtualPoint>, 3> a;
    shared_ptr<VirtualPoint> P0;

    // patch
    for (int i = 0; i < 3; ++i) {
        // fill B
        auto firstCorner = fillInInfo.corners[i].first;
        auto secondCorner = fillInInfo.corners[(i + 2) % 3].second;
        auto uDiff = (secondCorner.first - firstCorner.first) / 3;
        auto vDiff = (secondCorner.second - firstCorner.second) / 3;

        for (int k = 0; k < 2; k++) {
            B[i][k][0] = patches[i]->pointAt(firstCorner);
            B[i][k][1] = patches[i]->pointAt(make_pair(firstCorner.first + uDiff, firstCorner.second + vDiff));
            B[i][k][2] = patches[i]->pointAt(make_pair(firstCorner.first + 2 * uDiff, firstCorner.second + 2 * vDiff));
            B[i][k][3] = patches[i]->pointAt(secondCorner);

            if (k == 0) {
                if (uDiff == 0) {
                    auto diff = (firstCorner.first == 0 ? 1 : -1);
                    firstCorner.first += diff;
                    secondCorner.first += diff;
                } else {
                    auto diff = (firstCorner.second == 0 ? 1 : -1);
                    firstCorner.second += diff;
                    secondCorner.second += diff;
                }
            }
        }

        // fill R
        for (int k = 0; k < 2; ++k) {
            for (int j = 0; j < 3; ++j) {
                XMFLOAT3 pos;
                storeFloat3Lerp(pos, B[i][k][j]->position(), B[i][k][j + 1]->position(), 0.5f);
                R[i][k][j] = make_shared<VirtualPoint>(pos);
            }
        }

        // fill S
        for (int k = 0; k < 2; ++k) {
            for (int j = 0; j < 2; ++j) {
                XMFLOAT3 pos;
                storeFloat3Lerp(pos, R[i][k][j]->position(), R[i][k][j + 1]->position(), 0.5f);
                S[i][k][j] = make_shared<VirtualPoint>(pos);
            }
        }

        // fill T
        for (int k = 0; k < 2; ++k) {
            XMFLOAT3 pos;
            storeFloat3Lerp(pos, S[i][k][0]->position(), S[i][k][1]->position(), 0.5f);
            T[i][k][0] = make_shared<VirtualPoint>(pos);
        }

        // fill P
        XMFLOAT3 pos, t0Pos = T[i][0][0]->position(), t1Pos = T[i][1][0]->position();
        auto t0 = XMLoadFloat3(&t0Pos);
        auto t1 = XMLoadFloat3(&t1Pos);
        XMStoreFloat3(&pos, XMVectorAdd(t0, XMVectorSubtract(t0, t1)));
        P[i][3] = make_shared<VirtualPoint>(t0Pos);
        P[i][2] = make_shared<VirtualPoint>(pos);

        // a
        XMFLOAT3 p3Pos = t0Pos, p2Pos = pos;
        auto p3 = XMLoadFloat3(&p3Pos);
        auto p2 = XMLoadFloat3(&p2Pos);
        XMStoreFloat3(&pos, XMVectorAdd(p3, XMVectorScale(XMVectorSubtract(p2, p3), 1.5f)));
        a[i] = make_shared<VirtualPoint>(pos);
    }

    XMFLOAT3 p0Pos, a0Pos = a[0]->position(), a1Pos = a[1]->position(), a2Pos = a[2]->position();
    auto a0 = XMLoadFloat3(&a0Pos);
    auto a1 = XMLoadFloat3(&a1Pos);
    auto a2 = XMLoadFloat3(&a2Pos);

    XMStoreFloat3(&p0Pos, XMVectorScale(XMVectorAdd(XMVectorAdd(a0, a1), a2), 1.f / 3.f));
    P0 = make_shared<VirtualPoint>(p0Pos);

    for (int i = 0; i < 3; ++i) {
        XMFLOAT3 pos;
        storeFloat3Lerp(pos, p0Pos, a[i]->position(), 2.f/ 3.f);
        P[i][1] = make_shared<VirtualPoint>(pos);

        // edges
        for (int k = 0; k < 2; ++k){
            bezierMesh.vertices().push_back({B[i][k][0]->position(), {1, 1, 1}});
            bezierMesh.vertices().push_back({R[i][k][0]->position(), {1, 1, 1}});
            bezierMesh.vertices().push_back({S[i][k][0]->position(), {1, 1, 1}});
            bezierMesh.vertices().push_back({T[i][k][0]->position(), {1, 1, 1}});
            bezierMesh.vertices().push_back({S[i][k][1]->position(), {1, 1, 1}});
            bezierMesh.vertices().push_back({R[i][k][2]->position(), {1, 1, 1}});
            bezierMesh.vertices().push_back({B[i][k][3]->position(), {1, 1, 1}});

            // border
            bezierMesh.addLine(i * 17 + k * 7,     i * 17 + k * 7 + 1);
            bezierMesh.addLine(i * 17 + k * 7 + 1, i * 17 + k * 7 + 2);
            bezierMesh.addLine(i * 17 + k * 7 + 2, i * 17 + k * 7 + 3);
            bezierMesh.addLine(i * 17 + k * 7 + 3, i * 17 + k * 7 + 4);
            bezierMesh.addLine(i * 17 + k * 7 + 4, i * 17 + k * 7 + 5);
            bezierMesh.addLine(i * 17 + k * 7 + 5, i * 17 + k * 7 + 6);
        }

        auto index = bezierMesh.vertices().size();
        bezierMesh.vertices().push_back({P[i][3]->position(), {1, 1, 1}});
        bezierMesh.vertices().push_back({P[i][2]->position(), {1, 1, 1}});
        bezierMesh.vertices().push_back({P[i][1]->position(), {1, 1, 1}});
        bezierMesh.addLine(index,     index + 1);
        bezierMesh.addLine(index + 1, index + 2);
    }

    auto index = bezierMesh.vertices().size();
    bezierMesh.vertices().push_back({P0->position(), {1, 1, 1}});
    bezierMesh.addLine(index, 16);
    bezierMesh.addLine(index, 33);
    bezierMesh.addLine(index, 50);

    bezierMesh.update();
}

void GregoryPatch::setHandlers() {
    // TODO: store position changed handles for patches' border points
}

void GregoryPatch::clear() {
    vertices.clear();
    indices.clear();
    points.clear();
    bezierMesh.vertices().clear();
    bezierMesh.indices().clear();
}