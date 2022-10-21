//
// Created by Molom on 14/10/2022.
//

#include "CNCRouter.h"
#include "Utils/utils3D.h"

using namespace std;
using namespace DirectX;

const XMFLOAT3 CNCRouter::NEUTRAL_TOOL_POSITION = {0.f, 0.f, 10.f};

CNCRouter::CNCRouter(uint id, XMFLOAT3 position)
        : Object(id, "Router", position, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST),
          tool(NEUTRAL_TOOL_POSITION),
          drawPaths() {
    convertToShaded();
    drawPaths.setPosition(position);
    drawPaths.setRotation(XMFLOAT3(-XM_PIDIV2, 0.f, 0.f));
    Object::setPosition(position);
    Object::setRotation(XMFLOAT3(-XM_PIDIV2, 0.f, 0.f));
    Object::setScale(_size);
    generateBlock();
    drawPaths.noDepth = true;

    textureDisk = Mesh::disk(0.5f, 32);
    textureSquare = Mesh::square(1.f);
    textureDome = Mesh::dome(0.5f, 32, 32, -1.f, 0.5f);
    textureHalfCylinder = Mesh::halfCylinder(0.5f, 1.f, 1, 64);
    calculatePathToTexture();

    auto size = tool.size() / 10.f;
    XMStoreFloat4x4(&toolScale, XMMatrixScaling(size, size, size));
}

Type CNCRouter::type() const {
    return CNCROUTER;
}

void CNCRouter::setPosition(DirectX::XMFLOAT3 position) {
    drawPaths.setPosition(position);
    Object::setPosition(position);
}

void CNCRouter::setSize(DirectX::XMFLOAT3 size) {
    _size = size;
    Object::setScale(size);
    calculatePathToTexture();
}

void CNCRouter::setPointsDensity(std::pair<int, int> density) {
    _pointsDensity = density;
    generateBlock();
    calculatePathToTexture();
}

void CNCRouter::setMaxDepth(float depth) {
    _maxDepth = depth;
}

void CNCRouter::setToolType(CNCType type) {
    tool.setType(type);
}

void CNCRouter::setToolSize(int size) {
    auto mSize = size / 10.f;
    XMStoreFloat4x4(&toolScale, XMMatrixScaling(mSize, mSize, mSize));
    tool.setSize(size);
}

void CNCRouter::setShowPaths(bool show) {
    _showPaths = show;
}

void CNCRouter::setSimulationSpeed(int speed) {
    _simulationSpeed = speed;
}

void CNCRouter::draw(Renderer &renderer, DrawType drawType) {
    renderer.draw(*this);
    tool.draw(renderer, XMLoadFloat4x4(&noScaleMatrix));

    if (_showPaths) {
        drawPaths.draw(renderer, drawType);
    }
}

void CNCRouter::update(Renderer &renderer, float frameTime) {
    if (_state != RouterState::Started && _state != RouterState::Skipped) return;

    if (drawPaths.vertices().size() < 2) {
        _state = RouterState::Finished;
        tool.setPosition(NEUTRAL_TOOL_POSITION);
        drawPaths.update();
        return;
    }

    auto currentPosition = drawPaths.vertices().back().position;
    vector<pair<XMFLOAT3, XMFLOAT3>> toCarve;
    if (_state == RouterState::Started) {

        auto distanceToTravel = _simulationSpeed * TOOL_SPEED * frameTime;

        while (distanceToTravel > 0.f && drawPaths.vertices().size() >= 2) {
            auto nextTarget = (drawPaths.vertices().end() - 2)->position;
            auto distanceToTarget = XMVector3Length(
                    XMVectorSubtract(XMLoadFloat3(&nextTarget), XMLoadFloat3(&currentPosition))).m128_f32[0];
            if (distanceToTravel < distanceToTarget) {
                Utils3D::storeFloat3Lerp(drawPaths.vertices().back().position, currentPosition, nextTarget,
                                         distanceToTravel / distanceToTarget);
                toCarve.emplace_back(currentPosition, drawPaths.vertices().back().position);
                break;
            }

            toCarve.emplace_back(currentPosition, nextTarget);
            drawPaths.vertices().pop_back();
            distanceToTravel -= distanceToTarget;
            currentPosition = nextTarget;
        }

        if (!drawPaths.vertices().empty()) {
            tool.setPosition(drawPaths.vertices().back().position);
        }
        drawPaths.update();
    } else {
        for (int i = 0; i < PATHS_PER_FRAME_SKIP && drawPaths.vertices().size() >= 2; ++i) {
            auto nextTarget = (drawPaths.vertices().end() - 2)->position;
            toCarve.emplace_back(currentPosition, nextTarget);
            drawPaths.vertices().pop_back();
            currentPosition = nextTarget;
        }
    }
    if (!toCarve.empty()) {
        carvePaths(toCarve, renderer);
    }

    _progress = 100 - static_cast<int>(std::floor(
            static_cast<float>(drawPaths.vertices().size() * 100) / static_cast<float>(routerPath.moves.size())));
}

