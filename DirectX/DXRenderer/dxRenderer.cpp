#include <DirectXMath.h>
#include "dxRenderer.h"

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

    ID3D11Buffer *vsCbs[] = {m_cbMVP.get()};
    ID3D11Buffer *psCbs[] = {m_cbColor.get(), m_cbFarPlane.get()};
    ID3D11Buffer *hsCbs[] = {m_cbTesselation.get()};
    m_device.context()->VSSetConstantBuffers(0, 1, vsCbs);
    m_device.context()->PSSetConstantBuffers(0, 2, psCbs);
    m_device.context()->HSSetConstantBuffers(0, 1, hsCbs);
    m_device.context()->IASetInputLayout(m_layout.get());

    if (scene) {
        scene->draw(*this);
    }
}

void DxRenderer::setScene(shared_ptr <Scene> scenePtr) {
    scene = std::move(scenePtr);
    this->inputHandler.setScene(scene);
}

void DxRenderer::draw(const vector<VertexPositionColor> &points, Topology topology,
                      const XMMATRIX &mvp, XMFLOAT4 colorOverride) {
    updateBuffer(m_cbMVP, mvp);
    updateBuffer(m_cbColor, colorOverride);

    // set vertex buffer
    m_vertexBuffer = m_device.CreateVertexBuffer(points);
    ID3D11Buffer *vbs[] = {m_vertexBuffer.get()};
    UINT strides[] = {sizeof(VertexPositionColor)};
    UINT offsets[] = {0};
    m_device.context()->IASetVertexBuffers(
            0, 1, vbs, strides, offsets);

    // draw topology
    m_device.context()->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY) topology);
    m_device.context()->Draw(points.size(), 0);
}

void DxRenderer::drawIndexed(const vector<VertexPositionColor> &vertices, const vector<Index> &indices,
                             Topology topology, const XMMATRIX &mvp, XMFLOAT4 colorOverride) {
    updateBuffer(m_cbMVP, mvp);
    updateBuffer(m_cbColor, colorOverride);

    // set vertex and index buffers
    m_vertexBuffer = m_device.CreateVertexBuffer(vertices);
    m_indexBuffer = m_device.CreateIndexBuffer(indices);
    ID3D11Buffer *vbs[] = {m_vertexBuffer.get()};
    UINT strides[] = {sizeof(VertexPositionColor)};
    UINT offsets[] = {0};
    m_device.context()->IASetVertexBuffers(
            0, 1, vbs, strides, offsets);
    m_device.context()->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);

    // draw topology
    m_device.context()->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY) topology);
    m_device.context()->DrawIndexed(indices.size(), 0, 0);
}

void DxRenderer::drawCurve4(const vector<VertexPositionColor> &controlPoints,
                            const vector<Index> &indices, int lastPatchSize,
                            XMVECTOR min, XMVECTOR max,
                            const XMMATRIX &mvp, XMFLOAT4 colorOverride) {
    updateBuffer(m_cbMVP, mvp);
    updateBuffer(m_cbColor, colorOverride);

    // set vertex buffer
    m_vertexBuffer = m_device.CreateVertexBuffer(controlPoints);
    m_indexBuffer = m_device.CreateIndexBuffer(indices);
    ID3D11Buffer *vbs[] = {m_vertexBuffer.get()};
    UINT strides[] = {sizeof(VertexPositionColor)};
    UINT offsets[] = {0};
    m_device.context()->IASetVertexBuffers(0, 1, vbs, strides, offsets);
    m_device.context()->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);

    m_device.context()->HSSetShader(m_hullShader.get(), nullptr, 0);
    m_device.context()->DSSetShader(m_domainShader.get(), nullptr, 0);

    XMFLOAT2 vmin, vmax;
    auto viewport = scene->camera().viewport();
    XMStoreFloat2(&vmin, XMVector3Project(min, 0, 0,
                                          viewport.width(), viewport.height(),
                                          scene->camera().nearZ(), scene->camera().farZ(),
                                          mvp, XMMatrixIdentity(), XMMatrixIdentity()));
    XMStoreFloat2(&vmax, XMVector3Project(max, 0, 0,
                                          viewport.width(), viewport.height(),
                                          scene->camera().nearZ(), scene->camera().farZ(),
                                          mvp, XMMatrixIdentity(), XMMatrixIdentity()));

    // tesselationAmount, lastPatchID, lastPatchPoints
    XMINT4 tesselationAmount = {
            clamp(static_cast<int>(ceil(fmax(abs(vmax.x - vmin.x), abs(vmax.y - vmin.y)) / 64.0f)), 1, 64),
            static_cast<int>((indices.size() - 1) / 4),
            lastPatchSize, 0};
    int indexCount = indices.size();
    if (lastPatchSize == 0) {
        tesselationAmount.z = 4;
        tesselationAmount.y--;
        indexCount -= 4;
    }
    updateBuffer(m_cbTesselation, tesselationAmount);

    // draw patches
    m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
    m_device.context()->DrawIndexed(indexCount, 0, 0);

    m_device.context()->HSSetShader(nullptr, nullptr, 0);
    m_device.context()->DSSetShader(nullptr, nullptr, 0);
}

