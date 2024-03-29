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
    auto [_, params, points] = intersect.calculateIntersection(renderer, starting);
    assert(!params.empty());
    return calculateToolDistantPath(*object, params, points, toolSize, segment);
}

tuple<vector<XMFLOAT3>::iterator, vector<XMFLOAT3>::iterator, XMFLOAT3>
PathsCreatorHelper::findIntersection(vector<XMFLOAT3>::iterator path1, vector<XMFLOAT3>::iterator path2) {
    for (int i = 0;; ++i) {
        auto e1 = *(path1 + i);
        auto e2 = *(path1 + i + 1);

        auto da = XMVectorSet(e2.x - e1.x, e2.y - e1.y, e2.z - e1.z, 0);

        for (int j = 0; j <= i; ++j) {
            auto p0 = *(path2 + j);
            auto p1 = *(path2 + j + 1);

            auto db = XMVectorSet(p1.x - p0.x, p1.y - p0.y, p1.z - p0.z, 0);
            auto dc = XMVectorSet(p0.x - e1.x, p0.y - e1.y, p0.z - e1.z, 0);

            if (abs(XMVector3Dot(dc, XMVector3Cross(da, db)).m128_f32[0]) < 1e-3) {
                auto norm = XMVector3LengthSq(XMVector3Cross(da, db)).m128_f32[0];
                float t = XMVector3Dot(XMVector3Cross(dc, db), XMVector3Cross(da, db)).m128_f32[0] / norm;
                float s = XMVector3Dot(XMVector3Cross(dc, da), XMVector3Cross(da, db)).m128_f32[0] / norm;

                if ((t >= 0 && t <= 1) && (s >= 0 && s <= 1)) {
                    XMFLOAT3 p{};
                    XMStoreFloat3(&p, XMVectorLerp(XMLoadFloat3(&e1), XMLoadFloat3(&e2), t));
                    return make_tuple(path1 + i, path2 + j, p);
                }
            }
        }
    }
}