void CNCRouter::loadPath(CNCPath &&path) {
    _filename = QString::fromStdString(path.filename);
    routerPath = path;

    tool.setSize(path.size);
    tool.setType(path.type);

    fillDrawPaths();
    _state = fresh ? RouterState::FirstPathLoaded : RouterState::NextPathLoaded;
    _progress = 0;
}

void CNCRouter::start() {
    fresh = false;
    _state = RouterState::Started;
    tool.setPosition(drawPaths.vertices().back().position);
}

void CNCRouter::skip() {
    fresh = false;
    _state = RouterState::Skipped;
}

void CNCRouter::reset() {
    fresh = true;
    _state = routerPath.moves.empty() ? RouterState::Created : RouterState::FirstPathLoaded;
    _progress = 0;
    if (!routerPath.moves.empty()) {
        fillDrawPaths();
    }
    clearDepth(DxDevice::Instance());
}

void CNCRouter::fillDrawPaths() {
    auto size = routerPath.moves.size();
    drawPaths.vertices().clear();
    drawPaths.vertices().resize(size);

    // lines are stored in reverse to optimise their deletion when traversing
    for (auto i = 0; i < size; ++i) {
        drawPaths.vertices()[i].position = routerPath.moves[size - i - 1].target;
    }
    drawPaths.update();
}

void CNCRouter::carvePaths(vector<pair<XMFLOAT3, XMFLOAT3>> paths, Renderer &renderer) {
    if (paths.empty()) return;

    vector<pair<Renderable *, XMMATRIX>> toRender;

    auto scaleMtx = XMLoadFloat4x4(&toolScale), toTexMtx = XMLoadFloat4x4(&pathToTexture);
    Renderable *disk, *square;
    if (tool.endType() == CNCType::Flat) {
        disk = &textureDisk;
        square = &textureSquare;
    } else {
        disk = &textureDome;
        square = &textureHalfCylinder;
    }

    for (auto &[start, end]: paths) {
        auto startV = XMLoadFloat3(&start), endV = XMLoadFloat3(&end);

        auto mid = XMVectorLerp(startV, endV, 0.5f);
        auto rotZ = atan2f(end.y - start.y, end.x - start.x);
        auto scaleX = XMVector3Length(XMVectorSubtract(endV, startV)).m128_f32[0];

        auto rotMtx = XMMatrixRotationZ(rotZ);
        auto scaleYZ = tool.size() / 10.f;
        auto scaleXMtx = XMMatrixScaling(scaleX, scaleYZ, scaleYZ);
        auto moveMtx = XMMatrixTranslationFromVector(mid);

        // TODO: add rotation from z difference

        toRender.emplace_back(disk, scaleMtx * XMMatrixTranslationFromVector(startV) * toTexMtx);
        toRender.emplace_back(disk, scaleMtx * XMMatrixTranslationFromVector(endV) * toTexMtx);
        toRender.emplace_back(square, scaleXMtx * rotMtx * moveMtx * toTexMtx);
    }

    renderer.drawToTexture(*this, toRender);
}

void CNCRouter::calculatePathToTexture() {
    XMStoreFloat4x4(&pathToTexture, XMMatrixOrthographicLH(_size.x, _size.y, 0, _size.z));
}

void CNCRouter::generateBlock() {
    verticesShaded.clear();
    indices.clear();

    auto pointsX = _pointsDensity.first >> 5;
    auto pointsY = _pointsDensity.second >> 5;

    auto deltaX = 1.f / (pointsX - 1);
    auto deltaY = 1.f / (pointsY - 1);

    generateFaceZ(false, 1.f, 1.f, deltaX, deltaY, pointsX, pointsY); // z+ facing
    generateFaceZ(true, 0.f, -1.f, deltaX, deltaY, pointsX, pointsY); // z- facing
    generateFaceX(true, 0.5f, 1.f, deltaY, pointsY); // x+ facing
    generateFaceX(false, -0.5f, -1.f, deltaY, pointsY); // x- facing
    generateFaceY(false, 0.5f, 1.f, deltaX, pointsX); // y+ facing
    generateFaceY(true, -0.5f, -1.f, deltaX, pointsX); // y- facing

    updateBuffers();
    createDepthAndTexture(DxDevice::Instance());
}

