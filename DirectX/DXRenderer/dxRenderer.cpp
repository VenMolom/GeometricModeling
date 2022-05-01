#include "dxRenderer.h"
#include "DirectX/DXDevice/exceptions.h"
#include "Objects/object.h"
#include "Objects/Curve/brezierCurve.h"
#include "Objects/Curve/interpolationCurveC2.h"

using namespace mini;
using namespace std;
using namespace DirectX;

DxRenderer::DxRenderer(QWidget *parent) : m_device(this),
                                          inputHandler() {
    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_NativeWindow, true);

    init3D3();
}

void DxRenderer::renderScene() {
    m_device.context()->ClearRenderTargetView(m_backBuffer.get(), CLEAR_COLOR);

    m_device.context()->ClearDepthStencilView(
            m_depthBuffer.get(),
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    if (stereoscopic && scene) {
        renderStereoscopic();
        return;
    }

    updateCameraCB();

    if (scene) {
        scene->draw(*this);
    }
}

void DxRenderer::renderStereoscopic() {
    auto projections = scene->camera()->stereoscopicProjectionMatrix();
    auto views = scene->camera()->stereoscopicViewMatrix();
    clearColor = (float*) STEREO_CLEAR_COLOR;

    // render left eye
    renderEye(get<0>(projections), get<0>(views), m_stereoscopicLeftTarget);
    m_device.context()->ClearDepthStencilView(m_depthBuffer.get(),
                                              D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // render right eye
    renderEye(get<1>(projections), get<1>(views), m_stereoscopicRightTarget);
    m_device.context()->ClearDepthStencilView(m_depthBuffer.get(),
                                              D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    auto backBuffer = m_backBuffer.get();
    m_device.context()->OMSetRenderTargets(1, &backBuffer, m_depthBuffer.get());

    m_device.context()->IASetInputLayout(m_stereoLayout.get());
    m_device.context()->VSSetShader(m_vertexStereoShader.get(), nullptr, 0);
    m_device.context()->PSSetShader(m_pixelStereoShader.get(), nullptr, 0);
    setTextures({m_stereoscopicLeftTexture.get(), m_stereoscopicRightTexture.get()}, m_sampler);

    m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ID3D11Buffer *vb = m_ndcQuad.get();
    unsigned int stride = sizeof(VertexPositionTex);
    unsigned int offset = 0;
    m_device.context()->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
    m_device.context()->Draw(6, 0);

    m_device.context()->IASetInputLayout(m_layout.get());
    m_device.context()->VSSetShader(m_vertexShader.get(), nullptr, 0);
    m_device.context()->PSSetShader(m_pixelShader.get(), nullptr, 0);
    clearColor = (float*) CLEAR_COLOR;

    updateBuffer(m_cbProj, scene->camera()->projectionMatrix());
}

void DxRenderer::renderEye(const XMMATRIX &projection, const XMMATRIX &view,
                           const dx_ptr<ID3D11RenderTargetView> &target) {
    updateBuffer(m_cbProj, projection);
    updateCameraCB(view);

    auto renderTarget = target.get();
    m_device.context()->OMSetRenderTargets(1, &renderTarget, m_depthBuffer.get());
    m_device.context()->ClearRenderTargetView(target.get(), STEREO_CLEAR_COLOR);

    scene->draw(*this);
}

void DxRenderer::setScene(shared_ptr<Scene> scenePtr) {
    scene = std::move(scenePtr);
    this->inputHandler.setScene(scene);

    updateBuffer(m_cbProj, scene->camera()->projectionMatrix());
    updateBuffer(m_cbFarPlane, XMFLOAT4{scene->camera()->nearZ(), scene->camera()->farZ(), 0, 0});
    updateCameraCB();
}

void DxRenderer::draw(const Object &object, XMFLOAT4 color) {
    updateBuffer(m_cbModel, object.modelMatrix());
    updateBuffer(m_cbColor, color);

    object.render(m_device.context());
}

void DxRenderer::draw(const BrezierCurve &curve, XMFLOAT4 color) {
    updateBuffer(m_cbModel, XMMatrixIdentity());
    updateBuffer(m_cbColor, color);

    drawCurve(curve, curve.patchesCount() - 1, curve.lastPatchSize());
}

void DxRenderer::draw(const InterpolationCurveC2 &curve, DirectX::XMFLOAT4 color) {
    updateBuffer(m_cbModel, XMMatrixIdentity());
    updateBuffer(m_cbColor, color);

    // lastPatchId = -1 to set same size for all patches
    drawCurve(curve, 0, 4);
}

void DxRenderer::drawCurve(const Curve &curve, int lastPatchId, int lastPatchSize) {
    m_device.context()->HSSetShader(m_hullBrezierShader.get(), nullptr, 0);
    m_device.context()->DSSetShader(m_domainBrezierShader.get(), nullptr, 0);

    auto viewport = scene->camera()->viewport();
    // viewport.xy, lastPatchID, lastPatchPoints
    XMINT4 tesselationAmount = {
            static_cast<int32_t>(viewport.width()),
            static_cast<int32_t>(viewport.height()),
            lastPatchId,
            lastPatchSize
    };
    updateBuffer(m_cbTesselation, tesselationAmount);

    curve.render(m_device.context());

    m_device.context()->HSSetShader(nullptr, nullptr, 0);
    m_device.context()->DSSetShader(nullptr, nullptr, 0);
}

void DxRenderer::draw(const Grid &grid, XMFLOAT4 color) {
    updateBuffer(m_cbModel, grid.modelMatrix());
    updateBuffer(m_cbColor,  *(float(*)[4])clearColor);

    m_device.context()->PSSetShader(m_pixelFadeShader.get(), nullptr, 0);
    m_device.context()->OMSetDepthStencilState(m_dssNoDepthWrite.get(), 0);

    grid.render(m_device.context());

    m_device.context()->OMSetDepthStencilState(nullptr, 0);
    m_device.context()->PSSetShader(m_pixelShader.get(), nullptr, 0);
}

void DxRenderer::draw(const Point &point, XMFLOAT4 color) {
    updateBuffer(m_cbModel, point.modelMatrix());
    updateBuffer(m_cbColor, color);

    m_device.context()->VSSetShader(m_vertexNoProjectionShader.get(), nullptr, 0);
    m_device.context()->GSSetShader(m_geometryPointShader.get(), nullptr, 0);

    point.render(m_device.context());

    m_device.context()->VSSetShader(m_vertexShader.get(), nullptr, 0);
    m_device.context()->GSSetShader(nullptr, nullptr, 0);
}

void DxRenderer::enableStereoscopy(bool enable) {
    stereoscopic = enable;
    if (!enable) {
        updateBuffer(m_cbProj, scene->camera()->projectionMatrix());
    }
}

void DxRenderer::setLeftEyeColor(XMFLOAT3 color) {
    leftEyeColor = XMFLOAT4(color.x, color.y, color.z, 1.f);

    XMFLOAT4 stereoColors[2]{leftEyeColor, rightEyeColor};
    updateBuffer(m_cbStereoColor, stereoColors);
}

void DxRenderer::setRightEyeColor(XMFLOAT3 color) {
    rightEyeColor = XMFLOAT4(color.x, color.y, color.z, 1.f);

    XMFLOAT4 stereoColors[2]{leftEyeColor, rightEyeColor};
    updateBuffer(m_cbStereoColor, stereoColors);
}

template<typename T>
void DxRenderer::updateBuffer(const dx_ptr<ID3D11Buffer> &buffer, const T &data) {
    D3D11_MAPPED_SUBRESOURCE res;
    auto hr = m_device.context()->Map(buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
    if (FAILED(hr))
        THROW_DX(hr);
    memcpy(res.pData, &data, sizeof(T));
    m_device.context()->Unmap(buffer.get(), 0);
}

void DxRenderer::setTextures(initializer_list<ID3D11ShaderResourceView *> resList,
                             const dx_ptr<ID3D11SamplerState> &sampler) {
    m_device.context()->PSSetShaderResources(0, resList.size(), resList.begin());
    auto s_ptr = sampler.get();
    m_device.context()->PSSetSamplers(0, 1, &s_ptr);
}

float DxRenderer::frameTime() {
    auto oldTicks = currentTicks;
    QueryPerformanceCounter(&currentTicks);
    auto countDelta = currentTicks.QuadPart - oldTicks.QuadPart;
    return (static_cast<float>(countDelta) / static_cast<float>(ticksPerSecond.QuadPart));
}

uint DxRenderer::frameRate() {
    frameCount++;
    if (currentTicks.QuadPart - lastFrameRateTick.QuadPart > ticksPerSecond.QuadPart) {
        lastFrameRateTick = currentTicks;
        lastFrameRate = frameCount;
        frameCount = 0;
    }

    return lastFrameRate;
}

void DxRenderer::updateCameraCB(XMMATRIX viewMatrix) {
    auto invView = XMMatrixInverse(nullptr, viewMatrix);
    XMFLOAT4X4 view[2];
    XMStoreFloat4x4(view, viewMatrix);
    XMStoreFloat4x4(view + 1, invView);
    updateBuffer(m_cbView, view);
}

QPaintEngine *DxRenderer::paintEngine() const {
    return nullptr;
}

#pragma region Event_Handlers

void DxRenderer::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);

    scene->camera()->resize(size());

    m_backBuffer.reset();
    m_depthBuffer.reset();

    m_device.swapChain()->ResizeBuffers(1, width(), height(), DXGI_FORMAT_R8G8B8A8_UNORM, 0);

    updateBuffer(m_cbProj, scene->camera()->projectionMatrix());
    setupViewport();
}

void DxRenderer::paintEvent(QPaintEvent *event) {
    auto deltaTime = frameTime();
    auto frames = frameRate();

    statusBar->showMessage(QString("Frame time: ")
                                   .append(QString::number(deltaTime * 1000, 'f', 3))
                                   .append(" ms")
                                   .append("\t\t Frame rate: ")
                                   .append(QString::number(frames))
                                   .append("\t\t Mode: ")
                                   .append(inputHandler.currentMode()));

    renderScene();
    m_device.swapChain()->Present(0, 0);
    update();
}

void DxRenderer::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);
    inputHandler.mousePressEvent(event);
}