pair<vector<XMFLOAT3>::const_iterator, XMFLOAT3>
PathsCreatorHelper::findIntersection(vector<XMFLOAT3>::const_iterator path1, XMFLOAT3 start, XMFLOAT3 end) {
    auto e1 = start;
    auto e2 = end;

    auto da = XMVectorSet(e2.x - e1.x, e2.y - e1.y, e2.z - e1.z, 0);

    for (int i = 0;; ++i) {
        auto p0 = *(path1 + i);
        auto p1 = *(path1 + i + 1);

        auto db = XMVectorSet(p1.x - p0.x, p1.y - p0.y, p1.z - p0.z, 0);
        auto dc = XMVectorSet(p0.x - e1.x, p0.y - e1.y, p0.z - e1.z, 0);

        if (abs(XMVector3Dot(dc, XMVector3Cross(da, db)).m128_f32[0]) < 1e-3) {
            auto norm = XMVector3LengthSq(XMVector3Cross(da, db)).m128_f32[0];
            float t = XMVector3Dot(XMVector3Cross(dc, db), XMVector3Cross(da, db)).m128_f32[0] / norm;
            float s = XMVector3Dot(XMVector3Cross(dc, da), XMVector3Cross(da, db)).m128_f32[0] / norm;

            if ((t >= -1e-2 && t <= 1 + 1e-3) && (s >= 0 && s <= 1)) {
                XMFLOAT3 p{};
                XMStoreFloat3(&p, XMVectorLerp(XMLoadFloat3(&e1), XMLoadFloat3(&e2), t));
                return {path1 + i, p};
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

pair<vector<XMFLOAT3>::iterator, XMFLOAT3>
PathsCreatorHelper::findIntersectionHeightInter(vector<XMFLOAT3>::iterator path, float height) {
    for (int i = 0;; ++i) {
        auto e1 = *(path + i);
        auto e2 = *(path + i + 1);

        if (signbit(e1.y - height) != signbit(e2.y - height)) {

            auto t = (height - e1.y) / (e2.y - e1.y);
            XMFLOAT3 res = {e1.x + (e2.x - e1.x) * t, height, e1.z + (e2.z - e1.z) * t};
            return {path + i, res};
        }
    }
}

pair<pair<float, float>, size_t>
PathsCreatorHelper::findIntersection(const vector<pair<float, float>> &path,
                                     pair<float, float> start,
                                     pair<float, float> end,
                                     bool mod, bool patch) {
    static const auto intersect = [](pair<float, float> u0, pair<float, float> v0,
                                     pair<float, float> u1, pair<float, float> v1) -> pair<float, float> {
        float d = v1.first * v0.second - v0.first * v1.second;
        return {
                1 / d * -(-(u0.first - u1.first) * v1.second + (u0.second - u1.second) * v1.first),
                1 / d * ((u0.first - u1.first) * v0.second - (u0.second - u1.second) * v0.first)
        };
    };

    if (mod && start.first < 1.f) {
        start.first += 6.f;
        end.first += 6.f;
    }

    auto u0 = start;
    auto v0 = make_pair(end.first - start.first, end.second - start.second);

    auto u1 = path[0];
    for (int i = 1; i < path.size(); ++i) {
        auto tmp = path[i];

        if (mod) {
            if ((u1.first > 5.f && tmp.first < 1.f)) {
                tmp.first += 6.f;
            } else if (tmp.first > 5.f && u1.first < 1.f) {
                u1.first += 6.f;
            } else if (!patch && tmp.first < 1.f && u1.first < 1.f) {
                tmp.first += 6.f;
                u1.first += 6.f;
            }
        } else if (patch && ((u1.first > 5.f && tmp.first < 1.f) || (tmp.first > 5.f && u1.first < 1.f))) {
            continue;
        } else if ((u1.first > 5.f && tmp.first < 1.f) || tmp.first > 5.f && u1.first < 1.f) {
            u1 = path[i];
            continue;
        }

        auto v1 = make_pair(tmp.first - u1.first, tmp.second - u1.second);

        auto [t, s] = intersect(u0, v0, u1, v1);
        if (patch) { u1 = tmp; }
        else { u1 = path[i]; }
        if (isnan(t) || t < 0 || t > 1) continue;
        if (isnan(s) || s < 0 || s > 1) continue;

        pair<float, float> res = {u0.first + t * v0.first, u0.second + t * v0.second};
        if (mod && res.first >= 6.f) {
            res.first -= 6.f;
        }
        return {res, static_cast<size_t>(i - 1)};
    }

    return {{NAN, NAN}, 0xffffffff};
}

pair<pair<float, float>, size_t>
PathsCreatorHelper::findIntersectionEnd(const vector<pair<float, float>> &path,
                                        pair<float, float> start,
                                        pair<float, float> end, bool mod) {
    static const auto intersect = [](pair<float, float> u0, pair<float, float> v0,
                                     pair<float, float> u1, pair<float, float> v1) -> pair<float, float> {
        float d = v1.first * v0.second - v0.first * v1.second;
        return {
                1 / d * -(-(u0.first - u1.first) * v1.second + (u0.second - u1.second) * v1.first),
                1 / d * ((u0.first - u1.first) * v0.second - (u0.second - u1.second) * v0.first)
        };
    };

    if (mod && start.first < 1.f) {
        start.first += 6.f;
        end.first += 6.f;
    }

    auto u0 = start;
    auto v0 = make_pair(end.first - start.first, end.second - start.second);

    auto u1 = path.back();
    for (int i = path.size() - 2; i >= 0; --i) {
        auto tmp = path[i];

        if (mod) {
            if ((u1.first > 5.f && tmp.first < 1.f)) {
                tmp.first += 6.f;
            } else if (tmp.first > 5.f && u1.first < 1.f) {
                u1.first += 6.f;
            }
        }

        auto v1 = make_pair(tmp.first - u1.first, tmp.second - u1.second);

        auto [t, s] = intersect(u0, v0, u1, v1);
        u1 = tmp;
        if (isnan(t) || t < 0 || t > 1) continue;
        if (isnan(s) || s < 0 || s > 1) continue;

        pair<float, float> res = {u0.first + t * v0.first, u0.second + t * v0.second};
        if (mod && res.first >= 6.f) {
            res.first -= 6.f;
        }
        return {res, static_cast<size_t>(i)};
    }

    return {{NAN, NAN}, 0xffffffff};
}

vector<XMFLOAT3>
PathsCreatorHelper::createHandlePath(const vector<pair<float, float>> &topRing,
                                     const vector<pair<float, float>> &bottomRing,
                                     const vector<pair<float, float>> &insideLine,
                                     const vector<pair<float, float>> &outsideLine,
                                     const shared_ptr<ParametricObject<2>> &distant) {

    auto stepLength = 0.075f;
    auto pathsToSide = [stepLength, &distant](vector<XMFLOAT3> &handlePath,
                                              const vector<pair<float, float>> &firstEndRing,
                                              const vector<pair<float, float>> &firstStartRing,
                                              vector<pair<float, float>> sideRestriction,
                                              pair<float, float> startParam,
                                              pair<float, float> endParam,
                                              size_t endIdx, float sign, float pathStep) {
        size_t startIdx, nextEndIdx;
        size_t restrictionEndIdx, restrictionStartIdx;
        pair<float, float> restrictionStart, restrictionEnd;
        for (int i = 0; i < 100; ++i) {
            auto param = startParam;

            // find intersection with side restriction
            tie(restrictionStart, restrictionStartIdx) = findIntersection(sideRestriction, startParam, endParam);
            bool hasRestriction = false;
            if (restrictionStartIdx != 0xffffffff) {
                // intersection exists
                tie(restrictionEnd, restrictionEndIdx) = findIntersectionEnd(sideRestriction, startParam, endParam);

                // swap ends if needed
                if (abs(startParam.second - restrictionEnd.second) < abs(startParam.second - restrictionStart.second)) {
                    std::swap(restrictionStart, restrictionEnd);
                    std::swap(restrictionStartIdx, restrictionEndIdx);
                }

                hasRestriction = true;
            }

            XMFLOAT3 val;
            while (sign * param.second < sign * endParam.second) {
                // check if there will be collision
                if (hasRestriction && sign * param.second > sign * restrictionStart.second) {
                    XMStoreFloat3(&val, distant->value({restrictionStart.first, restrictionStart.second}));
                    handlePath.emplace_back(val);
                    auto dir = restrictionEndIdx > restrictionStartIdx ? 1 : -1;
                    auto idx = restrictionStartIdx;
                    // idx is index of segment first point, so when we move forward we need to start from second point
                    if (dir > 0) idx++;
                    while ((dir == -1 && idx != restrictionEndIdx) || (dir == 1 && idx <= restrictionEndIdx)) {
                        XMStoreFloat3(&val, distant->value({sideRestriction[idx].first, sideRestriction[idx].second}));
                        handlePath.emplace_back(val);
                        idx += dir;
                    }
                    XMStoreFloat3(&val, distant->value({restrictionEnd.first, restrictionEnd.second}));
                    handlePath.emplace_back(val);
                    param.second = restrictionEnd.second + sign * stepLength;
                    hasRestriction = false;
                }

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

    auto &lastPoint = handlePath.back();
    handlePath.emplace_back(lastPoint.x, lastPoint.y + 1.f, lastPoint.z);

    pathStep = -pathStep;
    auto paramU = startParam.first + pathStep;
    tie(startParam, endIdx) = findIntersection(bottomRing, {paramU, -10}, {paramU, 10});
    tie(endParam, endIdx) = findIntersection(topRing, startParam, {startParam.first, 10});

    auto startPoint = distant->value({startParam.first, startParam.second});
    handlePath.emplace_back(startPoint.m128_f32[0], lastPoint.y + 1.f, startPoint.m128_f32[2]);
    pathsToSide(handlePath, topRing, bottomRing, outsideLine, startParam, endParam, endIdx, sign, pathStep);

    return handlePath;
}

vector<XMFLOAT3>
PathsCreatorHelper::createHandleContour(const vector<XMFLOAT3> &topRing, const vector<XMFLOAT3> &bottomRing,
                                        const vector<XMFLOAT3> &insideLine, const vector<XMFLOAT3> &outsideLine) {
    vector<XMFLOAT3> handleContour;
    handleContour.insert(handleContour.end(), bottomRing.rbegin(), bottomRing.rend());
    handleContour.insert(handleContour.end(), insideLine.rbegin() + 1, insideLine.rend());
    handleContour.insert(handleContour.end(), topRing.rbegin() + 1, topRing.rend());
    handleContour.insert(handleContour.end(), outsideLine.rbegin() + 1, outsideLine.rend());

    return handleContour;
}

vector<XMFLOAT3>
PathsCreatorHelper::createDziubekPath(const vector<pair<float, float>> &outline,
                                      const vector<pair<float, float>> &mainRing,
                                      const shared_ptr<ParametricObject<2>> &distant) {
    auto stepLength = 0.075f;
    auto pathsToSide = [stepLength, &distant, &outline, &mainRing](vector<XMFLOAT3> &handlePath,
                                                                   pair<float, float> startParam,
                                                                   pair<float, float> endParam,
                                                                   size_t endIdx, float sign, float pathStep) {
        size_t startIdx, nextEndIdx;
        size_t restrictionEndIdx, restrictionStartIdx;
        pair<float, float> restrictionStart, restrictionEnd;
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
                tie(startParam, startIdx) = findIntersection(outline, {paramU, -10}, {paramU, 10});
                tie(endParam, nextEndIdx) = findIntersection(mainRing, {paramU, -10}, {paramU, 10});
                if (startIdx == 0xffffffff || nextEndIdx == 0xffffffff) break;
                // if end and new start do not intersect same segment in params
                if (startIdx != endIdx) {
                    auto dir = startIdx > endIdx ? 1 : -1;
                    auto idx = endIdx;
                    // idx is index of segment first point, so when we move forward we need to start from second point
                    if (dir > 0) idx++;
                    while ((dir == -1 && idx != startIdx) || (dir == 1 && idx <= startIdx)) {
                        XMStoreFloat3(&val, distant->value({outline[idx].first, outline[idx].second}));
                        handlePath.emplace_back(val);
                        idx += dir;
                    }
                }

                if (startIdx == 0xffffffff) break;
            } else {
                tie(startParam, startIdx) = findIntersection(mainRing, {paramU, -10}, {paramU, 10});
                tie(endParam, nextEndIdx) = findIntersection(outline, {paramU, -10}, {paramU, 10});
                if (startIdx == 0xffffffff || nextEndIdx == 0xffffffff) break;
                // if end and new start do not intersect same segment in params
                if (startIdx != endIdx) {
                    auto dir = startIdx > endIdx ? 1 : -1;
                    auto idx = endIdx;
                    // idx is index of segment first point, so when we move forward we need to start from second point
                    if (dir > 0) idx++;
                    while ((dir == -1 && idx != startIdx) || (dir == 1 && idx <= startIdx)) {
                        XMStoreFloat3(&val, distant->value({mainRing[idx].first, mainRing[idx].second}));
                        handlePath.emplace_back(val);
                        idx += dir;
                    }
                }
            }
            endIdx = nextEndIdx;
            sign = -sign;
        }
    };

    vector<XMFLOAT3> dziubekPath;
    auto sign = 1.f;
    auto pathStep = 0.05f;
    auto startParam = mainRing[mainRing.size() / 2];
    auto [endParam, endIdx] = findIntersection(outline, startParam, {startParam.first, 10});
    pathsToSide(dziubekPath, startParam, endParam, endIdx, sign, pathStep);

    auto &lastPoint = dziubekPath.back();
    dziubekPath.emplace_back(lastPoint.x, lastPoint.y + 1.f, lastPoint.z);

    pathStep = -pathStep;
    auto paramU = startParam.first + pathStep;
    tie(startParam, endIdx) = findIntersection(mainRing, {paramU, -10}, {paramU, 10});
    tie(endParam, endIdx) = findIntersection(outline, startParam, {startParam.first, 10});

    auto startPoint = distant->value({startParam.first, startParam.second});
    dziubekPath.emplace_back(startPoint.m128_f32[0], lastPoint.y + 1.f, startPoint.m128_f32[2]);
    pathsToSide(dziubekPath, startParam, endParam, endIdx, sign, pathStep);

    return dziubekPath;
}

vector<XMFLOAT3>
PathsCreatorHelper::createDziubekContour(const vector<XMFLOAT3> &outline, const vector<XMFLOAT3> &mainRing) {
    vector<XMFLOAT3> dziubekContour;
    dziubekContour.insert(dziubekContour.end(), outline.begin(), outline.end());
    dziubekContour.insert(dziubekContour.end(), mainRing.begin() + 1, mainRing.end());

    return dziubekContour;
}

void PathsCreatorHelper::transformAndAppend(vector<XMFLOAT3> &positions, const vector<XMFLOAT3> &path, float toolSize) {
    for (auto &point: path) {
        positions.emplace_back(point.x * 10.f, -point.z * 10.f,
                               max(point.y * 10.f + BLOCK_BOTTOM_LOCAL - toolSize / 2.f, BLOCK_BOTTOM_LOCAL));
    }
}

vector<XMFLOAT3>
PathsCreatorHelper::createMainContour(const vector<XMFLOAT3> &outline,
                                      const vector<XMFLOAT3> &midRing,
                                      const vector<XMFLOAT3> &dziubekRing,
                                      const vector<XMFLOAT3> &topRing,
                                      const vector<XMFLOAT3> &bottomRing) {
    vector<XMFLOAT3> mainContour;
    {
        auto dziubekIter1 = findIntersection(outline.begin(), dziubekRing[0], dziubekRing[1]).first;
        auto dziubekIter2 = findIntersection(outline.begin(), dziubekRing[dziubekRing.size() - 2],
                                             dziubekRing.back()).first;

        mainContour.insert(mainContour.end(), outline.begin(), dziubekIter2 + 1);
        mainContour.insert(mainContour.end(), dziubekRing.rbegin(), dziubekRing.rend());

        auto bottomIter1 = findIntersection(outline.begin() + outline.size() / 2, bottomRing[0], bottomRing[1]).first;
        auto bottomIter2 = findIntersection(outline.begin() + outline.size() / 2, bottomRing[bottomRing.size() - 2],
                                            bottomRing.back()).first;
        mainContour.insert(mainContour.end(), dziubekIter1 + 1, bottomIter2 + 1);
        mainContour.insert(mainContour.end(), bottomRing.rbegin(), bottomRing.rend());

        auto topIter1 = findIntersection(outline.begin() + 4 * outline.size() / 5, topRing[0], topRing[1]).first;
        auto topIter2 = findIntersection(outline.begin() + 4 * outline.size() / 5, topRing[topRing.size() - 2],
                                         topRing.back()).first;
        mainContour.insert(mainContour.end(), bottomIter1 + 1, topIter2 + 1);
        mainContour.insert(mainContour.end(), topRing.rbegin(), topRing.rend());
        mainContour.insert(mainContour.end(), topIter1 + 1, outline.end());

        mainContour.insert(mainContour.end(), midRing.rbegin() + 1, midRing.rend());
    }

    return mainContour;
}

vector<XMFLOAT3>
PathsCreatorHelper::createMainPath(const vector<pair<float, float>> &outline,
                                   const vector<pair<float, float>> &midRing,
                                   const vector<pair<float, float>> &dziubekRing,
                                   const vector<pair<float, float>> &topRing,
                                   const vector<pair<float, float>> &bottomRing,
                                   const shared_ptr<ParametricObject<2>> &distant) {
    const float V = 3.000001f;
    auto stepLength = 0.15f;
    auto pathsToSide = [stepLength, &distant, &outline, &midRing, V](vector<XMFLOAT3> &handlePath,
                                                                     const vector<pair<float, float>> &sideRestriction1,
                                                                     const vector<pair<float, float>> &sideRestriction2,
                                                                     pair<float, float> startParam,
                                                                     pair<float, float> endParam,
                                                                     size_t endIdx, float sign, float pathStep,
                                                                     bool mod) {

        size_t startIdx, nextEndIdx;
        vector<pair<size_t, size_t>> restrictionIdxs;
        vector<tuple<pair<float, float>, pair<float, float>, const vector<pair<float, float>> *>> restrictions;
        for (int i = 0; i < 100; ++i) {
            auto param = startParam;

            // find intersection with side restriction
            restrictions.clear();
            restrictionIdxs.clear();
            auto [restrictionStart, restrictionStartIdx] = findIntersection(sideRestriction1, startParam, endParam,
                                                                            mod, true);
            if (restrictionStartIdx != 0xffffffff) {
                // intersection exists
                auto [restrictionEnd, restrictionEndIdx] = findIntersectionEnd(sideRestriction1, startParam, endParam,
                                                                               mod);

                // swap ends if needed
                if (abs(startParam.second - restrictionEnd.second) < abs(startParam.second - restrictionStart.second)) {
                    std::swap(restrictionStart, restrictionEnd);
                    std::swap(restrictionStartIdx, restrictionEndIdx);
                }

                restrictions.emplace_back(restrictionStart, restrictionEnd, &sideRestriction1);
                restrictionIdxs.emplace_back(restrictionStartIdx, restrictionEndIdx);
            }

            tie(restrictionStart, restrictionStartIdx) = findIntersection(sideRestriction2, startParam, endParam, mod,
                                                                          true);
            if (restrictionStartIdx != 0xffffffff) {
                // intersection exists
                auto [restrictionEnd, restrictionEndIdx] = findIntersectionEnd(sideRestriction2, startParam, endParam,
                                                                               mod);

                // swap ends if needed
                if (abs(startParam.second - restrictionEnd.second) < abs(startParam.second - restrictionStart.second)) {
                    std::swap(restrictionStart, restrictionEnd);
                    std::swap(restrictionStartIdx, restrictionEndIdx);
                }

                int offset = 0;
                if (!restrictions.empty() &&
                    abs(startParam.second - restrictionStart.second) >
                    abs(startParam.second - get<0>(restrictions.front()).second)) {
                    offset = 1;
                }

                restrictions.insert(restrictions.begin() + offset,
                                    {restrictionStart, restrictionEnd, &sideRestriction2});
                restrictionIdxs.insert(restrictionIdxs.begin() + offset, {restrictionStartIdx, restrictionEndIdx});
            }

            XMFLOAT3 val;
            while (sign * param.second < sign * endParam.second) {
                // check if there will be collision
                if (!restrictions.empty() && sign * param.second > sign * get<0>(restrictions.front()).second) {
                    auto resStart = get<0>(restrictions.front());
                    auto resEnd = get<1>(restrictions.front());
                    auto restriction = get<2>(restrictions.front());
                    auto resStartIdx = restrictionIdxs.front().first;
                    auto resEndIdx = restrictionIdxs.front().second;
                    XMStoreFloat3(&val, distant->value({resStart.first, resStart.second}));
                    handlePath.emplace_back(val);
                    auto dir = resEndIdx > resStartIdx ? 1 : -1;
                    auto idx = resStartIdx;
                    // idx is index of segment first point, so when we move forward we need to start from second point
                    if (dir > 0) idx++;
                    while ((dir == -1 && idx != resEndIdx) || (dir == 1 && idx <= resEndIdx)) {
                        XMStoreFloat3(&val,
                                      distant->value({(*restriction)[idx].first, (*restriction)[idx].second}));
                        handlePath.emplace_back(val);
                        idx += dir;
                    }
                    XMStoreFloat3(&val, distant->value({resEnd.first, resEnd.second}));
                    handlePath.emplace_back(val);
                    param.second = resEnd.second + sign * stepLength;
                    restrictions.erase(restrictions.begin());
                    restrictionIdxs.erase(restrictionIdxs.begin());
                }

                XMStoreFloat3(&val, distant->value({param.first, param.second}));
                handlePath.emplace_back(val);
                param.second += sign * stepLength;
            }
            XMStoreFloat3(&val, distant->value({endParam.first, endParam.second}));
            handlePath.emplace_back(val);

            auto paramU = endParam.first + pathStep;
            if (paramU > 6.f) paramU -= 6.f;
            // use different params to get start and end
            if (i % 2 == 0) {
                tie(startParam, startIdx) = findIntersection(outline, {paramU, -20}, {paramU, 20}, mod);
                tie(endParam, nextEndIdx) = findIntersection(midRing, {paramU, -20}, {paramU, 20}, mod);
                endParam.second = V;
                if (startIdx == 0xffffffff || nextEndIdx == 0xffffffff || endParam.second > startParam.second) {
                    break;
                }
                // if end and new start do not intersect same segment in params
                if (startIdx != endIdx) {
                    auto dir = startIdx > endIdx ? 1 : -1;
                    auto idx = endIdx;
                    // idx is index of segment first point, so when we move forward we need to start from second point
                    if (dir > 0) idx++;
                    while ((dir == -1 && idx != startIdx) || (dir == 1 && idx <= startIdx)) {
                        XMStoreFloat3(&val, distant->value({outline[idx].first, outline[idx].second}));
                        handlePath.emplace_back(val);
                        idx += dir;
                    }
                }

                if (startIdx == 0xffffffff) break;
            } else {
                tie(startParam, startIdx) = findIntersection(midRing, {paramU, -20}, {paramU, 20}, mod);
                tie(endParam, nextEndIdx) = findIntersection(outline, {paramU, -20}, {paramU, 20}, mod);
                startParam.second = V;
                if (startIdx == 0xffffffff || nextEndIdx == 0xffffffff || endParam.second < startParam.second) {
                    break;
                }
                // if end and new start do not intersect same segment in params
                if (startIdx != endIdx) {
                    auto dir = startIdx > endIdx ? 1 : -1;
                    auto idx = endIdx;
                    // idx is index of segment first point, so when we move forward we need to start from second point
                    if (dir > 0) idx++;
                    while ((dir == -1 && idx != startIdx) || (dir == 1 && idx <= startIdx)) {
                        XMStoreFloat3(&val, distant->value({midRing[idx].first, midRing[idx].second}));
                        handlePath.emplace_back(val);
                        idx += dir;
                    }
                }
            }
            endIdx = nextEndIdx;
            sign = -sign;
        }
    };

    auto pathsToSide2 = [stepLength, &distant, &outline, &midRing, V](vector<XMFLOAT3> &handlePath,
                                                                      const vector<pair<float, float>> &sideRestriction,
                                                                      pair<float, float> startParam,
                                                                      pair<float, float> endParam,
                                                                      size_t endIdx, float sign, float pathStep,
                                                                      bool mod, int loopCut) {

        size_t startIdx, nextEndIdx;
        size_t restrictionEndIdx, restrictionStartIdx;
        pair<float, float> restrictionStart, restrictionEnd;
        float endParamU = NAN;
        bool endOnRestriction = false;
        for (int i = 0; i < loopCut; ++i) {
            auto param = startParam;

            // find intersection with side restriction
            tie(restrictionStart, restrictionStartIdx) = findIntersection(sideRestriction, startParam, endParam, mod);
            bool hasRestriction = false;
            if (restrictionStartIdx != 0xffffffff) {
                // intersection exists
                tie(restrictionEnd, restrictionEndIdx) = findIntersectionEnd(sideRestriction, startParam, endParam,
                                                                             mod);

                // swap ends if needed
                if (abs(startParam.second - restrictionEnd.second) < abs(startParam.second - restrictionStart.second)) {
                    std::swap(restrictionStart, restrictionEnd);
                    std::swap(restrictionStartIdx, restrictionEndIdx);
                }

                hasRestriction = true;

                if (restrictionStart == restrictionEnd) {
                    endOnRestriction = true;
                }
            }

            XMFLOAT3 val;
            while (sign * param.second < sign * endParam.second) {
                // check if there will be collision
                if (hasRestriction && sign * param.second > sign * restrictionStart.second) {
                    if (endOnRestriction) break;
                    XMStoreFloat3(&val, distant->value({restrictionStart.first, restrictionStart.second}));
                    handlePath.emplace_back(val);
                    auto dir = restrictionEndIdx > restrictionStartIdx ? 1 : -1;
                    auto idx = restrictionStartIdx;
                    // idx is index of segment first point, so when we move forward we need to start from second point
                    if (dir > 0) idx++;
                    while ((dir == -1 && idx != restrictionEndIdx) || (dir == 1 && idx <= restrictionEndIdx)) {
                        XMStoreFloat3(&val, distant->value({sideRestriction[idx].first, sideRestriction[idx].second}));
                        handlePath.emplace_back(val);
                        idx += dir;
                    }
                    XMStoreFloat3(&val, distant->value({restrictionEnd.first, restrictionEnd.second}));
                    handlePath.emplace_back(val);
                    param.second = restrictionEnd.second + sign * stepLength;
                    hasRestriction = false;
                }

                XMStoreFloat3(&val, distant->value({param.first, param.second}));
                handlePath.emplace_back(val);
                param.second += sign * stepLength;
            }
            XMStoreFloat3(&val, distant->value({endParam.first, endParam.second}));
            handlePath.emplace_back(val);

            if (i == loopCut - 1) {
                endParamU = endParam.first + pathStep;
                break;
            }

            auto paramU = endParam.first + pathStep;
            if (paramU > 6.f) paramU -= 6.f;
            // use different params to get start and end
            if (i % 2 == 0) {
                tie(startParam, startIdx) = findIntersection(outline, {paramU, -20}, {paramU, 20}, mod);
                tie(endParam, nextEndIdx) = findIntersection(midRing, {paramU, -20}, {paramU, 20}, mod);
                endParam.second = V;
                if (startIdx == 0xffffffff || nextEndIdx == 0xffffffff) {
                    break;
                }
                // if end and new start do not intersect same segment in params
                if (startIdx != endIdx) {
                    auto dir = startIdx > endIdx ? 1 : -1;
                    auto idx = endIdx;
                    // idx is index of segment first point, so when we move forward we need to start from second point
                    if (dir > 0) idx++;
                    while ((dir == -1 && idx != startIdx) || (dir == 1 && idx <= startIdx)) {
                        XMStoreFloat3(&val, distant->value({outline[idx].first, outline[idx].second}));
                        handlePath.emplace_back(val);
                        idx += dir;
                    }
                }

                if (startIdx == 0xffffffff) break;
            } else {
                tie(startParam, startIdx) = findIntersection(midRing, {paramU, -20}, {paramU, 20}, mod);
                tie(endParam, nextEndIdx) = findIntersection(outline, {paramU, -20}, {paramU, 20}, mod);
                startParam.second = V;
                if (startIdx == 0xffffffff || nextEndIdx == 0xffffffff) {
                    break;
                }
                // if end and new start do not intersect same segment in params
                if (startIdx != endIdx) {
                    auto dir = startIdx > endIdx ? 1 : -1;
                    auto idx = endIdx;
                    // idx is index of segment first point, so when we move forward we need to start from second point
                    if (dir > 0) idx++;
                    while ((dir == -1 && idx != startIdx) || (dir == 1 && idx <= startIdx)) {
                        XMStoreFloat3(&val, distant->value({midRing[idx].first, midRing[idx].second}));
                        handlePath.emplace_back(val);
                        idx += dir;
                    }
                }
            }
            endIdx = nextEndIdx;
            sign = -sign;
        }

//         handle case where line would start in intersection
        if (!isnan(endParamU)) {
            auto paramU = endParamU;
            tie(restrictionStart, restrictionStartIdx) = findIntersectionEnd(sideRestriction, {paramU, -20},
                                                                             {paramU, 20}, mod);
            bool hasRestriction = false;
            if (restrictionStartIdx != 0xffffffff) {
                // intersection exists
                sign = -1.f;
                XMFLOAT3 val;
                auto param = restrictionStart;
                XMStoreFloat3(&val, distant->value({param.first, param.second}));

                auto height = handlePath.back().y + 1.5f;
                handlePath.emplace_back(handlePath.back().x, height, handlePath.back().z);
                handlePath.emplace_back(val.x, height, val.z);

                handlePath.emplace_back(val);
                param.second += sign * stepLength;

                tie(endParam, endIdx) = findIntersection(midRing, {paramU, -20}, {paramU, 20}, mod);
                endParam.second = V;
                while (sign * param.second < sign * endParam.second) {
                    XMStoreFloat3(&val, distant->value({param.first, param.second}));
                    handlePath.emplace_back(val);
                    param.second += sign * stepLength;
                }
                XMStoreFloat3(&val, distant->value({endParam.first, endParam.second}));
                handlePath.emplace_back(val);
            }
        }
    };

    vector<XMFLOAT3> mainPath;
    auto sign = 1.f;
    auto pathStep = 0.0375f;
    auto startParam = midRing[midRing.size() / 2];
    auto [endParam, endIdx] = findIntersection(outline, startParam, {startParam.first, 20});
    pathsToSide(mainPath, bottomRing, topRing, startParam, endParam, endIdx, sign, pathStep, true);

    auto &lastPoint = mainPath.back();
    auto height = lastPoint.y + 2.5f;
    mainPath.emplace_back(lastPoint.x, height, lastPoint.z);

    pathStep = -pathStep;
    auto paramU = startParam.first + pathStep;
    tie(startParam, endIdx) = findIntersection(midRing, {paramU, -20}, {paramU, 20});
    startParam.second = V;
    tie(endParam, endIdx) = findIntersection(outline, startParam, {startParam.first, 20});

    auto startPoint = distant->value({startParam.first, startParam.second});
    mainPath.emplace_back(startPoint.m128_f32[0], height, startPoint.m128_f32[2]);
    pathsToSide2(mainPath, dziubekRing, startParam, endParam, endIdx, sign, pathStep, false, 35);

    return mainPath;
}

vector<XMFLOAT3>
PathsCreatorHelper::createMainTopContour(const vector<XMFLOAT3> &outline, const vector<XMFLOAT3> &mainRing) {
    vector<XMFLOAT3> mainTopContour;
    mainTopContour.insert(mainTopContour.end(), mainRing.begin(), mainRing.end());
    mainTopContour.insert(mainTopContour.end(), outline.begin() + 1, outline.end());

    return mainTopContour;
}

vector<XMFLOAT3>
PathsCreatorHelper::createMainTopPath(const vector<pair<float, float>> &outline,
                                      const vector<pair<float, float>> &mainRing,
                                      const shared_ptr<ParametricObject<2>> &distant) {
    const float V = 2.99999f;
    auto stepLength = 0.5f;
    auto pathsToSide = [stepLength, &distant, &outline, &mainRing, V](vector<XMFLOAT3> &handlePath,
                                                                      pair<float, float> startParam,
                                                                      pair<float, float> endParam,
                                                                      size_t endIdx, float sign, float pathStep,
                                                                      bool mod) {
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
            if (paramU > 6.f) paramU -= 6.f;
            // use different params to get start and end
            if (i % 2 == 0) {
                tie(startParam, startIdx) = findIntersection(outline, {paramU, -10}, {paramU, 10}, mod);
                tie(endParam, nextEndIdx) = findIntersection(mainRing, {paramU, -10}, {paramU, 10}, mod);
                endParam.second = V;
                if (startIdx == 0xffffffff || nextEndIdx == 0xffffffff) {
                    break;
                }
                // if end and new start do not intersect same segment in params
                if (startIdx != endIdx) {
                    auto dir = startIdx > endIdx ? 1 : -1;
                    auto idx = endIdx;
                    // idx is index of segment first point, so when we move forward we need to start from second point
                    if (dir > 0) idx++;
                    while ((dir == -1 && idx != startIdx) || (dir == 1 && idx <= startIdx)) {
                        XMStoreFloat3(&val, distant->value({outline[idx].first, outline[idx].second}));
                        handlePath.emplace_back(val);
                        idx += dir;
                    }
                }

                if (startIdx == 0xffffffff) break;
            } else {
                tie(startParam, startIdx) = findIntersection(mainRing, {paramU, -10}, {paramU, 10}, mod);
                startParam.second = V;
                tie(endParam, nextEndIdx) = findIntersection(outline, {paramU, -10}, {paramU, 10}, mod);
                if (startIdx == 0xffffffff || nextEndIdx == 0xffffffff) {
                    break;
                }
                // if end and new start do not intersect same segment in params
                if (startIdx != endIdx) {
                    auto dir = startIdx > endIdx ? 1 : -1;
                    auto idx = endIdx;
                    // idx is index of segment first point, so when we move forward we need to start from second point
                    if (dir > 0) idx++;
                    while ((dir == -1 && idx != startIdx) || (dir == 1 && idx <= startIdx)) {
                        XMStoreFloat3(&val, distant->value({mainRing[idx].first, mainRing[idx].second}));
                        handlePath.emplace_back(val);
                        idx += dir;
                    }
                }
            }
            endIdx = nextEndIdx;
            sign = -sign;
        }
    };

    vector<XMFLOAT3> mainTopPath;
    auto sign = -1.f;
    auto pathStep = 0.0375f;
    auto startParam = mainRing[mainRing.size() / 2];
    auto [endParam, endIdx] = findIntersection(outline, startParam, {startParam.first, -10}, true);
    pathsToSide(mainTopPath, startParam, endParam, endIdx, sign, pathStep, true);

    auto &lastPoint = mainTopPath.back();
    auto height = lastPoint.y + 2.5f;
    mainTopPath.emplace_back(lastPoint.x, height, lastPoint.z);

    pathStep = -pathStep;
    auto paramU = startParam.first + pathStep;
    tie(startParam, endIdx) = findIntersection(mainRing, {paramU, -10}, {paramU, 10});
    startParam.second = V;
    tie(endParam, endIdx) = findIntersection(outline, startParam, {startParam.first, -10});

    auto startPoint = distant->value({startParam.first, startParam.second});
    mainTopPath.emplace_back(startPoint.m128_f32[0], height, startPoint.m128_f32[2]);
    pathsToSide(mainTopPath, startParam, endParam, endIdx, sign, pathStep, false);

    return mainTopPath;
}

vector<XMFLOAT3>
PathsCreatorHelper::createHolePath(const vector<XMFLOAT3> &outline, float distance) {
    const float xMove = 0.075f;
    const float xStart = 3.25f;

    float x = xStart;
    float sign = 1;
    vector<XMFLOAT3> holePath;
    for(int i = 0; i < 9; i++) {
        auto [startIdx, startPoint] = findIntersection(outline.begin(), {x, distance, -sign * 100}, {x, distance, 0.5f});
        auto [endIdx, endPoint] = findIntersection(outline.begin(), {x, distance, sign * 100}, {x, distance, 0.5f});

        holePath.push_back(startPoint);
        holePath.push_back(endPoint);

        x += xMove;
        sign = -sign;
    }

    return holePath;
}