void CNCRouter::generateFaceZ(bool ccw, float z, float normalZ, float deltaX, float deltaY, int pointsX, int pointsY) {
    XMFLOAT3 pos = {-.5f, -.5f, z};
    XMFLOAT2 tex = {0.f, 1.f};
    auto idx = verticesShaded.size();
    for (int i = 0; i < pointsX; ++i) { // x points
        for (int j = 0; j < pointsY; ++j) { // y points
            auto tt = normalZ > 0 ? tex : XMFLOAT2{-1.f, -1.f};
            verticesShaded.push_back({pos, {0, 0, normalZ}, tt});
            pos.y += deltaY;
            tex.y -= deltaY;
        }
        pos.y = -.5f;
        pos.x += deltaX;
        tex.y = 1.f;
        tex.x += deltaX;
    }

    for (int i = 0; i < pointsX - 1; ++i) { // x points
        for (int j = 0; j < pointsY - 1; ++j) { // y points
            auto index = idx + i * pointsY + j;

            if (ccw) {
                indices.push_back(index);
                indices.push_back(index + pointsY);
                indices.push_back(index + 1);

                indices.push_back(index + pointsY + 1);
                indices.push_back(index + 1);
                indices.push_back(index + pointsY);
            } else {
                indices.push_back(index);
                indices.push_back(index + 1);
                indices.push_back(index + pointsY);

                indices.push_back(index + pointsY + 1);
                indices.push_back(index + pointsY);
                indices.push_back(index + 1);
            }
        }
    }
}

void CNCRouter::generateFaceX(bool ccw, float x, float normalX, float deltaY, int pointsY) {
    auto idx = verticesShaded.size();
    auto sign = ccw ? 1.f : 0.f;
    for (float y = -.5f; y <= .5f; y += deltaY) { // y points
        verticesShaded.push_back({{x,       y, 1.f},
                                  {normalX, 0, 0},
                                  {sign,    0.5f - y}});
        verticesShaded.push_back({{x,          y, 0.f},
                                  {normalX,    0, 0},
                                  {1.f - sign, 0.5f - y}});
    }

    for (int i = 0; i < pointsY - 1; ++i) { // y points
        auto index = idx + 2 * i;

        if (ccw) {
            indices.push_back(index + 1);
            indices.push_back(index);
            indices.push_back(index + 2);

            indices.push_back(index + 2);
            indices.push_back(index + 3);
            indices.push_back(index + 1);
        } else {
            indices.push_back(index);
            indices.push_back(index + 1);
            indices.push_back(index + 2);

            indices.push_back(index + 3);
            indices.push_back(index + 2);
            indices.push_back(index + 1);
        }
    }
}

void CNCRouter::generateFaceY(bool ccw, float y, float normalY, float deltaX, int pointsX) {
    auto idx = verticesShaded.size();
    auto sign = ccw ? 1.f : 0.f;
    for (float x = -.5f; x <= .5f; x += deltaX) { // y points
        verticesShaded.push_back({{x,        y,       1.f},
                                  {0,        normalY, 0},
                                  {x + 0.5f, sign}});
        verticesShaded.push_back({{x,        y,       0.f},
                                  {0,        normalY, 0},
                                  {x + 0.5f, 1.f - sign}});
    }

    for (int i = 0; i < pointsX - 1; ++i) { // y points
        auto index = idx + 2 * i;

        if (ccw) {
            indices.push_back(index + 1);
            indices.push_back(index);
            indices.push_back(index + 2);

            indices.push_back(index + 2);
            indices.push_back(index + 3);
            indices.push_back(index + 1);
        } else {
            indices.push_back(index);
            indices.push_back(index + 1);
            indices.push_back(index + 2);

            indices.push_back(index + 3);
            indices.push_back(index + 2);
            indices.push_back(index + 1);
        }
    }
}

void CNCRouter::createDepthAndTexture(const DxDevice &device) {
    Texture2DDescription texd;
    texd.Width = _pointsDensity.first;
    texd.Height = _pointsDensity.second;
    texd.MipLevels = 1;
    texd.Format = DXGI_FORMAT_R32_TYPELESS;
    texd.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    auto heightTexture = device.CreateTexture(texd);

    DepthStencilViewDescription dvd;
    dvd.Format = DXGI_FORMAT_D32_FLOAT;
    _depth = device.CreateDepthStencilView(heightTexture, dvd);

    ShaderResourceViewDescription srvd;
    srvd.Format = DXGI_FORMAT_R32_FLOAT;
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvd.Texture2D.MipLevels = 1;
    srvd.Texture2D.MostDetailedMip = 0;
    _texture = device.CreateShaderResourceView(heightTexture, srvd);

    texd.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    texd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    auto normalTexture = device.CreateTexture(texd);

    srvd.Format = texd.Format;
    _normal = device.CreateShaderResourceView(normalTexture, srvd);

    UnorderedAccessViewDescription uavd;
    uavd.Format = texd.Format;
    uavd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavd.Texture2D.MipSlice = 0;
    _normalUnordered = device.CreateUnorderedAccessView(normalTexture, uavd);

    clearDepth(device);
}

void CNCRouter::clearDepth(const DxDevice &device) {
    static const float clearColor[] = {0.5f, 0.5f, 1.f, 1.f};
    device.context()->ClearDepthStencilView(_depth.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
    device.context()->ClearUnorderedAccessViewFloat(_normalUnordered.get(), clearColor);
}