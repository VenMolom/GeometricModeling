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
    calculateIndices();
    calculateMeshIndices(segments, bezierMesh);
}

Type GregoryPatch::type() const {
    return GREGORY;
}

void GregoryPatch::calculateMeshIndices(std::array<int, 2> segments, Linelist &linelist) {
    static const int wholeSize = 18;

    for(int i = 0; i < 3; ++i) {
        // border
        int index = i * wholeSize;
        bezierMesh.addLine(index,     index + 1);
        bezierMesh.addLine(index + 1, index + 2);
        bezierMesh.addLine(index + 2, index + 3);
        bezierMesh.addLine(index + 3, index + 4);
        bezierMesh.addLine(index + 4, index + 5);
        bezierMesh.addLine(index + 5, index + 6);
        // inside edge
        bezierMesh.addLine(index + 7, index + 8);
        bezierMesh.addLine(index + 8, index + 9);
        // inside points
        bezierMesh.addLine(index + 1, index + 10);
        bezierMesh.addLine(index + 2, index + 11);
        bezierMesh.addLine(index + 4, index + 12);
        bezierMesh.addLine(index + 5, index + 13);

        bezierMesh.addLine(index + 9, index + 14);
        bezierMesh.addLine(index + 8, index + 15);
        bezierMesh.addLine(index + 9, index + 16);
        bezierMesh.addLine(index + 8, index + 17);
    }

    int index = bezierMesh.vertices().size() - 1;
    bezierMesh.addLine(index, wholeSize - 9);
    bezierMesh.addLine(index, 2 * wholeSize - 9);
    bezierMesh.addLine(index, 3 * wholeSize - 9);
    bezierMesh.update();
}

void GregoryPatch::calculateIndices() {
    static const int wholeSize = 18;

    int lastPoint = vertices.size() - 1;

    for (int i = 0; i < 3; ++i) {
        int index = i * wholeSize;
        auto iindex = ((i + 2) % 3) * wholeSize;

        indices.push_back(index + 3);
        indices.push_back(index + 8);
        indices.push_back(index + 9);
        indices.push_back(lastPoint);

        indices.push_back(index + 4);
        indices.push_back(index + 12);
        indices.push_back(index + 17);
        indices.push_back(index + 16);
        indices.push_back(iindex + 14);
        indices.push_back(iindex + 9);

        indices.push_back(index + 5);
        indices.push_back(index + 13);
        indices.push_back(iindex + 10);
        indices.push_back(iindex + 11);
        indices.push_back(iindex + 15);
        indices.push_back(iindex + 8);

        indices.push_back(iindex);
        indices.push_back(iindex + 1);
        indices.push_back(iindex + 2);
        indices.push_back(iindex + 3);
    }
    updateBuffers();
}

void GregoryPatch::drawMesh(Renderer &renderer, DrawType drawType) {
    renderer.draw(*this, drawType != DEFAULT ? SELECTED_COLOR : DEFAULT_COLOR);
}

void GregoryPatch::createPoints() {
    // edge points [patch][depth][bezier_index]
    array<array<array<shared_ptr<VirtualPoint>, 4>, 2>, 3> B;
    array<array<array<XMFLOAT3, 3>, 2>, 3> R{};
    array<array<array<XMFLOAT3, 2>, 2>, 3> S{};
    array<array<array<XMFLOAT3, 1>, 2>, 3> T{};
    // inside edge points [patch][bezier_index}
    array<XMFLOAT3, 3> Q{};
    array<array<XMFLOAT3, 4>, 3> P{};
    XMFLOAT3 P0{};
    // in
    array<array<XMFLOAT3, 4>, 3> U{};
    array<array<XMFLOAT3, 4>, 3> V{};

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

        // fill Q
        storeFloat3Lerp(Q[i], P[i][3], P[i][2], 2.f);
    }

    // calculate P0
    auto Q0 = XMLoadFloat3(&Q[0]);
    auto Q1 = XMLoadFloat3(&Q[1]);
    auto Q2 = XMLoadFloat3(&Q[2]);
    XMStoreFloat3(&P0, XMVectorScale(XMVectorAdd(XMVectorAdd(Q0, Q1), Q2), 1.f / 3.f));

    for (int i = 0; i < 3; ++i) {
        //calculate P^i{1}
        storeFloat3Lerp(P[i][1], P0, Q[i], 2.f / 3.f);

        // inside points from border
        storeFloat3Lerp(U[i][0], R[i][1][0], R[i][0][0], 2.f);
        storeFloat3Lerp(U[i][1], S[i][1][0], S[i][0][0], 2.f);
        storeFloat3Lerp(U[i][2], S[i][1][1], S[i][0][1], 2.f);
        storeFloat3Lerp(U[i][3], R[i][1][2], R[i][0][2], 2.f);
    }

    for (int i = 0; i < 3; ++i) {
        // inside points from P (toward B0, prev patch)
        auto a0 = XMVectorSubtract(XMLoadFloat3(&P0), XMLoadFloat3(&P[(i + 2) % 3][1]));
        auto b0 = XMVectorSubtract(XMLoadFloat3(&P[(i + 1) % 3][1]), XMLoadFloat3(&P0));
        auto g0 = XMVectorScale(XMVectorAdd(a0, b0), 0.5f);
        auto g2 = XMVectorSubtract(XMLoadFloat3(&S[i][0][0]), XMLoadFloat3(&T[i][0][0]));

        P[i][0] = P0;
        auto p = calculateInsidePoints(g0, g2, b0, g2, P[i]);
        V[i][0] = p.first;
        V[i][1] = p.second;

        // inside points from P (toward B3, next patch)
        a0 = XMVectorNegate(a0);
        g0 = XMVectorNegate(g0);
        g2 = XMVectorNegate(g2);

        p = calculateInsidePoints(g0, g2, a0, g2, P[i]);
        V[i][2] = p.first;
        V[i][3] = p.second;
    }

    for (int i = 0; i < 3; ++i) {
        // edges
        addPoint(B[i][0][0]->position());
        addPoint(R[i][0][0]);
        addPoint(S[i][0][0]);
        addPoint(T[i][0][0]);
        addPoint(S[i][0][1]);
        addPoint(R[i][0][2]);
        addPoint(B[i][0][3]->position()); //6

        // inside edge points
        addPoint(P[i][3]);
        addPoint(P[i][2]);
        addPoint(P[i][1]); //9

        // inside points
        addPoint(U[i][0]);
        addPoint(U[i][1]);
        addPoint(U[i][2]);
        addPoint(U[i][3]); //13

        addPoint(V[i][0]);
        addPoint(V[i][1]);
        addPoint(V[i][2]);
        addPoint(V[i][3]); //17
    }

    addPoint(P0);
    bezierMesh.update();
    updateBuffers();
}

