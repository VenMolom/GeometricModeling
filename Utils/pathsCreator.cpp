//
// Created by Molom on 10/11/2022.
//

#include "pathsCreator.h"

#include <cassert>
#include <utility>
#include "Utils/fileParser.h"
#include "Objects/Patch/bicubicC0Creator.h"
#include "Utils/pathsCreatorHelper.h"
#include "Objects/Parametric/toolDistantSurface.h"

using namespace std;
using namespace DirectX;
using namespace PathsCreatorHelper;

void PathsCreator::create(const filesystem::path &directory,
                          vector<shared_ptr<Object>> objects,
                          ObjectFactory &factory,
                          Renderer &renderer) {
    // objects order: main, handle, dziubek

    assert(filesystem::is_directory(directory));

    auto creator = PathsCreator(directory, std::move(objects));
    creator.createRoughPaths(16, renderer);
    creator.createFlatteningPaths(12, renderer, factory);
    creator.createDetailPaths(8, renderer, factory);
}

PathsCreator::PathsCreator(filesystem::path basePath, std::vector<std::shared_ptr<Object>> objects)
        : basePath(std::move(basePath)),
          objects(std::move(objects)) {
    QProperty<weak_ptr<Object>> prop{};
    auto patchCreator = BicubicC0Creator({0, 0, 0}, &prop);
    patchCreator.setSegments({5, 5});
    patchCreator.setSize({2.5, 2.5});
    patch = static_pointer_cast<Patch>(patchCreator.create(0));
}

void PathsCreator::createRoughPaths(int toolSize, Renderer &renderer) {
    static const float FIRST_LAYER_Z = 33.5f;
    static const float SECOND_LAYER_Z = 17.f;
    static const float Y_DIFF = 8.f;
    static const int Y_MOVES = 23;

    auto &device = DxDevice::Instance();
    auto tex = createDepthTextures(device);
    auto viewMatrix = XMMatrixLookAtRH(XMVectorSet(0.f, BLOCK_SIZE_Z - BLOCK_BOTTOM, 0.f, 1.f),
                                       XMVectorSet(0.f, 0.f, 0.f, 1.f),
                                       XMVectorSet(0.f, 0.f, -1.f, 0.f));
    auto projectionMatrix = XMMatrixOrthographicRH(BLOCK_SIZE_XY, BLOCK_SIZE_XY, 0, BLOCK_SIZE_Z);

    Renderer::Textures renderTex;
    renderTex.depth = tex.depth.get();
    renderTex.depthTexture = tex.depthTexture.get();
    renderTex.unorderedTexture = tex.allowedHeight.get();
    renderTex.viewportSize = make_pair(TEX_SIZE, TEX_SIZE);
    renderTex.toolSize = toolSize;
    renderTex.materialSize = make_pair(BLOCK_SIZE_XY * 10.f, BLOCK_SIZE_XY * 10.f);
    renderTex.materialDepth = BLOCK_SIZE_Z * 10.f;

    renderer.drawToTexture(renderTex, objects, projectionMatrix, viewMatrix);

    copyResource(device, tex.allowedHeight, tex.allowedHeightStaging);

    D3D11_MAPPED_SUBRESOURCE res;

    auto hr = device.context()->Map(tex.allowedHeightStaging.get(), 0, D3D11_MAP_READ, 0, &res);
    if (FAILED(hr))
        return;

    auto *data = reinterpret_cast<float *>(res.pData);

    vector<XMFLOAT3> positions;

    positions.emplace_back(0.f, 0.f, START_Z);

    //first layer
    positions.emplace_back(START_X, START_Y, START_Z);

    float y = START_Y;
    for (int i = 0; i < Y_MOVES; ++i) {
        int xDir = (i % 2) * 2 - 1;

        positions.emplace_back(-xDir * START_X, y, FIRST_LAYER_Z);

        int texY = (TEX_SIZE - 1) * ((BLOCK_END_LOCAL - y) / (BLOCK_END_LOCAL * 2.f));
        if (texY >= 0 && texY < TEX_SIZE) {
            addPositionsOnLine(positions, data,
                               xDir,
                               FIRST_LAYER_Z, BLOCK_SIZE_XY,
                               y, texY);
        }

        positions.emplace_back(xDir * START_X, y, FIRST_LAYER_Z);
        y -= Y_DIFF;
    }
    y = -START_Y;

    //second layer
    for (int i = 0; i < Y_MOVES; ++i) {
        int xDir = (i % 2) * 2 - 1;
        positions.emplace_back(xDir * START_X, y, SECOND_LAYER_Z);

        int texY = (TEX_SIZE - 1) * ((BLOCK_END_LOCAL - y) / (BLOCK_END_LOCAL * 2.f));
        if (texY >= 0 && texY < TEX_SIZE) {
            addPositionsOnLine(positions, data,
                               -xDir,
                               SECOND_LAYER_Z, BLOCK_SIZE_XY,
                               y, texY);
        }

        positions.emplace_back(-xDir * START_X, y, SECOND_LAYER_Z);
        y += Y_DIFF;
    }

    positions.emplace_back(((Y_MOVES % 2) * 2 - 1) * START_X, START_Y, START_Z);
    positions.emplace_back(0.f, 0.f, START_Z);

    device.context()->Unmap(tex.allowedHeightStaging.get(), 0);
    allowedHeightStaging = std::move(tex.allowedHeightStaging);

    FileParser::saveCNCPath(basePath / std::format("1.k{}", toolSize), positions);
}

