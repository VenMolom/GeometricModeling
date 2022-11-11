#include "dxRenderer.h"
#include "DirectX/DXDevice/exceptions.h"
#include "Objects/object.h"
#include "Objects/Curve/brezierCurve.h"
#include "Objects/Curve/interpolationCurveC2.h"
#include "Objects/CNC/CNCRouter.h"

using namespace mini;
using namespace std;
using namespace DirectX;

DxRenderer::DxRenderer(QWidget *parent) : m_device(this),
                                          m_viewport({width(), height()}),
                                          inputHandler() {
    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_NativeWindow, true);

    init3D3();
}

void DxRenderer::renderScene(float deltaTime) {
    auto backBuffer = m_backBuffer.get();
    m_device.context()->OMSetRenderTargets(1, &backBuffer, m_depthBuffer.get());
    m_device.context()->ClearRenderTargetView(m_backBuffer.get(), CLEAR_COLOR);

    m_device.context()->ClearDepthStencilView(
            m_depthBuffer.get(),
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    if (stereoscopic && scene) {
        renderStereoscopic();
        return;
    }

    if (scene) {
        updateCameraCB();
        scene->update(*this, deltaTime);
        scene->draw(*this);
    }

    if (inputHandler.selector().enabled) {
        drawSelector(inputHandler.selector());
    }
}

void DxRenderer::renderStereoscopic() {
    auto projections = scene->camera()->stereoscopicProjectionMatrix();
    auto views = scene->camera()->stereoscopicViewMatrix();
    clearColor = (float *) STEREO_CLEAR_COLOR;

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
    clearColor = (float *) CLEAR_COLOR;

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
    scene->camera()->resize(size());
    this->inputHandler.setScene(scene);

    updateBuffer(m_cbProj, scene->camera()->projectionMatrix());
    updateBuffer(m_cbFarPlane, XMFLOAT4{scene->camera()->nearZ(), scene->camera()->farZ(), 0, 0});
    updateCameraCB();
}

void DxRenderer::draw(const Object &object, XMFLOAT4 color) {
    updateBuffer(m_cbModel, object.modelMatrix());
    updateBuffer(m_cbColor, color);

    if (object.noDepth) {
        m_device.context()->OMSetDepthStencilState(m_dssNoDepth.get(), 0);
    }

    object.render(m_device.context());

    m_device.context()->OMSetDepthStencilState(nullptr, 0);
}

void DxRenderer::draw(const Torus &torus, DirectX::XMFLOAT4 color) {
    updateBuffer(m_cbModel, torus.modelMatrix());
    updateBuffer(m_cbColor, color);

    auto intersection = torus.intersectionInstance();
    if (intersection && intersection->active()) {
        m_device.context()->PSSetShader(m_pixelParamShader.get(), nullptr, 0);
        updateBuffer(m_cbTrim, XMFLOAT4(static_cast<int>(intersection->first()), 0, 0, 0));
        setTextures({intersection->texture().get()}, m_sampler);
    }

    m_device.context()->IASetInputLayout(m_paramLayout.get());
    m_device.context()->VSSetShader(m_vertexParamShader.get(), nullptr, 0);

    torus.render(m_device.context());

    m_device.context()->IASetInputLayout(m_layout.get());
    m_device.context()->VSSetShader(m_vertexShader.get(), nullptr, 0);
    m_device.context()->PSSetShader(m_pixelShader.get(), nullptr, 0);
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
    updateBuffer(m_cbColor, *(float (*)[4]) clearColor);

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

void DxRenderer::draw(const Patch &patch, DirectX::XMFLOAT4 color) {
    updateBuffer(m_cbModel, XMMatrixIdentity());
    updateBuffer(m_cbColor, color);
    m_device.context()->RSSetState(m_noCullWireframe.get());
    m_device.context()->HSSetShader(m_hullBicubicShader.get(), nullptr, 0);
    m_device.context()->DSSetShader(m_domainBicubicShader.get(), nullptr, 0);

    // density.uv, size.uv
    auto density = patch.density();
    auto size = patch.size();
    XMINT4 tesselationAmount = {
            static_cast<int32_t>(density[0]),
            static_cast<int32_t>(density[1]),
            size[0], size[1]
    };
    updateBuffer(m_cbTesselation, tesselationAmount);

    auto intersection = patch.intersectionInstance();
    if (intersection && intersection->active()) {
        m_device.context()->PSSetShader(m_pixelParamShader.get(), nullptr, 0);
        updateBuffer(m_cbTrim, XMFLOAT4(static_cast<int>(intersection->first()), 0, 0, 0));
        setTextures({intersection->texture().get()}, m_sampler);
    }

    patch.render(m_device.context());

    m_device.context()->RSSetState(nullptr);
    m_device.context()->HSSetShader(nullptr, nullptr, 0);
    m_device.context()->DSSetShader(nullptr, nullptr, 0);
    m_device.context()->PSSetShader(m_pixelShader.get(), nullptr, 0);
}

void DxRenderer::draw(const BicubicC2 &patch, DirectX::XMFLOAT4 color) {
    updateBuffer(m_cbModel, XMMatrixIdentity());
    updateBuffer(m_cbColor, color);
    m_device.context()->RSSetState(m_noCullWireframe.get());
    m_device.context()->HSSetShader(m_hullBicubicShader.get(), nullptr, 0);
    m_device.context()->DSSetShader(m_domainBicubicDeBoorShader.get(), nullptr, 0);

    // density.uv, size.uv
    auto density = patch.density();
    auto size = patch.size();
    XMINT4 tesselationAmount = {
            static_cast<int32_t>(density[0]),
            static_cast<int32_t>(density[1]),
            size[0], size[1]
    };
    updateBuffer(m_cbTesselation, tesselationAmount);

    auto intersection = patch.intersectionInstance();
    if (intersection && intersection->active()) {
        m_device.context()->PSSetShader(m_pixelParamShader.get(), nullptr, 0);
        updateBuffer(m_cbTrim, XMFLOAT4(static_cast<int>(intersection->first()), 0, 0, 0));
        setTextures({intersection->texture().get()}, m_sampler);
    }

    patch.render(m_device.context());

    m_device.context()->RSSetState(nullptr);
    m_device.context()->HSSetShader(nullptr, nullptr, 0);
    m_device.context()->DSSetShader(nullptr, nullptr, 0);
    m_device.context()->PSSetShader(m_pixelShader.get(), nullptr, 0);
}

void DxRenderer::draw(const GregoryPatch &patch, DirectX::XMFLOAT4 color) {
    updateBuffer(m_cbModel, XMMatrixIdentity());
    updateBuffer(m_cbColor, color);
    m_device.context()->RSSetState(m_noCullWireframe.get());
    m_device.context()->HSSetShader(m_hullGregoryShader.get(), nullptr, 0);
    m_device.context()->DSSetShader(m_domainGregoryShader.get(), nullptr, 0);

    // density.uv
    auto density = patch.density();
    XMINT4 tesselationAmount = {
            static_cast<int32_t>(density[0]),
            static_cast<int32_t>(density[1]),
            0, 0
    };
    updateBuffer(m_cbTesselation, tesselationAmount);

    patch.render(m_device.context());

    m_device.context()->RSSetState(nullptr);
    m_device.context()->HSSetShader(nullptr, nullptr, 0);
    m_device.context()->DSSetShader(nullptr, nullptr, 0);
}

void DxRenderer::draw(const IntersectionInstance &instance, bool clear) {
    updateBuffer(m_cbModel, XMMatrixIdentity());

    auto &target = instance.target();
    if (clear) {
        float clearColor[4] = {1, 1, 1, 1};
        m_device.context()->ClearRenderTargetView(target.get(), clearColor);
    }
    auto renderTarget = target.get();
    Viewport v({IntersectionInstance::SIZE, IntersectionInstance::SIZE});
    m_device.context()->RSSetViewports(1, &v);
    m_device.context()->OMSetRenderTargets(1, &renderTarget, nullptr);
    m_device.context()->VSSetShader(m_vertexTextureShader.get(), nullptr, 0);
    m_device.context()->PSSetShader(m_pixelTextureShader.get(), nullptr, 0);

    instance.render(m_device.context());

    auto backBuffer = m_backBuffer.get();
    m_device.context()->OMSetRenderTargets(1, &backBuffer, m_depthBuffer.get());
    m_device.context()->RSSetViewports(1, &m_viewport);
    m_device.context()->VSSetShader(m_vertexShader.get(), nullptr, 0);
    m_device.context()->PSSetShader(m_pixelShader.get(), nullptr, 0);
}

void DxRenderer::draw(const Mesh &mesh, DirectX::XMMATRIX modelMatrix) {
    updateBuffer(m_cbModel, modelMatrix);
    updateBuffer(m_cbColor, mesh.color());

    m_device.context()->VSSetShader(m_vertexPhongShader.get(), nullptr, 0);
    m_device.context()->PSSetShader(m_pixelPhongShader.get(), nullptr, 0);

    mesh.render(m_device.context());

    m_device.context()->VSSetShader(m_vertexShader.get(), nullptr, 0);
    m_device.context()->PSSetShader(m_pixelShader.get(), nullptr, 0);
}

void DxRenderer::draw(const CNCRouter &router) {
    updateBuffer(m_cbModel, router.modelMatrix());
    updateBuffer(m_cbColor, XMFLOAT3{0.5f, 0.5f, 0.5f});

    if (router.wireframe()) {
        m_device.context()->RSSetState(m_wireframe.get());
    }
    m_device.context()->IASetInputLayout(m_phongTexLayout.get());
    m_device.context()->VSSetShader(m_vertexPhongTexShader.get(), nullptr, 0);
    m_device.context()->PSSetShader(m_pixelPhongTexShader.get(), nullptr, 0);
    setTextures({m_woodTexture.get(), router.normal().get()}, m_samplerBorder);
    setVSTextures({router.depthTexture().get()}, m_sampler);

    router.render(m_device.context());

    m_device.context()->RSSetState(nullptr);
    m_device.context()->IASetInputLayout(m_layout.get());
    m_device.context()->VSSetShader(m_vertexShader.get(), nullptr, 0);
    m_device.context()->PSSetShader(m_pixelShader.get(), nullptr, 0);
}

void DxRenderer::drawToTexture(const CNCRouter &router, vector<pair<Renderable *, DirectX::XMMATRIX>> toRender,
                               bool downMove) {
    static constexpr UINT NO_OFFSET = -1;
    static constexpr ID3D11ShaderResourceView *NULL_SRV = nullptr;
    static constexpr ID3D11UnorderedAccessView *NULL_UAV = nullptr;

    auto size = router.pointsDensity();
    Viewport v({size.first, size.second});
    m_device.context()->RSSetState(m_noCull.get());
    m_device.context()->RSSetViewports(1, &v);
    m_device.context()->OMSetRenderTargets(0, nullptr, router.depth().get());
    m_device.context()->VSSetShader(m_vertexTextureShader.get(), nullptr, 0);
    m_device.context()->PSSetShader(m_pixelTextureShader.get(), nullptr, 0);

    for (auto &pair: toRender) {
        updateBuffer(m_cbModel, pair.second);
        pair.first->render(m_device.context());
    }

    auto backBuffer = m_backBuffer.get();
    m_device.context()->RSSetState(nullptr);
    m_device.context()->OMSetRenderTargets(1, &backBuffer, m_depthBuffer.get());
    m_device.context()->RSSetViewports(1, &m_viewport);
    m_device.context()->VSSetShader(m_vertexShader.get(), nullptr, 0);
    m_device.context()->PSSetShader(m_pixelShader.get(), nullptr, 0);

    NormalBuffer nb{
            XMINT4(size.first, size.second, (int) router.toolType(), router.toolWorkingHeight()),
            XMFLOAT4(router.size().z, downMove ? 1.f : 0.f, 0.f, 0.f)
    };
    updateBuffer(m_cbNormal, nb);
    m_device.context()->CSSetShader(m_computeNormal.get(), nullptr, 0);

    ID3D11ShaderResourceView *shaderRes[] = {router.depthTexture().get(), router.prevDepthTexture().get()};
    m_device.context()->CSSetShaderResources(0, 2, shaderRes);
    ID3D11UnorderedAccessView *unordered[] = {router.normalUnordered().get(), router.errorUnordered().get()};
    m_device.context()->CSSetUnorderedAccessViews(0, 2, unordered, &NO_OFFSET);

    m_device.context()->Dispatch(std::ceil(size.first / 16.f), std::ceil(size.second / 16.f), 1);

    m_device.context()->CSSetShaderResources(0, 2, &NULL_SRV);
    m_device.context()->CSSetUnorderedAccessViews(0, 2, &NULL_UAV, &NO_OFFSET);
    m_device.context()->CSSetShader(nullptr, nullptr, 0);
}

void DxRenderer::drawToTexture(const Renderer::Textures& textures,
                               std::vector<std::shared_ptr<Object>> objects,
                               DirectX::XMMATRIX projection, DirectX::XMMATRIX view) {
    static constexpr UINT NO_OFFSET = -1;
    static constexpr ID3D11ShaderResourceView *NULL_SRV = nullptr;
    static constexpr ID3D11UnorderedAccessView *NULL_UAV = nullptr;

    updateBuffer(m_cbProj, projection);
    updateCameraCB(view);
    updateBuffer(m_cbModel, XMMatrixIdentity());

    Viewport v({textures.viewportSize.first, textures.viewportSize.second});
    m_device.context()->RSSetViewports(1, &v);
    m_device.context()->OMSetRenderTargets(0, nullptr, textures.depth);

    m_device.context()->RSSetState(m_noCull.get());
    m_device.context()->HSSetShader(m_hullBicubicShader.get(), nullptr, 0);
    m_device.context()->DSSetShader(m_domainBicubicDeBoorShader.get(), nullptr, 0);

    for (auto &object: objects) {
        auto patch = dynamic_pointer_cast<BicubicC2>(object);
        if (!patch) continue;

        auto size = patch->size();
        XMINT4 tesselationAmount = {32, 32, size[0], size[1]};
        updateBuffer(m_cbTesselation, tesselationAmount);

        patch->render(m_device.context());
    }

    auto backBuffer = m_backBuffer.get();
    m_device.context()->RSSetState(nullptr);
    m_device.context()->OMSetRenderTargets(1, &backBuffer, m_depthBuffer.get());
    m_device.context()->RSSetViewports(1, &m_viewport);
    m_device.context()->VSSetShader(m_vertexShader.get(), nullptr, 0);
    m_device.context()->PSSetShader(m_pixelShader.get(), nullptr, 0);

    m_device.context()->RSSetState(nullptr);
    m_device.context()->HSSetShader(nullptr, nullptr, 0);
    m_device.context()->DSSetShader(nullptr, nullptr, 0);

    updateBuffer(m_cbProj, scene->camera()->projectionMatrix());

    NormalBuffer nb{
            XMINT4(textures.viewportSize.first, textures.viewportSize.second, textures.toolSize, 0),
            XMFLOAT4(textures.materialSize.first, textures.materialSize.second, textures.materialDepth, 0.f)
    };
    updateBuffer(m_cbNormal, nb);
    m_device.context()->CSSetShader(m_computeHeight.get(), nullptr, 0);

    ID3D11ShaderResourceView *shaderRes[] = {textures.depthTexture};
    m_device.context()->CSSetShaderResources(0, 1, shaderRes);
    ID3D11UnorderedAccessView *unordered[] = {textures.unorderedTexture};
    m_device.context()->CSSetUnorderedAccessViews(0, 1, unordered, &NO_OFFSET);

    m_device.context()->Dispatch(std::ceil(textures.viewportSize.first / 16.f), std::ceil(textures.viewportSize.second / 16.f), 1);

    m_device.context()->CSSetShaderResources(0, 1, &NULL_SRV);
    m_device.context()->CSSetUnorderedAccessViews(0, 1, &NULL_UAV, &NO_OFFSET);
    m_device.context()->CSSetShader(nullptr, nullptr, 0);
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

void DxRenderer::setVSTextures(initializer_list<ID3D11ShaderResourceView *> resList,
                               const dx_ptr<ID3D11SamplerState> &sampler) {
    m_device.context()->VSSetShaderResources(0, resList.size(), resList.begin());
    auto s_ptr = sampler.get();
    m_device.context()->VSSetSamplers(0, 1, &s_ptr);
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

void DxRenderer::drawSelector(const Selector &selector) {
    auto viewport = scene->camera()->viewport();
    auto width = abs(selector.start.x() - selector.end.x()) / viewport.width();
    auto height = abs(selector.start.y() - selector.end.y()) / viewport.height();
    QPointF center = {
            (selector.start.x() + selector.end.x()) / viewport.width() - 1.f,
            -(selector.start.y() + selector.end.y()) / viewport.height() + 1.f
    };

    auto mtx = XMMatrixScaling(width, height, 0) * XMMatrixTranslation(center.x(), center.y(), 0);

    updateBuffer(m_cbModel, mtx);
    updateBuffer(m_cbColor, SELECTOR_COLOR);
    m_device.context()->IASetInputLayout(m_selectorLayout.get());
    m_device.context()->VSSetShader(m_vertexSelectorShader.get(), nullptr, 0);
    m_device.context()->PSSetShader(m_pixelSelectorShader.get(), nullptr, 0);
    m_device.context()->OMSetBlendState(m_bsAlpha.get(), nullptr, UINT_MAX);

    m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ID3D11Buffer *vb = m_selectorQuad.get();
    unsigned int stride = 2 * sizeof(float);
    unsigned int offset = 0;
    m_device.context()->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
    m_device.context()->Draw(6, 0);

    m_device.context()->IASetInputLayout(m_layout.get());
    m_device.context()->VSSetShader(m_vertexShader.get(), nullptr, 0);
    m_device.context()->PSSetShader(m_pixelShader.get(), nullptr, 0);
    m_device.context()->OMSetBlendState(nullptr, nullptr, UINT_MAX);
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

    renderScene(deltaTime);
    m_device.swapChain()->Present(1, 0);
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
    const auto vsStereoBytes = DxDevice::LoadByteCode(L"vsStereo.cso");
    const auto vsSelectorBytes = DxDevice::LoadByteCode(L"vsSelector.cso");
    const auto vsParamBytes = DxDevice::LoadByteCode(L"vsParam.cso");
    const auto vsPhongTexBytes = DxDevice::LoadByteCode(L"vsPhongTex.cso");
    m_vertexShader = m_device.CreateVertexShader(vsBytes);
    m_vertexStereoShader = m_device.CreateVertexShader(vsStereoBytes);
    m_vertexSelectorShader = m_device.CreateVertexShader(vsSelectorBytes);
    m_vertexParamShader = m_device.CreateVertexShader(vsParamBytes);
    m_vertexPhongTexShader = m_device.CreateVertexShader(vsPhongTexBytes);
    m_vertexTextureShader = m_device.CreateVertexShader(DxDevice::LoadByteCode(L"vsTexture.cso"));
    m_vertexBillboardShader = m_device.CreateVertexShader(DxDevice::LoadByteCode(L"vsBillboard.cso"));
    m_vertexNoProjectionShader = m_device.CreateVertexShader(DxDevice::LoadByteCode(L"vsNoProjection.cso"));
    m_vertexPhongShader = m_device.CreateVertexShader(DxDevice::LoadByteCode(L"vsPhong.cso"));
    m_hullBrezierShader = m_device.CreateHullShader(DxDevice::LoadByteCode(L"hsBrezier.cso"));
    m_hullBicubicShader = m_device.CreateHullShader(DxDevice::LoadByteCode(L"hsBicubic.cso"));
    m_hullGregoryShader = m_device.CreateHullShader(DxDevice::LoadByteCode(L"hsGregory.cso"));
    m_domainBrezierShader = m_device.CreateDomainShader(DxDevice::LoadByteCode(L"dsBrezier.cso"));
    m_domainBicubicShader = m_device.CreateDomainShader(DxDevice::LoadByteCode(L"dsBicubic.cso"));
    m_domainGregoryShader = m_device.CreateDomainShader(DxDevice::LoadByteCode(L"dsGregory.cso"));
    m_domainBicubicDeBoorShader = m_device.CreateDomainShader(DxDevice::LoadByteCode(L"dsBicubicDeBoor.cso"));
    m_geometryPointShader = m_device.CreateGeometryShader(DxDevice::LoadByteCode(L"gsPoint.cso"));
    m_pixelShader = m_device.CreatePixelShader(DxDevice::LoadByteCode(L"ps.cso"));
    m_pixelFadeShader = m_device.CreatePixelShader(DxDevice::LoadByteCode(L"psCameraFade.cso"));
    m_pixelStereoShader = m_device.CreatePixelShader(DxDevice::LoadByteCode(L"psStereo.cso"));
    m_pixelSelectorShader = m_device.CreatePixelShader(DxDevice::LoadByteCode(L"psSelector.cso"));
    m_pixelTextureShader = m_device.CreatePixelShader(DxDevice::LoadByteCode(L"psTexture.cso"));
    m_pixelParamShader = m_device.CreatePixelShader(DxDevice::LoadByteCode(L"psParam.cso"));
    m_pixelPhongShader = m_device.CreatePixelShader(DxDevice::LoadByteCode(L"psPhong.cso"));
    m_pixelPhongTexShader = m_device.CreatePixelShader(DxDevice::LoadByteCode(L"psPhongTex.cso"));
    m_computeNormal = m_device.CreateComputeShader(DxDevice::LoadByteCode(L"csNormal.cso"));
    m_computeHeight = m_device.CreateComputeShader(DxDevice::LoadByteCode(L"csHeight.cso"));

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
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
                    D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0,
                                                            static_cast<UINT>(offsetof(VertexPositionTexture, tex)),
                    D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    m_paramLayout = m_device.CreateInputLayout(elements, vsParamBytes);

    elements = {
            {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,
                    D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
                                                         static_cast<UINT>(offsetof(VertexPositionTex, tex)),
                    D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    m_stereoLayout = m_device.CreateInputLayout(elements, vsStereoBytes);

    elements = {
            {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,
             D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    m_selectorLayout = m_device.CreateInputLayout(elements, vsSelectorBytes);

    elements = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
                    D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
                                                            static_cast<UINT>(offsetof(VertexPositionNormalTex,
                                                                                       normal)),
                    D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0,
                                                            static_cast<UINT>(offsetof(VertexPositionNormalTex, tex)),
                    D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    m_phongTexLayout = m_device.CreateInputLayout(elements, vsPhongTexBytes);

    m_cbModel = m_device.CreateConstantBuffer<XMFLOAT4X4>();
    m_cbView = m_device.CreateConstantBuffer<XMFLOAT4X4, 2>();
    m_cbProj = m_device.CreateConstantBuffer<XMFLOAT4X4>();
    m_cbColor = m_device.CreateConstantBuffer<XMFLOAT4>();
    m_cbTesselation = m_device.CreateConstantBuffer<XMINT4>();
    m_cbFarPlane = m_device.CreateConstantBuffer<XMFLOAT4>();
    m_cbStereoColor = m_device.CreateConstantBuffer<XMFLOAT4, 2>();
    m_cbTrim = m_device.CreateConstantBuffer<XMFLOAT4>();
    m_cbNormal = m_device.CreateConstantBuffer<NormalBuffer>();

    ID3D11Buffer *vsCbs[] = {m_cbModel.get(), m_cbView.get(), m_cbProj.get()};
    ID3D11Buffer *psCbs[] = {m_cbColor.get(), m_cbFarPlane.get(), m_cbStereoColor.get(), m_cbTrim.get()};
    ID3D11Buffer *hsCbs[] = {m_cbTesselation.get()};
    ID3D11Buffer *gsCbs[] = {m_cbFarPlane.get(), m_cbProj.get()};
    ID3D11Buffer *csCbs[] = {m_cbNormal.get()};
    m_device.context()->VSSetConstantBuffers(0, 3, vsCbs);
    m_device.context()->PSSetConstantBuffers(0, 4, psCbs);
    m_device.context()->HSSetConstantBuffers(0, 1, hsCbs);
    m_device.context()->GSSetConstantBuffers(0, 2, gsCbs);
    m_device.context()->CSSetConstantBuffers(0, 1, csCbs);

    DepthStencilDescription dssDesc;
    dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    m_dssNoDepthWrite = m_device.CreateDepthStencilState(dssDesc);

    dssDesc.DepthEnable = false;
    m_dssNoDepth = m_device.CreateDepthStencilState(dssDesc);

    SamplerDescription samplerDesc;
    m_sampler = m_device.CreateSamplerState(samplerDesc);
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.BorderColor[3] = 0.f;
    m_samplerBorder = m_device.CreateSamplerState(samplerDesc);

    BlendDescription blendDesc;
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    m_bsAlpha = m_device.CreateBlendState(blendDesc);

    RasterizerDescription rsDesc;
    rsDesc.FillMode = D3D11_FILL_WIREFRAME;
    m_wireframe = m_device.CreateRasterizerState(rsDesc);
    rsDesc.CullMode = D3D11_CULL_NONE;
    m_noCullWireframe = m_device.CreateRasterizerState(rsDesc);
    rsDesc.FillMode = D3D11_FILL_SOLID;
    m_noCull = m_device.CreateRasterizerState(rsDesc);

    vector<VertexPositionTex> quad{
            {{-1.f, -1.f}, {.0f, 1.f}},
            {{-1.f, 1.f},  {.0f, .0f}},
            {{1.f,  -1.f}, {1.f, 1.f}},
            {{1.f,  -1.f}, {1.f, 1.f}},
            {{-1.f, 1.f},  {.0f, .0f}},
            {{1.f,  1.f},  {1.f, .0f}}

    };
    m_ndcQuad = m_device.CreateVertexBuffer(quad);

    vector<DirectX::XMFLOAT2> selectorNdc{
            {-1.f, -1.f},
            {-1.f, 1.f},
            {1.f,  -1.f},
            {1.f,  -1.f},
            {-1.f, 1.f},
            {1.f,  1.f}
    };
    m_selectorQuad = m_device.CreateVertexBuffer(selectorNdc);

    m_woodTexture = m_device.CreateShaderResourceView(L"../wood-grain-texture.jpg");

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
    m_viewport = Viewport{wndSize};
    m_device.context()->RSSetViewports(1, &m_viewport);

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