#pragma once

#include "DirectX/DXDevice/dxDevice.h"
#include "Handlers/inputHandler.h"
#include "DirectX/stereoscopicRenderer.h"
#include <windowsx.h>
#include <Windows.h>
#include <DirectXMath.h>
#include <QStatusBar>

#undef max
#undef min

class DxRenderer : public QWidget, public Renderer, public StereoscopicRenderer {
Q_OBJECT

public:
    explicit DxRenderer(QWidget *parent);

    void setScene(std::shared_ptr<Scene> scenePtr);

    void setStatusBar(QStatusBar *bar) { statusBar = bar; }

    void draw(const Object &object, DirectX::XMFLOAT4 color) override;

    void draw(const Torus &torus, DirectX::XMFLOAT4 color) override;

    void draw(const BrezierCurve &curve, DirectX::XMFLOAT4 color) override;

    void draw(const InterpolationCurveC2 &curve, DirectX::XMFLOAT4 color) override;

    void draw(const Grid &grid, DirectX::XMFLOAT4 color) override;

    void draw(const Point &point, DirectX::XMFLOAT4 color) override;

    void draw(const Patch &patch, DirectX::XMFLOAT4 color) override;

    void draw(const BicubicC2 &patch, DirectX::XMFLOAT4 color) override;

    void draw(const GregoryPatch &patch, DirectX::XMFLOAT4 color) override;

    void draw(const IntersectionInstance &instance, bool clear = true) override;

    void draw(const Mesh &mesh, DirectX::XMMATRIX modelMatrix) override;

    void draw(const CNCRouter &router) override;

    void drawSelector(const Selector &selector);

    void enableStereoscopy(bool enable) override;

    void setLeftEyeColor(DirectX::XMFLOAT3 color) override;

    void setRightEyeColor(DirectX::XMFLOAT3 color) override;

    QPaintEngine *paintEngine() const override;

