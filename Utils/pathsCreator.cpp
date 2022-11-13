//
// Created by Molom on 10/11/2022.
//

#include "pathsCreator.h"

#include <cassert>
#include <utility>
#include "Utils/fileParser.h"
#include "Handlers/intersectHandler.h"
#include "Objects/Patch/bicubicC0Creator.h"

using namespace std;
using namespace DirectX;

void PathsCreator::create(const filesystem::path &directory,
                          vector<shared_ptr<Object>> objects,
                          ObjectFactory &factory,
                          Renderer &renderer) {
    // objects order: main, handle, dziubek

    assert(filesystem::is_directory(directory));

    auto creator = PathsCreator(directory, std::move(objects));
//    creator.createRoughPaths(16, renderer);
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

    FileParser::saveCNCPath(basePath / std::format("1.k{}", toolSize), positions);
}

void PathsCreator::addPositionsOnLine(vector<XMFLOAT3> &positions, float *data, int dir, float baseZ, float xSize,
                                      float y, int texY) {
    auto sizePerPixel = xSize * 10.f / TEX_SIZE;
    auto prevHeight = baseZ;
    auto slope = 0.f;

    auto start = dir > 0 ? 0 : TEX_SIZE - 1;
    auto end = dir > 0 ? TEX_SIZE : -1;
    for (int i = start; i != end; i += dir) {
        int index = texY * TEX_SIZE + i;
        auto height = max(data[index], baseZ);

        // continue if same slope
        if (abs(height - prevHeight - slope) < 0.001f) {
            prevHeight = height;
            continue;
        }

        slope = height - prevHeight;
        prevHeight = height;
        positions.emplace_back(i * sizePerPixel - 5.f * xSize, y, height);
    }
}

void PathsCreator::createFlatteningPaths(int toolSize, Renderer &renderer, ObjectFactory &factory) {
// patch C0 5x5 scale (2.5, 1, 2.5)\

// U,V - patch; S,T - surface

// handle - internal: starting - {u=0.139966473, v=2.86276722, s=4.21201801, t=2.62888551}
// handle - external: starting - {u=4.56668663, v=2.79134393, s=2.47588515, t=2.7601614}


// main - left: starting - {u=3.79921484, v=5.70407104, s=1.50598717, t=2.7277267}
// main - right: starting - {u=3.42344403, v=0.896888852, s=0.475813448, t=1.58128047}

// dziubek- top: starting - {u=0.852348148, v=0.997951567, s=2.90446663, t=6.00563574,}
// dziubek - bottom: starting - {u=0.337863445, v=1.43801117, s=0.147527888, t=7.22353839}

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
    intersect.setSurfaces({patch, handle});
    auto handleExternal = static_pointer_cast<Intersection>(
            intersect.calculateIntersection(renderer, {4.56668663, 2.79134393, 2.47588515, 2.7601614}));
    assert(handleExternal);
    auto handleDistant = calculateToolDistantPath(*handle, *handleExternal, toolSize, HandleExternal);

    // main right
    intersect.setSurfaces({patch, main});
    auto mainRight = static_pointer_cast<Intersection>(
            intersect.calculateIntersection(renderer, {3.42344403, 0.896888852, 0.475813448, 1.58128047}));
    assert(mainRight);
    auto mainRightDistant = calculateToolDistantPath(*main, *mainRight, toolSize, MainRight);

    // main left
//    auto mainLeft = static_pointer_cast<Intersection>(
//            intersect.calculateIntersection(renderer, {3.42344403, 0.896888852, 0.475813448, 1.58128047}));
//    assert(mainRight);
//    auto mainLeftDistant = calculateToolDistantPath(*main, *mainLeft, toolSize, MainLeft);

    vector<XMFLOAT3> positions;

    // TODO: replace with correct
    positions.emplace_back(0.f, 0.f, START_Z);
    positions.emplace_back(START_X, -START_Y, START_Z);
    positions.emplace_back(START_X, -START_Y, BLOCK_BOTTOM_LOCAL);

//    positions.insert(positions.end(), handleDistant.begin(), handleDistant.end());
    positions.insert(positions.end(), mainRightDistant.begin(), mainRightDistant.end());

    positions.emplace_back(START_X, START_Y, BLOCK_BOTTOM_LOCAL);
    positions.emplace_back(START_X, START_Y, START_Z);
    positions.emplace_back(0.f, 0.f, START_Z);
    FileParser::saveCNCPath(basePath / std::format("2.f{}", toolSize), positions);
}

