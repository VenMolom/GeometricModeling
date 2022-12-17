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

    auto main = static_pointer_cast<ParametricObject<2>>(objects[1]);
    auto handle = static_pointer_cast<ParametricObject<2>>(objects[0]);
    auto dziubek = static_pointer_cast<ParametricObject<2>>(objects[2]);

    IntersectHandler intersect(false, factory);

    // handle external
    auto handleDistant = intersectAndCalculateToolDistant(intersect, renderer, patch, handle,
                                                          {4.56668663, 2.79134393, 2.47588515, 2.7601614},
                                                          HandleExternal, toolSize);

    // dziubek
    auto dziubekDistant = intersectAndCalculateToolDistant(intersect, renderer, patch, dziubek,
                                                           {1.11931062, 1.73776686, 2.79065156, 4.85279322},
                                                           Dziubek, toolSize);

    // main right
    intersect.setSurfaces({patch, main});
    auto [_, mainRightParams, mainRightPoints] = intersect.calculateIntersection(renderer,
                                                                              {3.42344403, 0.896888852, 0.475813448,
                                                                               1.58128047});
    assert(!mainRightParams.empty());
    mainRightParams.back() = {0, 0};
    XMStoreFloat3(&mainRightPoints.back(), main->value({0, 0}));
    auto mainRightDistant = calculateToolDistantPath(*main, mainRightParams, mainRightPoints, toolSize, MainRight);

    // main left
    intersect.setSurfaces({patch, main});
    auto [__, mainLeftParams, mainLeftPoints] = intersect.calculateIntersection(renderer,
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
    auto mainDistant = make_shared<ToolDistantSurface>(static_pointer_cast<Patch>(objects[1]), distance);
    auto handleDistant = make_shared<ToolDistantSurface>(static_pointer_cast<Patch>(objects[0]), distance);
    auto dziubekDistant = make_shared<ToolDistantSurface>(static_pointer_cast<Patch>(objects[2]), distance);

    auto patchDistant = make_shared<ToolDistantSurface>(patch, distance);

    IntersectHandler intersect(false, factory);

    // handle paths
#pragma region Handle
    intersect.setSurfaces({patchDistant, handleDistant});
    auto [_, outsideParams, outsidePoints] = intersect.calculateIntersection(renderer,
                                                                             {4.57867765, 3.19881678, 3.17830682,
                                                                              2.34179115});
    auto [__, insideParams, insidePoints] = intersect.calculateIntersection(renderer,
                                                                           {4.02702141, 2.75102615, 0.896845459,
                                                                            2.65458512});

    intersect.setSurfaces({mainDistant, handleDistant});
    auto [topRingMainParams, topRingParams, topRingPoints] = intersect.calculateIntersection(renderer,
                                                                                             {5.95757055, 4.5840373,
                                                                                              4.29148722,
                                                                                              5.70556259});
    auto [bottomRingMainParams, bottomRingParams, bottomRingPoints] = intersect.calculateIntersection(renderer,
                                                                                                      {0.792187035,
                                                                                                       6.699512,
                                                                                                       0.868857979,
                                                                                                       0.548215151});

    // trim top and bottom ring by height and reorder
    {
        auto topRingIter1 = findIntersectionHeight(topRingPoints.begin(), distance);
        auto topRingIter2 = findIntersectionHeight(topRingIter1 + 10, distance);
        {
            vector<pair<float, float>> topRingTemp;
            auto topRingParamIter1 = topRingParams.begin() - (topRingPoints.begin() - topRingIter1);
            auto topRingParamIter2 = topRingParams.begin() - (topRingPoints.begin() - topRingIter2);
            topRingTemp.insert(topRingTemp.end(), topRingParamIter2, topRingParams.end());
            topRingTemp.insert(topRingTemp.end(), topRingParams.begin(), topRingParamIter1 + 2);
            topRingParams = topRingTemp;

            vector<pair<float, float>> topRingMainTemp;
            auto topRingMainParamIter1 = topRingMainParams.begin() - (topRingPoints.begin() - topRingIter1);
            auto topRingMainParamIter2 = topRingMainParams.begin() - (topRingPoints.begin() - topRingIter2);
            topRingTemp.insert(topRingTemp.end(), topRingMainParamIter2, topRingMainParams.end());
            topRingTemp.insert(topRingTemp.end(), topRingMainParams.begin(), topRingMainParamIter1 + 2);
            topRingMainParams = topRingTemp;
        }
        {
            vector<XMFLOAT3> topRingTemp;
            topRingTemp.insert(topRingTemp.end(), topRingIter2, topRingPoints.end());
            topRingTemp.insert(topRingTemp.end(), topRingPoints.begin(), topRingIter1 + 2);
            topRingPoints = topRingTemp;
        }
    }
    {
        auto bottomRingIter1 = findIntersectionHeight(bottomRingPoints.begin(), distance);
        auto bottomRingIter2 = findIntersectionHeight(bottomRingIter1 + 10, distance);
        auto bottomRingParamIter1 = bottomRingParams.begin() - (bottomRingPoints.begin() - bottomRingIter1);
        auto bottomRingParamIter2 = bottomRingParams.begin() - (bottomRingPoints.begin() - bottomRingIter2);
        auto bottomRingMainParamIter1 = bottomRingMainParams.begin() - (bottomRingPoints.begin() - bottomRingIter1);
        auto bottomRingMainParamIter2 = bottomRingMainParams.begin() - (bottomRingPoints.begin() - bottomRingIter2);
        bottomRingMainParams.erase(bottomRingMainParamIter2 + 2, bottomRingMainParams.end());
        bottomRingMainParams.erase(bottomRingMainParams.begin(), bottomRingMainParamIter1);
        bottomRingParams.erase(bottomRingParamIter2 + 2, bottomRingParams.end());
        bottomRingParams.erase(bottomRingParams.begin(), bottomRingParamIter1);
        bottomRingPoints.erase(bottomRingIter2 + 2, bottomRingPoints.end());
        bottomRingPoints.erase(bottomRingPoints.begin(), bottomRingIter1);
    }

    // trim bottom ring
    {
        auto [bottomRingIter1, outsideIter, inter1] = findIntersection(bottomRingPoints.begin() + 40,
                                                                       outsidePoints.begin());
        auto [insideIter, bottomRingIter2, inter2] = findIntersection(insidePoints.begin() + 70,
                                                                      bottomRingPoints.begin());

        bottomRingPoints.erase(bottomRingIter1 + 1, bottomRingPoints.end());
        bottomRingPoints.push_back(inter1);
        bottomRingPoints.erase(bottomRingPoints.begin(), bottomRingIter2 + 1);
        bottomRingPoints.insert(bottomRingPoints.begin(), inter2);

        outsidePoints.erase(outsidePoints.begin(), outsideIter + 1);
        outsidePoints.insert(outsidePoints.begin(), inter1);

        insidePoints.erase(insideIter + 1, insidePoints.end());
        insidePoints.push_back(inter2);
    }

    // trim top ring
    {
        auto [outsideIter, topRingIter1, inter1] = findIntersection(outsidePoints.begin() + 140,
                                                                    topRingPoints.begin());
        auto [topRingIter2, insideIter, inter2] = findIntersection(topRingPoints.begin() + 40,
                                                                   insidePoints.begin());

        topRingPoints.erase(topRingIter2 + 1, topRingPoints.end());
        topRingPoints.push_back(inter2);
        topRingPoints.erase(topRingPoints.begin(), topRingIter1 + 1);
        topRingPoints.insert(topRingPoints.begin(), inter1);

        insidePoints.erase(insidePoints.begin(), insideIter + 1);
        insidePoints.insert(insidePoints.begin(), inter2);

        outsidePoints.erase(outsideIter + 1, outsidePoints.end());
        outsidePoints.push_back(inter1);
    }

    auto handlePath = createHandlePath(topRingParams, bottomRingParams, insideParams, outsideParams, handleDistant);
    auto handleContour = createHandleContour(topRingPoints, bottomRingPoints, insidePoints, outsidePoints);
#pragma endregion

    // dziubek paths
#pragma region Dziubek
    intersect.setSurfaces({patchDistant, dziubekDistant});
    auto [___, dziubekOutlineParams, dziubekOutlinePoints] = intersect.calculateIntersection(renderer,
                                                                                           {1.29923332, 1.80520785,
                                                                                            3.2055254,
                                                                                            4.43139553});

    intersect.setSurfaces({mainDistant, dziubekDistant});
    auto [dziubekRingParams, mainRingParams, mainRingPoints] = intersect.calculateIntersection(renderer,
                                                                                              {3.5824976, 3.81490111,
                                                                                               3.00511742,
                                                                                               5.7883172});

    // trim outline
    {
        auto [outlineIter1, mainRingIter1, inter1] = findIntersection(dziubekOutlinePoints.begin(),
                                                                      mainRingPoints.begin());
        auto [mainRingIter2, outlineIter2, inter2] = findIntersection(mainRingIter1 + 20,
                                                                      dziubekOutlinePoints.begin());

        auto outlineParamIter1 = dziubekOutlineParams.begin() - (dziubekOutlinePoints.begin() - outlineIter1);
        auto outlineParamIter2 = dziubekOutlineParams.begin() - (dziubekOutlinePoints.begin() - outlineIter2);
        dziubekOutlineParams.erase(outlineParamIter1 + 2, dziubekOutlineParams.end());
        dziubekOutlineParams.erase(dziubekOutlineParams.begin(), outlineParamIter2);

        dziubekOutlinePoints.erase(outlineIter1 + 1, dziubekOutlinePoints.end());
        dziubekOutlinePoints.push_back(inter1);
        dziubekOutlinePoints.erase(dziubekOutlinePoints.begin(), outlineIter2 + 1);
        dziubekOutlinePoints.insert(dziubekOutlinePoints.begin(), inter2);

        auto mainRingParamIter1 = mainRingParams.begin() - (mainRingPoints.begin() - mainRingIter1);
        auto mainRingParamIter2 = mainRingParams.begin() - (mainRingPoints.begin() - mainRingIter2);
        mainRingParams.erase(mainRingParamIter2 + 2, mainRingParams.end());
        mainRingParams.erase(mainRingParams.begin(), mainRingParamIter1);

        auto dziubekRingParamIter1 = dziubekRingParams.begin() - (mainRingPoints.begin() - mainRingIter1);
        auto dziubekRingParamIter2 = dziubekRingParams.begin() - (mainRingPoints.begin() - mainRingIter2);
        dziubekRingParams.erase(dziubekRingParamIter2 + 2, dziubekRingParams.end());
        dziubekRingParams.erase(dziubekRingParams.begin(), dziubekRingParamIter1);

        mainRingPoints.erase(mainRingIter2 + 1, mainRingPoints.end());
        mainRingPoints.push_back(inter2);
        mainRingPoints.erase(mainRingPoints.begin(), mainRingIter1 + 1);
        mainRingPoints.insert(mainRingPoints.begin(), inter1);
    }

    auto dziubekPath = createDziubekPath(dziubekOutlineParams, mainRingParams, dziubekDistant);
    auto dziubekContour = createDziubekContour(dziubekOutlinePoints, mainRingPoints);
#pragma endregion

    // main part paths
#pragma region Main
    intersect.setSurfaces({patchDistant, mainDistant});
    auto [____, mainBottomParams, mainBottomPoints] = intersect.calculateIntersection(renderer,
                                                                                {4.07142401, 1.49915862, 0.36954397,
                                                                                 3.97987604});
    mainBottomParams.pop_back();
    mainBottomPoints.pop_back();
    // prolong first and last segment (for intersection later on)
    {
        XMFLOAT3 val;
        auto v1 = XMLoadFloat3(&mainBottomPoints[0]);
        auto v2 = XMLoadFloat3(&mainBottomPoints[1]);
        XMStoreFloat3(&val, XMVectorLerp(v1, v2, -10));
        mainBottomPoints.insert(mainBottomPoints.begin(), val);

        v1 = XMLoadFloat3(&mainBottomPoints[mainBottomPoints.size() - 1]);
        v2 = XMLoadFloat3(&mainBottomPoints[mainBottomPoints.size() - 2]);
        XMStoreFloat3(&val, XMVectorLerp(v1, v2, -10));
        mainBottomPoints.push_back(val);

        auto p1 = mainBottomParams[0];
        auto p2 = mainBottomParams[1];
        mainBottomParams.insert(mainBottomParams.begin(),
                                {p1.first - 10 * (p2.first - p1.first),
                                 p1.second - 10 * (p2.second - p1.second)});

        p1 = mainBottomParams[mainBottomParams.size() - 1];
        p2 = mainBottomParams[mainBottomParams.size() - 2];
        mainBottomParams.push_back({p1.first - 10 * (p2.first - p1.first),
                                    p1.second - 10 * (p2.second - p1.second)});
    }

    vector<XMFLOAT3> midRingPoints;
    vector<pair<float, float>> midRingParams;
    XMFLOAT3 val;
    const static float V = 3.000001f;
    for (int i = 0; i < 200; i++) {
        float u = i / 200.f * 6.f;
        midRingParams.push_back({u, V});
        XMStoreFloat3(&val, mainDistant->value({u, V}));
        midRingPoints.push_back(val);
    }

    // trim outline
    {
        auto [mainBottomIter1, midRingIter1, inter1] = findIntersection(mainBottomPoints.begin() + 400,
                                                                        midRingPoints.begin());
        auto [midRingIter2, mainBottomIter2, inter2] = findIntersection(midRingPoints.begin() + 80,
                                                                        mainBottomPoints.begin());

        auto mainBottomParamIter1 = mainBottomParams.begin() - (mainBottomPoints.begin() - mainBottomIter1);
        auto mainBottomParamIter2 = mainBottomParams.begin() - (mainBottomPoints.begin() - mainBottomIter2);
        mainBottomParams.erase(mainBottomParamIter1 + 2, mainBottomParams.end());
        mainBottomParams.erase(mainBottomParams.begin(), mainBottomParamIter2);

        mainBottomPoints.erase(mainBottomIter1 + 1, mainBottomPoints.end());
        mainBottomPoints.push_back(inter1);
        mainBottomPoints.erase(mainBottomPoints.begin(), mainBottomIter2 + 1);
        mainBottomPoints.insert(mainBottomPoints.begin(), inter2);

        vector<pair<float, float>> midRingParamTemp;
        auto midRingParamIter1 = midRingParams.begin() - (midRingPoints.begin() - midRingIter1);
        auto midRingParamIter2 = midRingParams.begin() - (midRingPoints.begin() - midRingIter2);
        midRingParamTemp.insert(midRingParamTemp.end(), midRingParamIter2, midRingParams.end());
        midRingParamTemp.insert(midRingParamTemp.end(), midRingParams.begin(), midRingParamIter1 + 2);
        midRingParams = midRingParamTemp;

        vector<XMFLOAT3> midRingTemp;
        midRingTemp.push_back(inter2);
        midRingTemp.insert(midRingTemp.end(), midRingIter2 + 1, midRingPoints.end());
        midRingTemp.insert(midRingTemp.end(), midRingPoints.begin(), midRingIter1 + 1);
        midRingTemp.push_back(inter1);
        midRingPoints = midRingTemp;
    }

    auto mainPath = createMainPath(mainBottomParams, midRingParams, dziubekRingParams, topRingMainParams,
                                   bottomRingMainParams, mainDistant);
    auto mainContour = createMainContour(mainBottomPoints, midRingPoints, mainRingPoints, topRingPoints,
                                         bottomRingPoints);
#pragma endregion
    // TODO: main top paths and hole paths

    vector<XMFLOAT3> positions;
    positions.emplace_back(0.f, 0.f, START_Z);
    positions.emplace_back(handlePath.front().x * 10.f, -handlePath.front().z * 10.f, START_Z);

    transformAndAppend(positions, handlePath, toolSize);
    transformAndAppend(positions, handleContour, toolSize);

    auto height = positions.back().z + 30.f;
    positions.emplace_back(positions.back().x, positions.back().y, height);
    positions.emplace_back(dziubekPath.front().x * 10.f, -dziubekPath.front().z * 10.f, height);

    transformAndAppend(positions, dziubekPath, toolSize);
    transformAndAppend(positions, dziubekContour, toolSize);

    height = positions.back().z + 30.f;
    positions.emplace_back(positions.back().x, positions.back().y, height);
    positions.emplace_back(mainPath.front().x * 10.f, -mainPath.front().z * 10.f, height);

    transformAndAppend(positions, mainPath, toolSize);
    positions.emplace_back(mainContour.front().x * 10.f, -mainContour.front().z * 10.f, positions.back().z);
    transformAndAppend(positions, mainContour, toolSize);

    positions.emplace_back(positions.back().x, positions.back().y, START_Z);
    positions.emplace_back(0.f, 0.f, START_Z);
    FileParser::saveCNCPath(basePath / std::format("3.k{:0>2}", toolSize), positions);
}