//
// Created by Molom on 10/11/2022.
//

#include "pathsCreator.h"

#include <cassert>
#include <utility>
#include "Utils/fileParser.h"

using namespace std;
using namespace DirectX;

void PathsCreator::create(const filesystem::path &directory,
                          vector<shared_ptr<Object>> objects,
                          Renderer &renderer) {
    assert(filesystem::is_directory(directory));

    auto creator = PathsCreator(directory, std::move(objects));
    creator.createRoughPaths(16, renderer);
    creator.createFlatteningPaths(12);
    creator.createDetailPaths(8);
}

PathsCreator::PathsCreator(filesystem::path basePath, std::vector<std::shared_ptr<Object>> objects)
        : basePath(std::move(basePath)),
          objects(std::move(objects)) {

}

void PathsCreator::createRoughPaths(int toolSize, Renderer &renderer) {
    static float START_X = 88.f;
    static float START_Y = 88.f;
    static float START_Z = 66.f;
    static float FIRST_LAYER_Z = 33.5f;
    static float SECOND_LAYER_Z = 17.f;
    static float Y_DIFF = 8.f;
    static int Y_MOVES = 23;

    auto& device = DxDevice::Instance();
    auto [depth, staging] = createDepthTexture(device);
    auto viewMatrix = XMMatrixLookAtRH(XMVectorSet(0.f, BLOCK_SIZE_Z - BLOCK_BOTTOM, 0.f, 1.f),
                                       XMVectorSet(0.f, 0.f, 0.f, 1.f),
                                       XMVectorSet(0.f, 0.f, -1.f, 0.f));
    auto projectionMatrix = XMMatrixOrthographicRH(BLOCK_SIZE_XY, BLOCK_SIZE_XY, 0, BLOCK_SIZE_Z);

    renderer.drawToTexture(depth, make_pair(TEX_SIZE, TEX_SIZE), objects, projectionMatrix, viewMatrix);

    copyResource(device, depth, staging);

    D3D11_MAPPED_SUBRESOURCE res;

    auto hr = device.context()->Map(staging.get(), 0, D3D11_MAP_READ, 0, &res);
    if (FAILED(hr))
        return;

    float *data = reinterpret_cast<float *>(res.pData);

    vector<XMFLOAT3> positions;

    positions.emplace_back(0.f, 0.f, START_Z);

    //first layer
    positions.emplace_back(START_X, START_Y, START_Z);

    float x = START_X, y = START_Y;
    for (int i = 0; i < Y_MOVES; ++i) {
        positions.emplace_back(x, y, FIRST_LAYER_Z);
        positions.emplace_back(-x, y, FIRST_LAYER_Z);
        x = -x;
        y -= Y_DIFF;
    }
    y = -START_Y;

    //second layer
    for (int i = 0; i < Y_MOVES; ++i) {
        positions.emplace_back(x, y, SECOND_LAYER_Z);
        positions.emplace_back(-x, y, SECOND_LAYER_Z);
        x = -x;
        y += Y_DIFF;
    }

    positions.emplace_back(x, START_Y, START_Z);
    positions.emplace_back(0.f, 0.f, START_Z);

    device.context()->Unmap(staging.get(), 0);

    FileParser::saveCNCPath(basePath / std::format("1.k{}", toolSize), positions);
}

void PathsCreator::createFlatteningPaths(int toolSize) {

}

void PathsCreator::createDetailPaths(int toolSize) {

}

pair<mini::dx_ptr<ID3D11DepthStencilView>, mini::dx_ptr<ID3D11Texture2D>>
PathsCreator::createDepthTexture(const DxDevice &device) {
    Texture2DDescription texd;
    texd.Width = TEX_SIZE;
    texd.Height = TEX_SIZE;
    texd.MipLevels = 1;
    texd.Format = DXGI_FORMAT_D32_FLOAT;
    texd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    auto heightTexture = device.CreateTexture(texd);

    texd.Usage = D3D11_USAGE_STAGING;
    texd.BindFlags = 0;
    texd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    auto staging = device.CreateTexture(texd);

    DepthStencilViewDescription dvd;
    dvd.Format = DXGI_FORMAT_D32_FLOAT;
    auto depth = device.CreateDepthStencilView(heightTexture, dvd);

    device.context()->ClearDepthStencilView(depth.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f - 1.5f / 5.f, 0);
    return make_pair(std::move(depth), std::move(staging));
}

void PathsCreator::copyResource(const DxDevice &device, mini::dx_ptr<ID3D11DepthStencilView> &source,
                               mini::dx_ptr<ID3D11Texture2D> &target) {
    ID3D11Resource *sourceRes = nullptr;
    source->GetResource(&sourceRes);
    device.context()->CopyResource(target.get(), sourceRes);
}