void PathsCreator::createFlatteningPaths(int toolSize, Renderer &renderer, ObjectFactory &factory) {
// patch C0 5x5 scale (2.5, 1, 2.5)\

// U,V - patch; S,T - surface

// handle - internal: starting - {u=0.139966473, v=2.86276722, s=4.21201801, t=2.62888551}
// handle - external: starting - {u=4.56668663, v=2.79134393, s=2.47588515, t=2.7601614}

// main - left: starting - {u=1.83718324, v=0.757870793, s=3.52815628, t=1.48308873}
// main - right: starting - {u=3.42344403, v=0.896888852, s=0.475813448, t=1.58128047}

// dziubek: starting - {u=0.587088287, v=1.62481773, s=0.297016472, t=5.40027523}

    auto main = static_pointer_cast<ParametricObject<2>>(objects[0]);
    auto handle = static_pointer_cast<ParametricObject<2>>(objects[1]);
    auto dziubek = static_pointer_cast<ParametricObject<2>>(objects[2]);

    IntersectHandler intersect(false, factory);

    // handle external
    auto handleDistant = intersectAndCalculateToolDistant(intersect, renderer, patch, handle,
                                                          {4.56668663, 2.79134393, 2.47588515, 2.7601614},
                                                          HandleExternal, toolSize);

    // dziubek top
    auto dziubekDistant = intersectAndCalculateToolDistant(intersect, renderer, patch, dziubek,
                                                           {0.587088287, 1.62481773, 0.297016472, 5.40027523},
                                                           Dziubek, toolSize);

    // main right
    intersect.setSurfaces({patch, main});
    auto [mainRightParams, mainRightPoints] = intersect.calculateIntersection(renderer,
                                                                              {3.42344403, 0.896888852, 0.475813448,
                                                                               1.58128047});
    assert(!mainRightParams.empty());
    mainRightParams.back() = {0, 0};
    XMStoreFloat3(&mainRightPoints.back(), main->value({0, 0}));
    auto mainRightDistant = calculateToolDistantPath(*main, mainRightParams, mainRightPoints, toolSize, MainRight);

    // main left
    intersect.setSurfaces({patch, main});
    auto [mainLeftParams, mainLeftPoints] = intersect.calculateIntersection(renderer,
                                                                            {1.83718324, 0.757870793, 3.52815628,
                                                                             1.48308873});
    assert(!mainLeftParams.empty());
    mainLeftParams[0] = {0, 0};
    mainLeftPoints[0] = mainRightPoints.back();
    auto mainLeftDistant = calculateToolDistantPath(*main, mainLeftParams, mainLeftPoints, toolSize, MainLeft);

    // connect main right and main left;
    vector<XMFLOAT3> envelope(std::move(mainRightDistant));
    {
        auto paramsStart = mainRightParams[mainRightParams.size() - 3];
        array<float, 2> paramStart = {paramsStart.first, paramsStart.second};
        auto tangentStart = main->tangent(paramStart);
        auto bitangentStart = main->bitangent(paramStart);
        auto normalStart = XMVector3Cross(bitangentStart, tangentStart);
        normalStart.m128_f32[1] = 0;
        normalStart = XMVector3Normalize(normalStart);
        auto point = XMLoadFloat3(&mainRightPoints.back());

        auto paramsEnd = mainLeftParams[2];
        array<float, 2> paramEnd = {paramsEnd.first, paramsEnd.second};
        auto tangentEnd = main->tangent(paramEnd);
        auto bitangentEnd = main->bitangent(paramEnd);
        auto normalEnd = XMVector3Cross(bitangentEnd, tangentEnd);
        normalEnd.m128_f32[1] = 0;
        normalEnd = XMVector3Normalize(normalEnd);

        insertSlerpNormalPosition(envelope, XMVectorScale(point, 10.f), normalStart, normalEnd,
                                  toolSize / 2.f);
        envelope.insert(envelope.end(), mainLeftDistant.begin(), mainLeftDistant.end());
    }

    // add handle into envelope
    auto [envelopeIter, handleIter, inter1] = findIntersection(envelope.begin() + 50, handleDistant.begin() + 10);
    auto [envelopeIter2, handleIter2, inter2] = findIntersection(envelopeIter + 100, handleDistant.end() - 50);

    envelope.erase(envelopeIter + 1, envelopeIter2 + 1);
    envelope.insert(envelopeIter + 1, inter2);
    envelope.insert(envelopeIter + 1, inter1);
    envelope.insert(envelopeIter + 2, handleIter + 1, handleIter2 + 1);

    // add dziubek into envelope
    auto [envelopeIter3, dziubekIter, inter3] = findIntersection(envelope.begin() + 350, dziubekDistant.begin() + 50);
    auto [dziubekIter2, envelopeIter4, inter4] = findIntersection(dziubekIter + 100, envelopeIter3 + 50);

    envelope.erase(envelopeIter3 + 1, envelopeIter4 + 1);
    envelope.insert(envelopeIter3 + 1, inter4);
    envelope.insert(envelopeIter3 + 1, inter3);
    envelope.insert(envelopeIter3 + 2, dziubekIter + 1, dziubekIter2 + 1);

    D3D11_MAPPED_SUBRESOURCE res;
    auto &device = DxDevice::Instance();

    auto hr = device.context()->Map(allowedHeightStaging.get(), 0, D3D11_MAP_READ, 0, &res);
    if (FAILED(hr))
        return;

    auto *data = reinterpret_cast<float *>(res.pData);
    auto zigzag = createZigZagLines(data, toolSize);
    device.context()->Unmap(allowedHeightStaging.get(), 0);

    vector<XMFLOAT3> positions;
    positions.emplace_back(0.f, 0.f, START_Z);
    positions.emplace_back(START_X, -START_Y, START_Z);

    positions.insert(positions.end(), zigzag.begin(), zigzag.end());

    positions.emplace_back(START_X, envelope.front().y, BLOCK_BOTTOM_LOCAL);

    positions.insert(positions.end(), envelope.begin(), envelope.end());

    positions.emplace_back(START_X, envelope.back().y, BLOCK_BOTTOM_LOCAL);
    positions.emplace_back(START_X, -START_Y, START_Z);
    positions.emplace_back(0.f, 0.f, START_Z);
    FileParser::saveCNCPath(basePath / std::format("2.f{}", toolSize), positions);
}