void DxRenderer::mouseReleaseEvent(QMouseEvent *event) {
    QWidget::mouseReleaseEvent(event);
    inputHandler.mouseReleaseEvent(event);
}

void DxRenderer::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);
    inputHandler.mouseMoveEvent(event);
}

void DxRenderer::wheelEvent(QWheelEvent *event) {
    QWidget::wheelEvent(event);
    inputHandler.wheelEvent(event);
}

void DxRenderer::keyPressEvent(QKeyEvent *event) {
    QWidget::keyPressEvent(event);
    inputHandler.keyPressEvent(event);
}

void DxRenderer::keyReleaseEvent(QKeyEvent *event) {
    QWidget::keyReleaseEvent(event);
    inputHandler.keyReleaseEvent(event);
}

void DxRenderer::focusOutEvent(QFocusEvent *event) {
    QWidget::focusOutEvent(event);
    inputHandler.focusLost();
}

void DxRenderer::handleKeyEvent(QKeyEvent *event) {
    switch (event->type()) {
        case QEvent::KeyPress:
            keyPressEvent(event);
            break;
        case QEvent::KeyRelease:
            keyReleaseEvent(event);
            break;
    }
}

#pragma endregion Event_Handlers

#pragma region Init

void DxRenderer::init3D3() {
    setupViewport();

    const auto vsBytes = DxDevice::LoadByteCode(L"vs.cso");
    const auto vsBillboardBytes = DxDevice::LoadByteCode(L"vsBillboard.cso");
    const auto vsNoProjectionBytes = DxDevice::LoadByteCode(L"vsNoProjection.cso");
    const auto vsStereoBytes = DxDevice::LoadByteCode(L"vsStereo.cso");
    const auto hsBrezierBytes = DxDevice::LoadByteCode(L"hsBrezier.cso");
    const auto dsBrezierBytes = DxDevice::LoadByteCode(L"dsBrezier.cso");
    const auto gsPointBytes = DxDevice::LoadByteCode(L"gsPoint.cso");
    const auto psBytes = DxDevice::LoadByteCode(L"ps.cso");
    const auto psFadeBytes = DxDevice::LoadByteCode(L"psCameraFade.cso");
    const auto psStereoBytes = DxDevice::LoadByteCode(L"psStereo.cso");
    m_vertexShader = m_device.CreateVertexShader(vsBytes);
    m_vertexBillboardShader = m_device.CreateVertexShader(vsBillboardBytes);
    m_vertexNoProjectionShader = m_device.CreateVertexShader(vsNoProjectionBytes);
    m_vertexStereoShader = m_device.CreateVertexShader(vsStereoBytes);
    m_hullBrezierShader = m_device.CreateHullShader(hsBrezierBytes);
    m_domainBrezierShader = m_device.CreateDomainShader(dsBrezierBytes);
    m_geometryPointShader = m_device.CreateGeometryShader(gsPointBytes);
    m_pixelShader = m_device.CreatePixelShader(psBytes);
    m_pixelFadeShader = m_device.CreatePixelShader(psFadeBytes);
    m_pixelStereoShader = m_device.CreatePixelShader(psStereoBytes);

    m_device.context()->VSSetShader(m_vertexShader.get(), nullptr, 0);
    m_device.context()->PSSetShader(m_pixelShader.get(), nullptr, 0);

    vector<D3D11_INPUT_ELEMENT_DESC> elements{
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
                    D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
                                                            static_cast<UINT>(offsetof(VertexPositionColor, color)),
                    D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    m_layout = m_device.CreateInputLayout(elements, vsBytes);
    m_device.context()->IASetInputLayout(m_layout.get());

    elements = {
            {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,
                    D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
                                                         static_cast<UINT>(offsetof(VertexPositionTex, tex)),
                    D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    m_stereoLayout = m_device.CreateInputLayout(elements, vsStereoBytes);

    m_cbModel = m_device.CreateConstantBuffer<XMFLOAT4X4>();
    m_cbView = m_device.CreateConstantBuffer<XMFLOAT4X4, 2>();
    m_cbProj = m_device.CreateConstantBuffer<XMFLOAT4X4>();
    m_cbColor = m_device.CreateConstantBuffer<XMFLOAT4>();
    m_cbTesselation = m_device.CreateConstantBuffer<XMINT4>();
    m_cbFarPlane = m_device.CreateConstantBuffer<XMFLOAT4>();
    m_cbStereoColor = m_device.CreateConstantBuffer<XMFLOAT4, 2>();

    ID3D11Buffer *vsCbs[] = {m_cbModel.get(), m_cbView.get(), m_cbProj.get()};
    ID3D11Buffer *psCbs[] = {m_cbColor.get(), m_cbFarPlane.get(), m_cbStereoColor.get()};
    ID3D11Buffer *hsCbs[] = {m_cbTesselation.get()};
    ID3D11Buffer *gsCbs[] = {m_cbFarPlane.get(), m_cbProj.get()};
    m_device.context()->VSSetConstantBuffers(0, 3, vsCbs);
    m_device.context()->PSSetConstantBuffers(0, 3, psCbs);
    m_device.context()->HSSetConstantBuffers(0, 1, hsCbs);
    m_device.context()->GSSetConstantBuffers(0, 2, gsCbs);

    DepthStencilDescription dssDesc;
    dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    m_dssNoDepthWrite = m_device.CreateDepthStencilState(dssDesc);

    SamplerDescription samplerDesc;
    m_sampler = m_device.CreateSamplerState(samplerDesc);

    vector<VertexPositionTex> quad{{
                                           {{-1.f, -1.f}, {.0f, 1.f}},
                                           {{-1.f, 1.f}, {.0f, .0f}},
                                           {{1.f, -1.f}, {1.f, 1.f}},
                                           {{1.f, -1.f}, {1.f, 1.f}},
                                           {{-1.f, 1.f}, {.0f, .0f}},
                                           {{1.f, 1.f}, {1.f, .0f}}
                                   }
    };
    m_ndcQuad = m_device.CreateVertexBuffer(quad);

    QueryPerformanceFrequency(&ticksPerSecond);
    QueryPerformanceCounter(&currentTicks);
    QueryPerformanceCounter(&lastFrameRateTick);
}

void DxRenderer::setupViewport() {
    ID3D11Texture2D *temp;
    dx_ptr<ID3D11Texture2D> backTexture;
    m_device.swapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&temp));
    backTexture.reset(temp);

    m_backBuffer = m_device.CreateRenderTargetView(backTexture);

    SIZE wndSize{width(), height()};
    m_depthBuffer = m_device.CreateDepthStencilView(wndSize);
    auto backBuffer = m_backBuffer.get();
    m_device.context()->OMSetRenderTargets(1,
                                           &backBuffer, m_depthBuffer.get());
    Viewport viewport{wndSize};
    m_device.context()->RSSetViewports(1, &viewport);

    Texture2DDescription textureDesc(width(), height());
    textureDesc.MipLevels = 1;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    auto leftTexture = m_device.CreateTexture(textureDesc);
    auto rightTexture = m_device.CreateTexture(textureDesc);

    ShaderResourceViewDescription srvd;
    srvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvd.Texture2D.MipLevels = 1;
    srvd.Texture2D.MostDetailedMip = 0;
    m_stereoscopicLeftTexture = m_device.CreateShaderResourceView(leftTexture, srvd);
    m_stereoscopicRightTexture = m_device.CreateShaderResourceView(rightTexture, srvd);

    m_stereoscopicLeftTarget = m_device.CreateRenderTargetView(leftTexture);
    m_stereoscopicRightTarget = m_device.CreateRenderTargetView(rightTexture);
}

#pragma endregion Init