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
        if (abs(height - prevHeight - slope) < 0.01f) {
            prevHeight = height;
            continue;
        }

        slope = height - prevHeight;
        prevHeight = height;
        positions.emplace_back(i * sizePerPixel - 5.f * xSize, y, height);
    }
}

vector<XMFLOAT3> PathsCreatorHelper::createZigZagLines(const float *data, int toolSize) {
    float lineDistance = static_cast<float>(toolSize) * 0.75f;
    int yMoves = ceil(10.f * BLOCK_SIZE_XY / lineDistance);

    vector<XMFLOAT3> zigZag;
    zigZag.emplace_back(START_X, -BLOCK_END_LOCAL, BLOCK_BOTTOM_LOCAL);
    float x = BLOCK_END_LOCAL;
    float y = -BLOCK_END_LOCAL;
    int dir = -1.f;

    zigZagLines(zigZag, data, yMoves + 1, lineDistance, y, x, dir);

    zigZag.emplace_back(x, y, BLOCK_BOTTOM_LOCAL);
    zigZag.emplace_back(-x, y, BLOCK_BOTTOM_LOCAL);

    x = -x;
    dir = -dir;
    y -= 3.f * lineDistance;
    zigZag.emplace_back(x, y, BLOCK_BOTTOM_LOCAL);

    zigZagLines(zigZag, data, 12, -lineDistance, y, x, dir);

    return zigZag;
}

void PathsCreatorHelper::zigZagLines(vector<XMFLOAT3> &zigZag, const float *data, int yMoves, float lineDistance,
                                     float &y, float &x, int &dir) {
    static const float sizePerPixel = BLOCK_SIZE_XY * 10.f / TEX_SIZE;

    int i = dir > 0 ? 0 : TEX_SIZE - 1;
    for (int j = 0; j < yMoves; ++j) {
        if (abs(y) > BLOCK_END_LOCAL) break;

        int texY = (TEX_SIZE - 1) * ((BLOCK_END_LOCAL - y) / (BLOCK_END_LOCAL * 2.f));
        while (i >= 0 && i < TEX_SIZE && data[texY * TEX_SIZE + i] < BLOCK_BOTTOM_LOCAL) i += dir;

        // line goes all the way
        if (i < 0 || i >= TEX_SIZE) {
            x = dir * START_X;
            zigZag.emplace_back(x, y, BLOCK_BOTTOM_LOCAL);

            // don't start next line if this is last one
            if (j == yMoves - 1) continue;

            i = clamp(i, 0, TEX_SIZE - 1);
            y += lineDistance;
            zigZag.emplace_back(x, y, BLOCK_BOTTOM_LOCAL);
            dir = -dir;

            continue;
        }

        // line needs to stop
        i -= dir;
        x = i * sizePerPixel - BLOCK_END_LOCAL;
        zigZag.emplace_back(x, y, BLOCK_BOTTOM_LOCAL);

        // don't start next line if this is last one
        if (j == yMoves - 1) continue;

        y += lineDistance;
        texY = (TEX_SIZE - 1) * ((BLOCK_END_LOCAL - y) / (BLOCK_END_LOCAL * 2.f));

        dir = -dir;

        if (j == 13) { //! hard coded for first zigzag path, where we need to go directly above and right.
            zigZag.emplace_back(x, y, BLOCK_BOTTOM_LOCAL);
        }

        while (i >= 0 && i < TEX_SIZE && data[texY * TEX_SIZE + i] < BLOCK_BOTTOM_LOCAL) i -= dir;
        while (data[texY * TEX_SIZE + i] > BLOCK_BOTTOM_LOCAL) i += dir;

        x = i * sizePerPixel - BLOCK_END_LOCAL;
        zigZag.emplace_back(x, y, BLOCK_BOTTOM_LOCAL);
    }
}

vector<XMFLOAT3>
PathsCreatorHelper::calculateToolDistantPath(ParametricObject<2> &patch, const vector<pair<float, float>> &parameters,
                                             const vector<XMFLOAT3> &points, int toolSize,
                                             FlatteningSegment segment) {
    static float MAX_ANGLE_COS = 0.5f;

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
            normal = XMVector3Cross(bitangent, tangent);
            normal.m128_f32[1] = 0;
            normal = XMVector3Normalize(normal);
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
    static int ANGLE_INTERPOLATION_STEPS = 20;

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
    tex.allowedHeight = move(allowedHeight);
    tex.allowedHeightStaging = move(allowedHeightStaging);
    tex.depth = move(depth);
    tex.depthTexture = move(depthTexture);

    return tex;
}