void PathsCreator::createDetailPaths(int toolSize, Renderer &renderer, ObjectFactory &factory) {
    float distance = toolSize / 20.f;
    auto mainDistant = make_shared<ToolDistantSurface>(static_pointer_cast<Patch>(objects[0]), distance);
    auto handleDistant = make_shared<ToolDistantSurface>(static_pointer_cast<Patch>(objects[1]), distance);
    auto dziubekDistant = make_shared<ToolDistantSurface>(static_pointer_cast<Patch>(objects[2]), distance);

    auto patchDistant = make_shared<ToolDistantSurface>(patch, distance);

    IntersectHandler intersect(false, factory);

    // handle paths
    intersect.setSurfaces({patchDistant, handleDistant});
    auto [outsideParams, outsidePoints] = intersect.calculateIntersection(renderer, {4.57867765, 3.19881678, 3.17830682,
                                                                                     2.34179115});
    auto [insideParams, insidePoints] = intersect.calculateIntersection(renderer, {4.02702141, 2.75102615, 0.896845459,
                                                                                   2.65458512});

    intersect.setSurfaces({mainDistant, handleDistant});
    auto [topRingParams, topRingPoints] = intersect.calculateIntersection(renderer, {5.95757055, 4.5840373, 4.29148722,
                                                                                     5.70556259});
    auto [bottomRingParams, bottomRingPoints] = intersect.calculateIntersection(renderer,
                                                                                {0.792187035, 6.699512, 0.868857979,
                                                                                 0.548215151});

    // trim top and bottom ring
    auto topRingIter1 = findIntersectionHeight(topRingPoints.begin(), distance);
    auto topRingIter2 = findIntersectionHeight(topRingIter1 + 10, distance);
    {
        vector<pair<float, float>> topRingTemp;
        auto topRingParamIter1 = topRingParams.begin() - (topRingPoints.begin() - topRingIter1);
        auto topRingParamIter2 = topRingParams.begin() - (topRingPoints.begin() - topRingIter2);
        topRingTemp.insert(topRingTemp.end(), topRingParamIter2, topRingParams.end());
        topRingTemp.insert(topRingTemp.end(), topRingParams.begin(), topRingParamIter1 + 1);
        topRingParams = topRingTemp;
    }
    {
        vector<XMFLOAT3> topRingTemp;
        topRingTemp.insert(topRingTemp.end(), topRingIter2, topRingPoints.end());
        topRingTemp.insert(topRingTemp.end(), topRingPoints.begin(), topRingIter1 + 1);
        topRingPoints = topRingTemp;
    }

    auto bottomRingIter1 = findIntersectionHeight(bottomRingPoints.begin(), distance);
    auto bottomRingIter2 = findIntersectionHeight(bottomRingIter1 + 10, distance);
    auto bottomRingParamIter1 = bottomRingParams.begin() - (bottomRingPoints.begin() - bottomRingIter1);
    auto bottomRingParamIter2 = bottomRingParams.begin() - (bottomRingPoints.begin() - bottomRingIter2);
    bottomRingParams.erase(bottomRingParamIter2, bottomRingParams.end());
    bottomRingParams.erase(bottomRingParams.begin(), bottomRingParamIter1 + 1);
    bottomRingPoints.erase(bottomRingIter2, bottomRingPoints.end());
    bottomRingPoints.erase(bottomRingPoints.begin(), bottomRingIter1 + 1);

    auto handlePath = createHandlePath(topRingParams, bottomRingParams, insideParams, outsideParams, handleDistant);

    vector<XMFLOAT3> positions;
    for (auto &point: handlePath) {
        positions.emplace_back(point.x * 10.f, -point.z * 10.f, point.y * 10.f + BLOCK_BOTTOM_LOCAL - toolSize / 2.f);
    }
//    for (auto& point: topRingPoints) {
//        positions.emplace_back(point.x * 10.f, -point.z * 10.f, point.y * 10.f + BLOCK_BOTTOM_LOCAL);
//    }
//    for (auto& point: bottomRingPoints) {
//        positions.emplace_back(point.x * 10.f, -point.z * 10.f, point.y * 10.f + BLOCK_BOTTOM_LOCAL);
//    }
    FileParser::saveCNCPath(basePath / std::format("3.k{:0>2}", toolSize), positions);
}