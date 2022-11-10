//
// Created by Molom on 10/11/2022.
//

#include "pathsCreator.h"

#include <cassert>
#include <utility>

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
    auto depth = createDepthTexture(DxDevice::Instance());
    auto viewMatrix = XMMatrixLookAtRH(XMVectorSet(0.f, BLOCK_SIZE_Z - BLOCK_BOTTOM, 0.f, 1.f),
                                       XMVectorSet(0.f, 0.f, 0.f, 1.f),
                                       XMVectorSet(0.f, 0.f, -1.f, 0.f));
    auto projectionMatrix = XMMatrixOrthographicRH(BLOCK_SIZE_XY, BLOCK_SIZE_XY, 0, BLOCK_SIZE_Z);

    renderer.drawToTexture(depth, make_pair(TEX_SIZE, TEX_SIZE), objects, projectionMatrix, viewMatrix);
    int a = 5;
}

void PathsCreator::createFlatteningPaths(int toolSize) {

}

void PathsCreator::createDetailPaths(int toolSize) {

}

mini::dx_ptr<ID3D11DepthStencilView> PathsCreator::createDepthTexture(const DxDevice &device) {
    Texture2DDescription texd;
    texd.Width = TEX_SIZE;
    texd.Height = TEX_SIZE;
    texd.MipLevels = 1;
    texd.Format = DXGI_FORMAT_D32_FLOAT;
    texd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
//    texd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    auto heightTexture = device.CreateTexture(texd);

    DepthStencilViewDescription dvd;
    dvd.Format = DXGI_FORMAT_D32_FLOAT;
    auto depth = device.CreateDepthStencilView(heightTexture, dvd);

    device.context()->ClearDepthStencilView(depth.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f - 1.5f / 5.f, 0);
    return depth;
}