void PathsCreatorHelper::copyResource(const DxDevice &device, mini::dx_ptr<ID3D11UnorderedAccessView> &source,
                                      mini::dx_ptr<ID3D11Texture2D> &target) {
    ID3D11Resource *sourceRes = nullptr;
    source->GetResource(&sourceRes);
    device.context()->CopyResource(target.get(), sourceRes);
}

vector<XMFLOAT3> PathsCreatorHelper::intersectAndCalculateToolDistant(IntersectHandler &intersect,
                                                                      Renderer &renderer,
                                                                      shared_ptr<ParametricObject<2>> patch,
                                                                      shared_ptr<ParametricObject<2>> object,
                                                                      const array<float, 4> &starting,
                                                                      FlatteningSegment segment,
                                                                      int toolSize) {
    intersect.setSurfaces({move(patch), object});
    auto [params, points] = intersect.calculateIntersection(renderer, starting);
    assert(!params.empty());
    return calculateToolDistantPath(*object, params, points, toolSize, segment);
}

tuple<vector<XMFLOAT3>::iterator, vector<XMFLOAT3>::iterator, XMFLOAT3>
PathsCreatorHelper::findIntersection(vector<XMFLOAT3>::iterator path1, vector<XMFLOAT3>::iterator path2) {
    static const auto ccw = [](XMFLOAT3 p1, XMFLOAT3 p2, XMFLOAT3 q1) -> bool {
        float delta = (p2.y - p1.y) * (q1.x - p2.x) -
                      (p2.x - p1.x) * (q1.y - p2.y);
        return delta > 0.f;
    };

    for (int i = 0;; ++i) {
        auto e1 = *(path1 + i);
        auto e2 = *(path1 + i + 1);

        auto normal = XMVector3Normalize(XMVectorSet(e2.y - e1.y, e1.x - e2.x, 0, 0));

        for (int j = 0; j <= i; ++j) {
            auto p0 = *(path2 + j);
            auto p1 = *(path2 + j + 1);

            if (ccw(e1, e2, p0) != ccw(e1, e2, p1) && (ccw(p0, p1, e1) != ccw(p0, p1, e2))) {
                XMVECTOR d{p1.x - p0.x, p1.y - p0.y, 0, 0};
                auto dot = XMVector3Dot(normal, d).m128_f32[0];
                if (abs(dot) < FLT_EPSILON) {
                    continue;
                }

                XMVECTOR diff{p0.x - e1.x, p0.y - e1.y, 0, 0};
                float t = -XMVector3Dot(normal, diff).m128_f32[0] / dot;

                if (t >= 0 && t <= 1) {
                    XMFLOAT3 p{};
                    XMStoreFloat3(&p, XMVectorLerp(XMLoadFloat3(&p0), XMLoadFloat3(&p1), t));
                    return make_tuple(path1 + i, path2 + j, p);
                }
            }
        }
    }
}

vector<XMFLOAT3>::iterator
PathsCreatorHelper::findIntersectionHeight(vector<XMFLOAT3>::iterator path, float height) {
    for (int i = 0;; ++i) {
        auto e1 = *(path + i);
        auto e2 = *(path + i + 1);

        if (signbit(e1.y - height) != signbit(e2.y - height)) {
            return path + i;
        }
    }
    // TODO: return additionally t between points
}

pair<pair<float, float>, size_t>
PathsCreatorHelper::findIntersection(const vector<pair<float, float>> &path, pair<float, float> start,
                                     pair<float, float> end) {
    static const auto intersect = [](pair<float, float> u0, pair<float, float> v0,
                                     pair<float, float> u1, pair<float, float> v1) -> pair<float, float> {
        float d = v1.first * v0.second - v0.first * v1.second;
        return {
                1 / d * -(-(u0.first - u1.first) * v1.second + (u0.second - u1.second) * v1.first),
                1 / d * ((u0.first - u1.first) * v0.second - (u0.second - u1.second) * v0.first)
        };
    };

    auto u0 = start;
    auto v0 = make_pair(end.first - start.first, end.second - start.second);

    auto u1 = path[0];
    for (int i = 1; i < path.size(); ++i) {
        auto tmp = path[i];
        auto v1 = make_pair(tmp.first - u1.first, tmp.second - u1.second);

        auto [t, s] = intersect(u0, v0, u1, v1);
        u1 = tmp;
        if (isnan(t) || t < 0 || t > 1) continue;
        if (isnan(s) || s < 0 || s > 1) continue;

        return {{u0.first + t * v0.first, u0.second + t * v0.second}, static_cast<size_t>(i - 1)};
    }

    return {{NAN, NAN}, 0xffffffff};
}

