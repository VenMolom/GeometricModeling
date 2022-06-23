//
// Created by Molom on 2022-06-23.
//

#include "intersectionInstance.h"
#include <DirectXMath.h>
#include <random>
#include <stack>
#include <QPainter>

using namespace std;
using namespace DirectX;

IntersectionInstance::IntersectionInstance(const vector<pair<float, float>> &parameters,
                                           const array<tuple<float, float>, 2> &range,
                                           bool closed, Renderer &renderer)
        : Renderable(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP) {

    const DxDevice &device = DxDevice::Instance();

    // CPU
    Texture2DDescription textureDesc(SIZE, SIZE);
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R32_FLOAT;
    textureDesc.Usage = D3D11_USAGE_STAGING;
    textureDesc.BindFlags = 0;
    textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    auto cpuTex = device.CreateTexture(textureDesc);

    // GPU
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;;
    textureDesc.CPUAccessFlags = 0;
    auto tex = device.CreateTexture(textureDesc);

    ShaderResourceViewDescription srvd;
    srvd.Format = DXGI_FORMAT_R32_FLOAT;
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvd.Texture2D.MipLevels = 1;
    srvd.Texture2D.MostDetailedMip = 0;
    _texture = device.CreateShaderResourceView(tex, srvd);
    _target = device.CreateRenderTargetView(tex);

    auto[uStart, uEnd] = range[0];
    auto[vStart, vEnd] = range[1];
    auto transform = XMMatrixTranslation(-uStart, -vStart, 0)
                     * XMMatrixScaling(1.f / (uEnd - uStart), 1.f / (vEnd - vStart), 1.0f)
                     * XMMatrixScaling(2.f, -2.f, 1.f)
                     * XMMatrixTranslation(-1, 1, 0);

    uint index = 0;
    // TODO: handle wrapped
    for (auto[u, v]: parameters) {
        XMFLOAT3 vec(u, v, 0);
        XMStoreFloat3(&vec, XMVector3TransformCoord(XMLoadFloat3(&vec), transform));
        vertices.push_back({vec, {1, 1, 1}});
        indices.push_back(index++);
    }
    if (closed) {
        indices.push_back(0);
    }
    updateBuffers();

    renderer.draw(*this);

    D3D11_MAPPED_SUBRESOURCE res;

    ID3D11Resource* sourceRes = nullptr;
    _texture->GetResource(&sourceRes);
    device.context()->CopyResource(cpuTex.get(), sourceRes);

    auto hr = device.context()->Map(cpuTex.get(), 0, D3D11_MAP_READ_WRITE, 0, &res);
    if (FAILED(hr))
        return;

    auto* data = reinterpret_cast<float*>(res.pData);

    floodFill(data);
    createPixmap(data);

    device.context()->Unmap(cpuTex.get(), 0);
    device.context()->CopyResource(sourceRes, cpuTex.get());

    sourceRes->Release();
}

void IntersectionInstance::floodFill(float *data) {
    //TODO: handle wrapped
    std::default_random_engine m_random{};
    static const uniform_int_distribution<int> posDistribution(0, SIZE - 1);

    int x,y, index;
    float pointData;
    do {
        x = posDistribution(m_random);
        y = posDistribution(m_random);
        index = y * static_cast<int>(SIZE) + x;

        pointData = data[index];
    } while (pointData != 1.f);

    stack<pair<int, int>> indexStack{};
    indexStack.push({x, y});

    while (!indexStack.empty()) {
        auto [u, v] = indexStack.top();
        indexStack.pop();

        if (u >= SIZE || u < 0 || v < 0 || v >= SIZE) continue;

        index = v * static_cast<int>(SIZE) + u;
        if (data[index] == 1.f) {
            data[index] = 0.f;
            indexStack.push({u + 1, v});
            indexStack.push({u - 1, v});
            indexStack.push({u, v + 1});
            indexStack.push({u, v - 1});
        }
    }
}

void IntersectionInstance::createPixmap(float *data) {
    QImage image{SIZE, SIZE, QImage::Format_RGB32};

    for (int x = 0; x < SIZE; ++x) {
        for (int y = 0; y < SIZE; ++y) {
            auto index = y * static_cast<int>(SIZE) + x;
            int value = static_cast<int>(data[index] * 255.f);

            image.setPixelColor(x, y, QColor::fromRgb(value, value, value));
        }
    }

    _pixmap = QPixmap::fromImage(image);
}
