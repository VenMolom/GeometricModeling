//
// Created by Molom on 10/11/2022.
//

#include "pathsCreator.h"

#include <cassert>
#include <utility>
#include "Utils/fileParser.h"
#include "Objects/Patch/bicubicC0Creator.h"
#include "Utils/pathsCreatorHelper.h"

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
    creator.createDetailPaths(8);
}

PathsCreator::PathsCreator(filesystem::path basePath, std::vector<std::shared_ptr<Object>> objects)
        : basePath(std::move(basePath)),
          objects(std::move(objects)) {

}

void PathsCreator::createRoughPaths(int toolSize, Renderer &renderer) {
    static float FIRST_LAYER_Z = 33.5f;
    static float SECOND_LAYER_Z = 17.f;
    static float Y_DIFF = 8.f;
    static int Y_MOVES = 23;

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

        int texY = (BLOCK_SIZE_XY * 5.f - y) * TEX_SIZE / (10.f * BLOCK_SIZE_XY);
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

        int texY = (BLOCK_SIZE_XY * 5.f - y) * TEX_SIZE / (10.f * BLOCK_SIZE_XY);
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

    QProperty<weak_ptr<Object>> prop{};
    auto patchCreator = BicubicC0Creator({0, 0, 0}, &prop);
    patchCreator.setSegments({5, 5});
    patchCreator.setSize({2.5, 2.5});
    auto patch = static_pointer_cast<ParametricObject<2>>(patchCreator.create(0));

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
    auto mainRight = static_pointer_cast<Intersection>(
            intersect.calculateIntersection(renderer, {3.42344403, 0.896888852, 0.475813448, 1.58128047}));
    assert(mainRight);
    auto mainRightDistant = calculateToolDistantPath(*main, *mainRight, toolSize, MainRight);

    // main left
    intersect.setSurfaces({patch, main});
    auto mainLeft = static_pointer_cast<Intersection>(
            intersect.calculateIntersection(renderer, {1.83718324, 0.757870793, 3.52815628, 1.48308873}));
    assert(mainLeft);
    auto mainLeftDistant = calculateToolDistantPath(*main, *mainLeft, toolSize, MainLeft);

    // connect main right and main left;
    vector<XMFLOAT3> envelope(std::move(mainRightDistant));
    {
        auto paramsStart = mainRight->secondParameters()[mainRight->secondParameters().size() - 3];
        array<float, 2> paramStart = {paramsStart.first, paramsStart.second};
        auto tangentStart = main->tangent(paramStart);
        auto bitangentStart = main->bitangent(paramStart);
        auto normalStart = XMVector3Normalize(XMVector3Cross(bitangentStart, tangentStart));
        auto point = main->value({0.f, 0.f});

        auto paramsEnd = mainLeft->secondParameters()[2];
        array<float, 2> paramEnd = {paramsEnd.first, paramsEnd.second};
        auto tangentEnd = main->tangent(paramEnd);
        auto bitangentEnd = main->bitangent(paramEnd);
        auto normalEnd = XMVector3Normalize(XMVector3Cross(bitangentEnd, tangentEnd));

        insertSlerpNormalPosition(envelope, XMVectorScale(point, 10.f), normalStart, normalEnd,
                                  toolSize / 2.f);
        envelope.insert(envelope.end(), mainLeftDistant.begin(), mainLeftDistant.end());
    }

    // TODO: connect paths into envelope
//    auto [mainInter, handleInter] = findIntersection(envelope.begin() + 25, handleDistant.begin() + 4);
//    auto [mainInter2, handleInter2] = findIntersection(mainInter + 10, handleDistant.end() - 20);

    vector<XMFLOAT3> positions;

    // TODO: replace with correct
    positions.emplace_back(0.f, 0.f, START_Z);
    positions.emplace_back(START_X, -START_Y, START_Z);
    positions.emplace_back(START_X, -START_Y, BLOCK_BOTTOM_LOCAL);

//    positions.insert(positions.end(), mainDistant.begin(), mainDistant.end());
//    positions.insert(positions.end(), handleDistant.begin(), handleDistant.end());

    positions.emplace_back(START_X, START_Y, BLOCK_BOTTOM_LOCAL);
    positions.emplace_back(START_X, START_Y, START_Z);
    positions.emplace_back(0.f, 0.f, START_Z);
    FileParser::saveCNCPath(basePath / std::format("2.f{}", toolSize), positions);
}

void PathsCreator::createDetailPaths(int toolSize) {

}