void GregoryPatch::setHandlers() {
    // TODO: store position changed handles for patches' border points
}

void GregoryPatch::clear() {
    vertices.clear();
    bezierMesh.vertices().clear();
}

pair<XMFLOAT3, XMFLOAT3> GregoryPatch::calculateInsidePoints(XMVECTOR g0, XMVECTOR g2,
                                                             XMVECTOR b0, XMVECTOR b2, array<XMFLOAT3, 4> P) {
    auto g1 = XMVectorScale(XMVectorAdd(g0, g2), 0.5f);

    auto c0 = XMVectorSubtract(XMLoadFloat3(&P[1]), XMLoadFloat3(&P[0]));
    auto c1 = XMVectorSubtract(XMLoadFloat3(&P[2]), XMLoadFloat3(&P[1]));
    auto c2 = XMVectorSubtract(XMLoadFloat3(&P[3]), XMLoadFloat3(&P[2]));

    auto [k0, h0] = solve(g0, c0, b0);
    //auto [k1, h1] = solve(g2, c2, b2);

    auto k1 = 1.f;
    auto h1 = 0.f;

    auto gg1 = deCasteljau({g0, g1, g2}, 1.f / 3.f);
    auto gg2 = deCasteljau({g0, g1, g2}, 2.f / 3.f);
    auto cc1 = deCasteljau({c0, c1, c2}, 1.f / 3.f);
    auto cc2 = deCasteljau({c0, c1, c2}, 2.f / 3.f);

    auto d1 = XMVectorAdd(XMVectorScale(gg1, k0 * (2.f / 3.f) + k1 * (1.f / 3.f)),
                          XMVectorScale(cc1, h0 * (2.f / 3.f) + h1 * (1.f / 3.f)));
    auto d2 = XMVectorAdd(XMVectorScale(gg2, k0 * (1.f / 3.f) + k1 * (2.f / 3.f)),
                          XMVectorScale(cc2, h0 * (1.f / 3.f) + h1 * (2.f / 3.f)));

    XMFLOAT3 D1, D2;
    XMStoreFloat3(&D1, XMVectorAdd(XMLoadFloat3(&P[1]), d1));
    XMStoreFloat3(&D2, XMVectorAdd(XMLoadFloat3(&P[2]), d2));

    return {D1, D2};
}

XMVECTOR GregoryPatch::deCasteljau(array<XMVECTOR, 3> points, float t) {
    float t1 = 1.f - t;

    for (int j = 2; j > 0; --j) {
        for (int i = 0; i < j; ++i) {
            points[i] = XMVectorAdd(XMVectorScale(points[i], t1), XMVectorScale(points[i + 1], t));
        }
    }

    return points[0];
}

std::pair<float, float> GregoryPatch::solve(DirectX::XMVECTOR g, DirectX::XMVECTOR c, DirectX::XMVECTOR b) {
    auto h = (b.m128_f32[1] - g.m128_f32[1] / g.m128_f32[2] * b.m128_f32[2]) /
            (c.m128_f32[1] - g.m128_f32[1] / g.m128_f32[2] * c.m128_f32[2]);
    auto k = (b.m128_f32[2] - h * c.m128_f32[2]) / g.m128_f32[2];
    return {k, h};
}

void GregoryPatch::addPoint(XMFLOAT3 position) {
    vertices.push_back({position, {1, 1, 1}});
    bezierMesh.vertices().push_back({position, {1, 1, 1}});
}