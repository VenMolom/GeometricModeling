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
    float clearColor[] = CLEAR_COLOR;
    m_device.context()->ClearRenderTargetView(m_backBuffer.get(), clearColor);

    m_device.context()->ClearDepthStencilView(
            m_depthBuffer.get(),
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    ID3D11Buffer *cbs[] = {m_cbMVP.get()};
    m_device.context()->VSSetConstantBuffers(0, 1, cbs);
    m_device.context()->IASetInputLayout(m_layout.get());

    if (scene) {
        scene->draw(*this);
    }
}

void DxRenderer::setScene(std::shared_ptr<Scene> scenePtr) {
    scene = std::move(scenePtr);
    this->inputHandler.setScene(scene);
}

void DxRenderer::drawLines(const vector<VertexPositionColor> &vertices,
                           const vector<Index> &indices,
                           const DirectX::XMMATRIX &mvp) {
    // map MVP matrix to shader buffer
    D3D11_MAPPED_SUBRESOURCE res;
    m_device.context()->Map(m_cbMVP.get(), 0,
                            D3D11_MAP_WRITE_DISCARD, 0, &res);
    memcpy(res.pData, &mvp, sizeof(XMMATRIX));
    m_device.context()->Unmap(m_cbMVP.get(), 0);

    // set vertex and index buffers
    m_vertexBuffer = m_device.CreateVertexBuffer(vertices);
    m_indexBuffer = m_device.CreateIndexBuffer(indices);
    ID3D11Buffer *vbs[] = {m_vertexBuffer.get()};
    UINT strides[] = {sizeof(VertexPositionColor)};
    UINT offsets[] = {0};
    m_device.context()->IASetVertexBuffers(
            0, 1, vbs, strides, offsets);
    m_device.context()->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);

    // draw lines
    m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    m_device.context()->DrawIndexed(indices.size(), 0, 0);
}

void DxRenderer::drawLines(const vector<VertexPositionColor> &vertices,
                           const DirectX::XMMATRIX &mvp) {
    // map MVP matrix to shader buffer
    D3D11_MAPPED_SUBRESOURCE res;
    m_device.context()->Map(m_cbMVP.get(), 0,
                            D3D11_MAP_WRITE_DISCARD, 0, &res);
    memcpy(res.pData, &mvp, sizeof(XMMATRIX));
    m_device.context()->Unmap(m_cbMVP.get(), 0);

    // set vertex
    m_vertexBuffer = m_device.CreateVertexBuffer(vertices);
    ID3D11Buffer *vbs[] = {m_vertexBuffer.get()};
    UINT strides[] = {sizeof(VertexPositionColor)};
    UINT offsets[] = {0};
    m_device.context()->IASetVertexBuffers(
            0, 1, vbs, strides, offsets);

    // draw lines
    m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    m_device.context()->Draw(vertices.size(), 0);
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
    inputHandler.wheelEvent(event, size());
}

void DxRenderer::keyPressEvent(QKeyEvent *event) {
    QWidget::keyPressEvent(event);
    inputHandler.keyPressEvent(event);
}

void DxRenderer::keyReleaseEvent(QKeyEvent *event) {
    QWidget::keyReleaseEvent(event);
    inputHandler.keyReleaseEvent(event);
}

#pragma endregion Event_Handlers

#pragma region Init

void DxRenderer::init3D3() {
    setupViewport();

    const auto vsBytes = DxDevice::LoadByteCode(L"vs.cso");
    const auto psBytes = DxDevice::LoadByteCode(L"ps.cso");
    m_vertexShader = m_device.CreateVertexShader(vsBytes);
    m_pixelShader = m_device.CreatePixelShader(psBytes);

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