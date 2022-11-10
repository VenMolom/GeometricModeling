//
// Created by Molom on 14/10/2022.
//

#include "CNCRouter.h"
#include "Utils/utils3D.h"
#include <cmath>

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

    textureDisk = Mesh::disk(0.5f, 128);
    textureSquare = Mesh::square(1.f);
    textureDome = Mesh::dome(0.5f, 128, 128, -1.f, 0.5f);
    textureHalfCylinder = Mesh::halfCylinder(0.5f, 1.f, 1, 256);
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
        _progress = 100;
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

            if (_size.z - nextTarget.z > _maxDepth) {
                showErrorAndFinish("Tool will exceed max depth");
                return;
            }

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
            currentLine++;
        }

        if (!drawPaths.vertices().empty()) {
            tool.setPosition(drawPaths.vertices().back().position);
        }
        drawPaths.update();
    } else {
        for (int i = 0; i < PATHS_PER_FRAME_SKIP && drawPaths.vertices().size() >= 2; ++i) {
            auto nextTarget = (drawPaths.vertices().end() - 2)->position;

            if (_size.z - nextTarget.z > _maxDepth) {
                showErrorAndFinish("Tool will exceed max depth");
                return;
            }

            toCarve.emplace_back(currentPosition, nextTarget);
            drawPaths.vertices().pop_back();
            currentPosition = nextTarget;
            currentLine++;
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
    currentLine = 2;
    _state = RouterState::Started;
    tool.setPosition(drawPaths.vertices().back().position);
}

void CNCRouter::skip() {
    fresh = false;
    if (_state != RouterState::Started) currentLine = 2;
    _state = RouterState::Skipped;
}

void CNCRouter::reset() {
    fresh = true;
    _state = routerPath.moves.empty() ? RouterState::Created : RouterState::FirstPathLoaded;
    _progress = 0;
    currentLine = 2;
    tool.setPosition(NEUTRAL_TOOL_POSITION);
    if (!routerPath.moves.empty()) {
        fillDrawPaths();
    }
    clearDepth(DxDevice::Instance());
    clearErrorMap(DxDevice::Instance());
}

void CNCRouter::stop() {
    _state = RouterState::Finished;
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

    if (!routerPath.moves.empty()) {
        tool.setPosition(routerPath.moves[0].target);
    }
}

void CNCRouter::carvePaths(vector<pair<XMFLOAT3, XMFLOAT3>> paths, Renderer &renderer) {
    if (paths.empty()) return;

    vector<pair<Renderable *, XMMATRIX>> toRender;

    auto scaleMtx = XMLoadFloat4x4(&toolScale), toTexMtx = XMLoadFloat4x4(&pathToTexture);
    auto scaleYZ = tool.size() / 10.f;
    auto moveZ = 0.f;
    bool downMove = true;
    Renderable *disk, *square;
    if (tool.endType() == CNCType::Flat) {
        disk = &textureDisk;
        square = &textureSquare;
    } else {
        disk = &textureDome;
        square = &textureHalfCylinder;
        moveZ = scaleYZ / 2.f;
    }

    for (auto &[start, end]: paths) {
        auto startV = XMLoadFloat3(&start), endV = XMLoadFloat3(&end);

        auto diffV = XMVectorSubtract(endV, startV);
        auto mid = XMVectorLerp(startV, endV, 0.5f);
        mid.m128_f32[2] += moveZ;
        auto length = XMVector3Length(diffV).m128_f32[0];
        auto len2 = XMVector2Length(diffV).m128_f32[0];
        auto rotZ = atan2f(diffV.m128_f32[1], diffV.m128_f32[0]);
        auto rotY = atan2f(diffV.m128_f32[2], len2);

        auto rotMtx = XMMatrixRotationY(-rotY) * XMMatrixRotationZ(rotZ);
        auto scaleXMtx = XMMatrixScaling(length, scaleYZ, scaleYZ);
        auto moveMtx = XMMatrixTranslationFromVector(mid);
        downMove = downMove && diffV.m128_f32[2] != 0.f;

        toRender.emplace_back(disk, scaleMtx * XMMatrixTranslationFromVector(startV) * toTexMtx);
//        toRender.emplace_back(disk, scaleMtx * XMMatrixTranslationFromVector(endV) * toTexMtx);
        toRender.emplace_back(square, scaleXMtx * rotMtx * moveMtx * toTexMtx);
    }

    renderer.drawToTexture(*this, toRender, downMove);

    auto &device = DxDevice::Instance();
    copyErrorMap(device);
    checkForErrors(device);

    clearErrorMap(device);
    copyDepth(device);
}

void CNCRouter::calculatePathToTexture() {
    XMStoreFloat4x4(&pathToTexture, XMMatrixOrthographicLH(_size.x, _size.y, 0, _size.z));
}

void CNCRouter::generateBlock() {
    verticesShaded.clear();
    indices.clear();

    auto pointsX = _pointsDensity.first >> 4;
    auto pointsY = _pointsDensity.second >> 4;

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
    // Height map and prev
    Texture2DDescription texd;
    texd.Width = _pointsDensity.first;
    texd.Height = _pointsDensity.second;
    texd.MipLevels = 1;
    texd.Format = DXGI_FORMAT_R32_TYPELESS;
    texd.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    auto heightTexture = device.CreateTexture(texd);
    auto prevHeightTexture = device.CreateTexture(texd);

    DepthStencilViewDescription dvd;
    dvd.Format = DXGI_FORMAT_D32_FLOAT;
    _depth = device.CreateDepthStencilView(heightTexture, dvd);

    ShaderResourceViewDescription srvd;
    srvd.Format = DXGI_FORMAT_R32_FLOAT;
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvd.Texture2D.MipLevels = 1;
    srvd.Texture2D.MostDetailedMip = 0;
    _depthTexture = device.CreateShaderResourceView(heightTexture, srvd);
    _prevDepthTexture = device.CreateShaderResourceView(prevHeightTexture, srvd);

    // Normal map
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

    // Error map
    // staging
    Texture1DDescription texd1;
    texd1.Width = 2U;
    texd1.MipLevels = 1;
    texd1.Format = DXGI_FORMAT_R32_UINT;
    texd1.Usage = D3D11_USAGE_STAGING;
    texd1.BindFlags = 0;
    texd1.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    _errorStaging = device.CreateTexture1D(texd1);

    // unordered
    texd1.Usage = D3D11_USAGE_DEFAULT;
    texd1.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
    texd1.CPUAccessFlags = 0;
    auto errorTexture = device.CreateTexture1D(texd1);

    uavd.Format = texd1.Format;
    uavd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
    uavd.Texture1D.MipSlice = 0;
    _errorUnordered = device.CreateUnorderedAccessView(errorTexture, uavd);

    clearDepth(device);
    clearErrorMap(device);
}

void CNCRouter::clearDepth(const DxDevice &device) {
    static const float clearColor[] = {0.5f, 0.5f, 1.f, 1.f};
    device.context()->ClearDepthStencilView(_depth.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
    device.context()->ClearUnorderedAccessViewFloat(_normalUnordered.get(), clearColor);
    copyDepth(device);
}

void CNCRouter::clearErrorMap(const DxDevice &device) {
    static const uint clearColor[] = {0, 0};
    device.context()->ClearUnorderedAccessViewUint(_errorUnordered.get(), clearColor);
    copyErrorMap(device);
}

void CNCRouter::copyDepth(const DxDevice &device) {
    ID3D11Resource *sourceRes = nullptr, *targetRes = nullptr;
    _depthTexture->GetResource(&sourceRes);
    _prevDepthTexture->GetResource(&targetRes);
    device.context()->CopyResource(targetRes, sourceRes);
}

void CNCRouter::copyErrorMap(const DxDevice &device) {
    ID3D11Resource *sourceRes = nullptr;
    _errorUnordered->GetResource(&sourceRes);
    device.context()->CopyResource(_errorStaging.get(), sourceRes);
}

void CNCRouter::checkForErrors(const DxDevice &device) {
    D3D11_MAPPED_SUBRESOURCE res;

    auto hr = device.context()->Map(_errorStaging.get(), 0, D3D11_MAP_READ, 0, &res);
    if (FAILED(hr))
        return;

    uint *data = reinterpret_cast<uint *>(res.pData);

    bool tooBigChange = data[1] > 0;
    bool flatStraightDown = data[0] > 0;

    device.context()->Unmap(_errorStaging.get(), 0);

    if (tooBigChange) {
        showErrorAndFinish("Max depth exceeded");
    }
    if (flatStraightDown) {
        showErrorAndFinish("Flat tool moving down into material");
    }
}

void CNCRouter::showErrorAndFinish(const QString &text) {
    errorBox = new QMessageBox(nullptr);
    errorBox->setIcon(QMessageBox::Warning);
    errorBox->setWindowTitle("Error in path");
    errorBox->setText(QString("Error in move ")
                            .append(QString::fromStdString(std::to_string(currentLine)))
                            .append(": ")
                            .append(text));
    QPushButton *btnCancel = errorBox->addButton("Ok", QMessageBox::RejectRole);
    errorBox->setAttribute(Qt::WA_DeleteOnClose); // delete pointer after close
    errorBox->setModal(false);
    errorBox->show();

    _state = Finished;
    _progress = 100;
}