void DxRenderer::drawGrid(const vector<VertexPositionColor> &points, const DirectX::XMMATRIX &mvp) {
    updateBuffer(m_cbMVP, mvp);
    updateBuffer(m_cbColor, CLEAR_COLOR);
    updateBuffer(m_cbFarPlane, XMFLOAT4{scene->camera().farZ(), 0, 0, 0});

    m_device.context()->PSSetShader(m_pixelFadeShader.get(), nullptr, 0);

    // set vertex buffer
    m_vertexBuffer = m_device.CreateVertexBuffer(points);
    ID3D11Buffer *vbs[] = {m_vertexBuffer.get()};
    UINT strides[] = {sizeof(VertexPositionColor)};
    UINT offsets[] = {0};
    m_device.context()->IASetVertexBuffers(
            0, 1, vbs, strides, offsets);

    // draw topology
    m_device.context()->OMSetDepthStencilState(m_dssNoDepthWrite.get(), 0);
    m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    m_device.context()->Draw(points.size(), 0);

    m_device.context()->PSSetShader(m_pixelShader.get(), nullptr, 0);
    m_device.context()->OMSetDepthStencilState(nullptr, 0);
}

template<typename T>
void DxRenderer::updateBuffer(const dx_ptr<ID3D11Buffer> &buffer, const T &data) {
    D3D11_MAPPED_SUBRESOURCE res;
    m_device.context()->Map(buffer.get(), 0,
                            D3D11_MAP_WRITE_DISCARD, 0, &res);
    memcpy(res.pData, &data, sizeof(T));
    m_device.context()->Unmap(buffer.get(), 0);
}

float DxRenderer::frameTime() {
    auto oldTicks = currentTicks;
    QueryPerformanceCounter(&currentTicks);
    auto countDelta = currentTicks.QuadPart - oldTicks.QuadPart;
    return (static_cast<float>(countDelta) / static_cast<float>(ticksPerSecond.QuadPart));
}

QPaintEngine *DxRenderer::paintEngine() const {
    return nullptr;
}

#pragma region Event_Handlers

void DxRenderer::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);

    scene->camera().resize(size());

    m_backBuffer.reset();
    m_depthBuffer.reset();

    m_device.swapChain()->ResizeBuffers(1, width(), height(), DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    setupViewport();
}

void DxRenderer::paintEvent(QPaintEvent *event) {
    auto deltaTime = frameTime();

    statusBar->showMessage(QString("Frame time: ")
                                   .append(QString::number(deltaTime * 1000, 'f', 3))
                                   .append(" ms"));

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
    const auto hsBytes = DxDevice::LoadByteCode(L"hs.cso");
    const auto dsBytes = DxDevice::LoadByteCode(L"ds.cso");
    const auto psBytes = DxDevice::LoadByteCode(L"ps.cso");
    const auto psFadeBytes = DxDevice::LoadByteCode(L"psCameraFade.cso");
    m_vertexShader = m_device.CreateVertexShader(vsBytes);
    m_hullShader = m_device.CreateHullShader(hsBytes);
    m_domainShader = m_device.CreateDomainShader(dsBytes);
    m_pixelShader = m_device.CreatePixelShader(psBytes);
    m_pixelFadeShader = m_device.CreatePixelShader(psFadeBytes);

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

    m_cbMVP = m_device.CreateConstantBuffer<XMFLOAT4X4>();
    m_cbColor = m_device.CreateConstantBuffer<XMFLOAT4>();
    m_cbTesselation = m_device.CreateConstantBuffer<XMINT4>();
    m_cbFarPlane = m_device.CreateConstantBuffer<XMFLOAT4>();

    DepthStencilDescription dssDesc;
    dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    m_dssNoDepthWrite = m_device.CreateDepthStencilState(dssDesc);

    QueryPerformanceFrequency(&ticksPerSecond);
    QueryPerformanceCounter(&currentTicks);
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
}

#pragma endregion Init