vector<XMFLOAT3>
PathsCreatorHelper::createHandlePath(const vector<pair<float, float>> &topRing,
                                     const vector<pair<float, float>> &bottomRing,
                                     const vector<pair<float, float>> &insideLine,
                                     const vector<pair<float, float>> &outsideLine,
                                     const shared_ptr<ParametricObject<2>> &distant) {

    auto stepLength = 0.05f;
    auto pathsToSide = [stepLength, &distant](vector<XMFLOAT3> &handlePath,
                                              const vector<pair<float, float>> &firstEndRing,
                                              const vector<pair<float, float>> &firstStartRing,
                                              const vector<pair<float, float>> &sideRestriction,
                                              pair<float, float> startParam,
                                              pair<float, float> endParam,
                                              size_t endIdx, float sign, float pathStep) {
        size_t startIdx, nextEndIdx;
        for (int i = 0; i < 100; ++i) {
            auto param = startParam;
            XMFLOAT3 val;
            while (sign * param.second < sign * endParam.second) {
                XMStoreFloat3(&val, distant->value({param.first, param.second}));
                handlePath.emplace_back(val);
                param.second += sign * stepLength;
            }
            XMStoreFloat3(&val, distant->value({endParam.first, endParam.second}));
            handlePath.emplace_back(val);

            auto paramU = endParam.first + pathStep;
            // use different params to get start and end
            if (i % 2 == 0) {
                tie(startParam, startIdx) = findIntersection(firstEndRing, {paramU, -10}, {paramU, 10});
                tie(endParam, nextEndIdx) = findIntersection(firstStartRing, {paramU, -10}, {paramU, 10});
                if (startIdx == 0xffffffff || nextEndIdx == 0xffffffff) break;
                // if end and new start do not intersect same segment in params
                if (startIdx != endIdx) {
                    auto dir = startIdx > endIdx ? 1 : -1;
                    auto idx = endIdx;
                    // idx is index of segment first point, so when we move forward we need to start from second point
                    if (dir > 0) idx++;
                    while ((dir == -1 && idx != startIdx) || (dir == 1 && idx <= startIdx)) {
                        XMStoreFloat3(&val, distant->value({firstEndRing[idx].first, firstEndRing[idx].second}));
                        handlePath.emplace_back(val);
                        idx += dir;
                    }
                }

                if (startIdx == 0xffffffff) break;
            } else {
                tie(startParam, startIdx) = findIntersection(firstStartRing, {paramU, -10}, {paramU, 10});
                tie(endParam, nextEndIdx) = findIntersection(firstEndRing, {paramU, -10}, {paramU, 10});
                if (startIdx == 0xffffffff || nextEndIdx == 0xffffffff) break;
                // if end and new start do not intersect same segment in params
                if (startIdx != endIdx) {
                    auto dir = startIdx > endIdx ? 1 : -1;
                    auto idx = endIdx;
                    // idx is index of segment first point, so when we move forward we need to start from second point
                    if (dir > 0) idx++;
                    if (dir > 0) idx++;
                    while ((dir == -1 && idx != startIdx) || (dir == 1 && idx <= startIdx)) {
                        XMStoreFloat3(&val, distant->value({firstStartRing[idx].first, firstStartRing[idx].second}));
                        handlePath.emplace_back(val);
                        idx += dir;
                    }
                }
            }
            endIdx = nextEndIdx;
            sign = -sign;
        }
    };

    vector<XMFLOAT3> handlePath;
    auto sign = 1.f;
    auto pathStep = 0.05f;
    auto startParam = bottomRing[bottomRing.size() / 2];
    auto [endParam, endIdx] = findIntersection(topRing, startParam, {startParam.first, 10});
    pathsToSide(handlePath, topRing, bottomRing, insideLine, startParam, endParam, endIdx, sign, pathStep);

    pathStep = -pathStep;
    auto paramU = startParam.first + pathStep;
    tie(startParam, endIdx) = findIntersection(bottomRing, {paramU, -10}, {paramU, 10});
    tie(endParam, endIdx) = findIntersection(topRing, startParam, {startParam.first, 10});
    pathsToSide(handlePath, topRing, bottomRing, outsideLine, startParam, endParam, endIdx, sign, pathStep);

    return handlePath;
}
