//
// Created by Molom on 14/11/2022.
//

#include "pathsCreatorHelper.h"

#include <utility>

using namespace std;
using namespace DirectX;
using namespace PathsCreatorHelper;

void PathsCreatorHelper::addPositionsOnLine(vector<XMFLOAT3> &positions, float *data, int dir, float baseZ, float xSize,
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

std::vector<DirectX::XMFLOAT3>
PathsCreatorHelper::calculateToolDistantPath(ParametricObject<2> &patch, Intersection &intersection, int toolSize,
                                             FlatteningSegment segment) {
    static float MAX_ANGLE_COS = 0.5f;

    auto parameters = intersection.secondParameters();
    auto points = intersection.points();

    assert(parameters.size() == points.size());

    XMVECTOR prevNormal;
    vector<XMFLOAT3> path;
    path.reserve(points.size());
    for (int i = 0; i < points.size(); ++i) {
        array<float, 2> params = {parameters[i].first, parameters[i].second};
        if (i >= points.size() - 2 && segment == MainRight) {
            params = {parameters[points.size() - 3].first, parameters[points.size() - 3].second};
        }
        if (i < 2 && segment == MainLeft) {
            params = {parameters[2].first, parameters[2].second};
        }

        XMVECTOR normal;
        // override normal on end points
        if ((i == 0 && segment == MainRight) || (i == points.size() - 1 && segment == MainLeft)) {
            normal = XMVectorSet(0, 0, 1.f, 0);
        } else {
            auto tangent = patch.tangent(params);
            auto bitangent = patch.bitangent(params);
            normal = XMVector3Normalize(XMVector3Cross(bitangent, tangent));
        }

        if (i > 0 && XMVector3Dot(prevNormal, normal).m128_f32[0] < MAX_ANGLE_COS) {
            array<float, 2> midPointParams = {(parameters[i].first + parameters[i - 1].first) * 0.5f,
                                              (parameters[i].second + parameters[i - 1].second) * 0.5f};
            auto point = XMVectorScale(patch.value(midPointParams), 10.f);
            insertSlerpNormalPosition(path, point, prevNormal, normal, toolSize / 2.f);
        }

        auto step = XMVectorAdd(XMVectorScale(XMLoadFloat3(&points[i]), 10.f), XMVectorScale(normal, toolSize / 2.f));
        path.emplace_back(step.m128_f32[0], -step.m128_f32[2], BLOCK_BOTTOM_LOCAL);

        prevNormal = normal;
    }

    return path;
}

void PathsCreatorHelper::insertSlerpNormalPosition(vector<XMFLOAT3> &path, XMVECTOR position, XMVECTOR normalStart,
                                                   XMVECTOR normalEnd, float normalLength) {
    static int ANGLE_INTERPOLATION_STEPS = 12;

    for (int j = 1; j < ANGLE_INTERPOLATION_STEPS; ++j) {
        auto slerpNormal = XMVector3Normalize(
                XMVectorLerp(normalStart, normalEnd, static_cast<float>(j) / ANGLE_INTERPOLATION_STEPS));
        auto step = XMVectorAdd(position, XMVectorScale(slerpNormal, normalLength));
        path.emplace_back(step.m128_f32[0], -step.m128_f32[2], BLOCK_BOTTOM_LOCAL);
    }
}

PathsCreatorHelper::Textures PathsCreatorHelper::createDepthTextures(const DxDevice &device) {
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

void PathsCreatorHelper::copyResource(const DxDevice &device, mini::dx_ptr<ID3D11UnorderedAccessView> &source,
                                      mini::dx_ptr<ID3D11Texture2D> &target) {
    ID3D11Resource *sourceRes = nullptr;
    source->GetResource(&sourceRes);
    device.context()->CopyResource(target.get(), sourceRes);
}

std::vector<DirectX::XMFLOAT3>
PathsCreatorHelper::intersectAndCalculateToolDistant(IntersectHandler &intersect,
                                                     Renderer &renderer,
                                                     shared_ptr<ParametricObject<2>> patch,
                                                     shared_ptr<ParametricObject<2>> object,
                                                     const array<float, 4> &starting,
                                                     FlatteningSegment segment,
                                                     int toolSize) {
    intersect.setSurfaces({patch, object});
    auto intersection = static_pointer_cast<Intersection>(
            intersect.calculateIntersection(renderer, starting));
    assert(intersection);
    return calculateToolDistantPath(*object, *intersection, toolSize, segment);
}
