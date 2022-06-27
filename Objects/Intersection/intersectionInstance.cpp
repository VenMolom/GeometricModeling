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
                                           const array<bool, 2> &looped,
                                           bool closed, Renderer &renderer)
        : Renderable(D3D11_PRIMITIVE_TOPOLOGY_LINELIST) {

    const DxDevice &device = DxDevice::Instance();

    auto cpuTex = createTargetAndTexture(device);

    auto[uStart, uEnd] = range[0];
    auto[vStart, vEnd] = range[1];
    auto transform = XMMatrixTranslation(-uStart, -vStart, 0)
                     * XMMatrixScaling(1.f / (uEnd - uStart), 1.f / (vEnd - vStart), 1.0f)
                     * XMMatrixScaling(2.f, -2.f, 1.f)
                     * XMMatrixTranslation(-1, 1, 0);

    fillBuffer(parameters, looped, closed, transform);

    renderer.draw(*this);

    mapAndFill(looped, device, cpuTex);
}

IntersectionInstance::IntersectionInstance(const vector<std::pair<float, float>> &firstParameters,
                                           const vector<std::pair<float, float>> &secondParameters,
                                           const array<std::tuple<float, float>, 2> &range,
                                           const array<bool, 2> &looped, bool closed, Renderer &renderer)
        : Renderable(D3D11_PRIMITIVE_TOPOLOGY_LINELIST) {

    const DxDevice &device = DxDevice::Instance();

    auto cpuTex = createTargetAndTexture(device);

    auto[uStart, uEnd] = range[0];
    auto[vStart, vEnd] = range[1];
    auto transform = XMMatrixTranslation(-uStart, -vStart, 0)
                     * XMMatrixScaling(1.f / (uEnd - uStart), 1.f / (vEnd - vStart), 1.0f)
                     * XMMatrixScaling(2.f, -2.f, 1.f)
                     * XMMatrixTranslation(-1, 1, 0);

    fillBuffer(firstParameters, looped, closed, transform);

    renderer.draw(*this);

    mapAndFill(looped, device, cpuTex, false);

    fillBuffer(secondParameters, looped, closed, transform);

    renderer.draw(*this, false);

    mapAndFill(looped, device, cpuTex, true);
}

mini::dx_ptr<ID3D11Texture2D> IntersectionInstance::createTargetAndTexture(const DxDevice &device) {
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

    return cpuTex;
}

void IntersectionInstance::fillBuffer(const vector<pair<float, float>> &parameters, const array<bool, 2> &looped,
                                      bool closed, XMMATRIX transform) {
    vertices.clear();
    indices.clear();

    uint index = 0;
    for (auto[u, v]: parameters) {
        XMFLOAT3 vec(u, v, 0);
        XMStoreFloat3(&vec, XMVector3TransformCoord(XMLoadFloat3(&vec), transform));

        // !may not work if no wrap happened but points are far away in parameter space
        if (!vertices.empty() && (looped[0] || looped[1])) {
            auto vv = vertices.back().position;
            auto vvv = vec;
            auto loop = false;
            if (looped[0] && abs(vv.x - vec.x) > 1.f) { // wrap happened
                auto sign = vec.x > 0 ? 1.f : -1.f;
                vv.x += 2.f * sign;
                vvv.x -= 2.f * sign;
                loop = true;
            }
            if (looped[1] && abs(vv.y - vec.y) > 1.f) { // wrap happened
                auto sign = vec.y > 0 ? 1.f : -1.f;
                vv.y += 2.f * sign;
                vvv.y -= 2.f * sign;
                loop = true;
            }
            if (loop) {
                vertices.push_back({vvv, {1, 1, 1}});
                indices.push_back(index++);

                vertices.push_back({vv, {1, 1, 1}});
                indices.push_back(index++);
            }
        }

        vertices.push_back({vec, {1, 1, 1}});

        if (vertices.size() > 1) {
            indices.push_back(index);
        }
        indices.push_back(index++);
    }

    if (closed) {
        indices.push_back(0);
    }
    updateBuffers();
}

void IntersectionInstance::mapAndFill(const array<bool, 2> &looped, const DxDevice &device,
                                      const mini::dx_ptr<ID3D11Texture2D> &stagingTex, bool withPixmap) {
    D3D11_MAPPED_SUBRESOURCE res;

    ID3D11Resource *sourceRes = nullptr;
    _texture->GetResource(&sourceRes);
    device.context()->CopyResource(stagingTex.get(), sourceRes);

    auto hr = device.context()->Map(stagingTex.get(), 0, D3D11_MAP_READ_WRITE, 0, &res);
    if (FAILED(hr))
        return;

    auto *data = reinterpret_cast<float *>(res.pData);

    floodFill(data, looped);
    if (withPixmap) createPixmap(data);

    device.context()->Unmap(stagingTex.get(), 0);
    device.context()->CopyResource(sourceRes, stagingTex.get());

    sourceRes->Release();
}

void IntersectionInstance::floodFill(float *data, const array<bool, 2> &looped) {
    auto[loopedU, loopedV] = looped;
    static default_random_engine m_random{};
    static const uniform_int_distribution<int> posDistribution(0, SIZE - 1);

    int x, y, index;
    float pointData;
    do {
        x = posDistribution(m_random);
        y = posDistribution(m_random);
        index = y * static_cast<int>(SIZE) + x;

        pointData = data[index];
    } while (abs(pointData - 0.5f) < 1e-3);

    auto startColor = round(pointData);
    auto fillColor = round(1.f - pointData);

    stack<pair<int, int>> indexStack{};
    indexStack.push({x, y});

    while (!indexStack.empty()) {
        auto[u, v] = indexStack.top();
        indexStack.pop();

        if (u >= SIZE || u < 0) {
            if (loopedU) {
                u = u - SIZE * (u / SIZE);
            } else {
                continue;
            }
        }

        if (v >= SIZE || v < 0) {
            if (loopedV) {
                v = v - SIZE * (v / SIZE);
            } else {
                continue;
            }
        }

        index = v * static_cast<int>(SIZE) + u;
        if (data[index] == startColor) {
            data[index] = fillColor;
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

            QColor color = abs(value - 127) < 3 ? QColor(255, 140, 0) : QColor(value, value, value);

            image.setPixelColor(x, y, color);
        }
    }

    _pixmap = QPixmap::fromImage(image);
}