    void handleKeyEvent(QKeyEvent *event);

protected:
    void paintEvent(QPaintEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void focusOutEvent(QFocusEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

private:
    std::shared_ptr<Scene> scene;
    InputHandler inputHandler;
    QStatusBar *statusBar;

    DxDevice m_device;
    Viewport m_viewport;

    mini::dx_ptr<ID3D11RenderTargetView> m_backBuffer;
    mini::dx_ptr<ID3D11DepthStencilView> m_depthBuffer;

    mini::dx_ptr<ID3D11VertexShader> m_vertexShader;
    mini::dx_ptr<ID3D11VertexShader> m_vertexBillboardShader;
    mini::dx_ptr<ID3D11VertexShader> m_vertexNoProjectionShader;
    mini::dx_ptr<ID3D11VertexShader> m_vertexStereoShader;
    mini::dx_ptr<ID3D11VertexShader> m_vertexSelectorShader;
    mini::dx_ptr<ID3D11VertexShader> m_vertexTextureShader;
    mini::dx_ptr<ID3D11VertexShader> m_vertexParamShader;
    mini::dx_ptr<ID3D11VertexShader> m_vertexPhongShader;
    mini::dx_ptr<ID3D11VertexShader> m_vertexPhongTexShader;

    mini::dx_ptr<ID3D11HullShader> m_hullBrezierShader;
    mini::dx_ptr<ID3D11HullShader> m_hullBicubicShader;
    mini::dx_ptr<ID3D11HullShader> m_hullGregoryShader;
    mini::dx_ptr<ID3D11DomainShader> m_domainBrezierShader;
    mini::dx_ptr<ID3D11DomainShader> m_domainBicubicShader;
    mini::dx_ptr<ID3D11DomainShader> m_domainGregoryShader;
    mini::dx_ptr<ID3D11DomainShader> m_domainBicubicDeBoorShader;

    mini::dx_ptr<ID3D11GeometryShader> m_geometryPointShader;

    mini::dx_ptr<ID3D11PixelShader> m_pixelShader;
    mini::dx_ptr<ID3D11PixelShader> m_pixelFadeShader;
    mini::dx_ptr<ID3D11PixelShader> m_pixelStereoShader;
    mini::dx_ptr<ID3D11PixelShader> m_pixelSelectorShader;
    mini::dx_ptr<ID3D11PixelShader> m_pixelTextureShader;
    mini::dx_ptr<ID3D11PixelShader> m_pixelParamShader;
    mini::dx_ptr<ID3D11PixelShader> m_pixelPhongShader;

    mini::dx_ptr<ID3D11InputLayout> m_layout;
    mini::dx_ptr<ID3D11InputLayout> m_stereoLayout;
    mini::dx_ptr<ID3D11InputLayout> m_paramLayout;
    mini::dx_ptr<ID3D11InputLayout> m_selectorLayout;
    mini::dx_ptr<ID3D11InputLayout> m_phongTexLayout;

    mini::dx_ptr<ID3D11Buffer> m_cbModel;
    mini::dx_ptr<ID3D11Buffer> m_cbView;
    mini::dx_ptr<ID3D11Buffer> m_cbProj;
    mini::dx_ptr<ID3D11Buffer> m_cbColor;
    mini::dx_ptr<ID3D11Buffer> m_cbTesselation;
    mini::dx_ptr<ID3D11Buffer> m_cbFarPlane;
    mini::dx_ptr<ID3D11Buffer> m_cbStereoColor;
    mini::dx_ptr<ID3D11Buffer> m_cbTrim;

    mini::dx_ptr<ID3D11Buffer> m_ndcQuad;
    mini::dx_ptr<ID3D11Buffer> m_selectorQuad;

    mini::dx_ptr<ID3D11DepthStencilState> m_dssNoDepthWrite;
    mini::dx_ptr<ID3D11DepthStencilState> m_dssNoDepth;

    LARGE_INTEGER currentTicks, ticksPerSecond, lastFrameRateTick;
    uint frameCount = 0, lastFrameRate = 0;

    const float CLEAR_COLOR[4]{0.2f, 0.2f, 0.2f, 1.0f};
    const float STEREO_CLEAR_COLOR[4]{0.f, 0.f, 0.f, 1.f};
    const float SELECTOR_COLOR[4]{0.f, 0.f, 1.f, .2f};

    float *clearColor = (float *) CLEAR_COLOR;

    bool stereoscopic{false};

    DirectX::XMFLOAT4 leftEyeColor, rightEyeColor;

    mini::dx_ptr<ID3D11SamplerState> m_sampler, m_samplerBorder;

    mini::dx_ptr<ID3D11BlendState> m_bsAlpha;

    mini::dx_ptr<ID3D11RasterizerState> m_noCullWireframe;

    mini::dx_ptr<ID3D11RenderTargetView> m_stereoscopicLeftTarget, m_stereoscopicRightTarget;
    mini::dx_ptr<ID3D11ShaderResourceView> m_stereoscopicLeftTexture, m_stereoscopicRightTexture;
    mini::dx_ptr<ID3D11ShaderResourceView> m_woodTexture;

    void renderScene(float deltaTime);

    void renderStereoscopic();

    void renderEye(const DirectX::XMMATRIX &projection, const DirectX::XMMATRIX &view,
                   const mini::dx_ptr<ID3D11RenderTargetView> &target);

    void init3D3();

    void setupViewport();

    template<typename T>
    void updateBuffer(const mini::dx_ptr<ID3D11Buffer> &buffer, const T &data);

    void setTextures(std::initializer_list<ID3D11ShaderResourceView*> resList,
                     const mini::dx_ptr<ID3D11SamplerState>& sampler);

    void setVSTextures(std::initializer_list<ID3D11ShaderResourceView*> resList,
                     const mini::dx_ptr<ID3D11SamplerState>& sampler);

    float frameTime();

    uint frameRate();

    void updateCameraCB(DirectX::XMMATRIX viewMatrix);

    void updateCameraCB() { updateCameraCB(scene->camera()->viewMatrix()); }

    void drawCurve(const Curve &curve, int lastPatchId, int lastPatchSize);
};