std::vector<DirectX::XMFLOAT3>
PathsCreator::calculateToolDistantPath(ParametricObject<2> &patch, Intersection &intersection, int toolSize,
                                       FlatteningSegment segment) {
    auto parameters = intersection.secondParameters();
    auto points = intersection.points();

    assert(parameters.size() == points.size());

    vector<XMFLOAT3> path(points.size());
    for (int i = 0; i < path.size(); ++i) {
        array<float, 2> params = {parameters[i].first, parameters[i].second};
        if (i >= points.size() - 2 && segment == MainRight) {
            params = {parameters[points.size() - 3].first, parameters[points.size() - 3].second};
        };

        auto tangent = patch.tangent(params);
        auto bitangent = patch.bitangent(params);
        auto normal = XMVector3Normalize(XMVector3Cross(bitangent, tangent));

        // override normal on end points
        if (i == 0 && segment == MainRight) {
            normal = XMVectorSet(0, 0, 1.f, 0);
        }

        auto step = XMVectorAdd(XMVectorScale(XMLoadFloat3(&points[i]), 10.f), XMVectorScale(normal, toolSize / 2.f));
        path[i] = {step.m128_f32[0], -step.m128_f32[2], BLOCK_BOTTOM_LOCAL};
    }

    // TODO: for Main interpolate on C0 edge

    return path;
}

void PathsCreator::createDetailPaths(int toolSize) {

}

PathsCreator::Textures PathsCreator::createDepthTextures(const DxDevice &device) {
    Texture2DDescription texd;
    texd.Width = TEX_SIZE;
    texd.Height = TEX_SIZE;
    texd.MipLevels = 1;
    texd.Format = DXGI_FORMAT_R32_TYPELESS;
    texd.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    auto heightTexture = device.CreateTexture(texd);

    DepthStencilViewDescription dvd;
    dvd.Format = DXGI_FORMAT_D32_FLOAT;
    auto depth = device.CreateDepthStencilView(heightTexture, dvd);

    ShaderResourceViewDescription srvd;
    srvd.Format = DXGI_FORMAT_R32_FLOAT;
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvd.Texture2D.MipLevels = 1;
    srvd.Texture2D.MostDetailedMip = 0;
    auto depthTexture = device.CreateShaderResourceView(heightTexture, srvd);

    texd.Format = DXGI_FORMAT_R32_FLOAT;
    texd.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
    auto unorderedTex = device.CreateTexture(texd);

    UnorderedAccessViewDescription uavd;
    uavd.Format = texd.Format;
    uavd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavd.Texture2D.MipSlice = 0;
    auto allowedHeight = device.CreateUnorderedAccessView(unorderedTex, uavd);

    texd.Usage = D3D11_USAGE_STAGING;
    texd.BindFlags = 0;
    texd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    auto allowedHeightStaging = device.CreateTexture(texd);

    device.context()->ClearDepthStencilView(depth.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

    Textures tex;
    tex.allowedHeight = std::move(allowedHeight);
    tex.allowedHeightStaging = std::move(allowedHeightStaging);
    tex.depth = std::move(depth);
    tex.depthTexture = std::move(depthTexture);

    return tex;
}

void PathsCreator::copyResource(const DxDevice &device, mini::dx_ptr<ID3D11UnorderedAccessView> &source,
                                mini::dx_ptr<ID3D11Texture2D> &target) {
    ID3D11Resource *sourceRes = nullptr;
    source->GetResource(&sourceRes);
    device.context()->CopyResource(target.get(), sourceRes);
}