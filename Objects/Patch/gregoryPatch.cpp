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
    // edge points [patch][depth][bezier_index]
    array<array<array<shared_ptr<VirtualPoint>, 4>, 2>, 3> B;
    array<array<array<XMFLOAT3, 3>, 2>, 3> R{};
    array<array<array<XMFLOAT3, 2>, 2>, 3> S{};
    array<array<array<XMFLOAT3, 1>, 2>, 3> T{};
    // inside edge points [patch][bezier_index}
    array<XMFLOAT3, 3> a{};
    array<array<XMFLOAT3, 4>, 3> P{};
    XMFLOAT3 P0{};
    // in
    array<array<XMFLOAT3, 4>, 3> U{};

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
                storeFloat3Lerp(R[i][k][j], B[i][k][j]->position(), B[i][k][j + 1]->position(), 0.5f);
            }
        }

        // fill S
        for (int k = 0; k < 2; ++k) {
            for (int j = 0; j < 2; ++j) {
                storeFloat3Lerp(S[i][k][j], R[i][k][j], R[i][k][j + 1], 0.5f);
            }
        }

        // fill T
        for (int k = 0; k < 2; ++k) {
            storeFloat3Lerp(T[i][k][0], S[i][k][0], S[i][k][1], 0.5f);
        }

        // fill P
        storeFloat3Lerp(P[i][2], T[i][1][0], T[i][0][0], 2.f);
        P[i][3] = T[i][0][0];

        // a
        storeFloat3Lerp(a[i], P[i][3], P[i][2], 2.f);
    }

    auto a0 = XMLoadFloat3(&a[0]);
    auto a1 = XMLoadFloat3(&a[1]);
    auto a2 = XMLoadFloat3(&a[2]);
    XMStoreFloat3(&P0, XMVectorScale(XMVectorAdd(XMVectorAdd(a0, a1), a2), 1.f / 3.f));

    for (int i = 0; i < 3; ++i) {
        storeFloat3Lerp(P[i][1], P0, a[i], 2.f / 3.f);

        storeFloat3Lerp(U[i][0], R[i][1][0], R[i][0][0], 2.f);
        storeFloat3Lerp(U[i][1], S[i][1][0], S[i][0][0], 2.f);
        storeFloat3Lerp(U[i][2], S[i][1][1], S[i][0][1], 2.f);
        storeFloat3Lerp(U[i][3], R[i][1][2], R[i][0][2], 2.f);

        // edges
        for (int k = 0; k < 2; ++k) {
            bezierMesh.vertices().push_back({B[i][k][0]->position(), {1, 1, 1}});
            bezierMesh.vertices().push_back({R[i][k][0], {1, 1, 1}});
            bezierMesh.vertices().push_back({S[i][k][0], {1, 1, 1}});
            bezierMesh.vertices().push_back({T[i][k][0], {1, 1, 1}});
            bezierMesh.vertices().push_back({S[i][k][1], {1, 1, 1}});
            bezierMesh.vertices().push_back({R[i][k][2], {1, 1, 1}});
            bezierMesh.vertices().push_back({B[i][k][3]->position(), {1, 1, 1}});

            // inside points
            bezierMesh.vertices().push_back({U[i][0], {1, 1, 1}});
            bezierMesh.vertices().push_back({U[i][1], {1, 1, 1}});
            bezierMesh.vertices().push_back({U[i][2], {1, 1, 1}});
            bezierMesh.vertices().push_back({U[i][3], {1, 1, 1}});

            // border
            bezierMesh.addLine(i * 25 + k * 11, i * 25 + k * 11 + 1);
            bezierMesh.addLine(i * 25 + k * 11 + 1, i * 25 + k * 11 + 2);
            bezierMesh.addLine(i * 25 + k * 11 + 2, i * 25 + k * 11 + 3);
            bezierMesh.addLine(i * 25 + k * 11 + 3, i * 25 + k * 11 + 4);
            bezierMesh.addLine(i * 25 + k * 11 + 4, i * 25 + k * 11 + 5);
            bezierMesh.addLine(i * 25 + k * 11 + 5, i * 25 + k * 11 + 6);
            bezierMesh.addLine(i * 25 + k * 11 + 1, i * 25 + k * 11 + 7);
            bezierMesh.addLine(i * 25 + k * 11 + 2, i * 25 + k * 11 + 8);
            bezierMesh.addLine(i * 25 + k * 11 + 4, i * 25 + k * 11 + 9);
            bezierMesh.addLine(i * 25 + k * 11 + 5, i * 25 + k * 11 + 10);
        }

        auto index = bezierMesh.vertices().size();
        bezierMesh.vertices().push_back({P[i][3], {1, 1, 1}});
        bezierMesh.vertices().push_back({P[i][2], {1, 1, 1}});
        bezierMesh.vertices().push_back({P[i][1], {1, 1, 1}});
        bezierMesh.addLine(index, index + 1);
        bezierMesh.addLine(index + 1, index + 2);
    }

    auto index = bezierMesh.vertices().size();
    bezierMesh.vertices().push_back({P0, {1, 1, 1}});
    bezierMesh.addLine(index, 24);
    bezierMesh.addLine(index, 49);
    bezierMesh.addLine(index, 74);

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