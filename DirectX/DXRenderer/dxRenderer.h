#pragma once

#include "DirectX/DXDevice/dxDevice.h"
#include "Handlers/inputHandler.h"
#include <windowsx.h>
#include <string>
#include <basetsd.h>

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <DirectXMath.h>
#include <QStatusBar>

#undef max
#undef min

class DxRenderer : public QWidget, public Renderer {
    Q_OBJECT

public:
    explicit DxRenderer(QWidget *parent);

    void renderScene();

    void setScene(std::shared_ptr<Scene> scenePtr);

    void setStatusBar(QStatusBar *bar) { statusBar = bar; }

    void drawIndexed(const std::vector<VertexPositionColor> &vertices, const std::vector<Index> &indices,
                     Topology topology, const DirectX::XMMATRIX &mvp, DirectX::XMFLOAT4 colorOverride) override;

    void draw(const std::vector<VertexPositionColor> &points, Topology topology,
              const DirectX::XMMATRIX &mvp, DirectX::XMFLOAT4 colorOverride) override;

    void drawCurve4(const std::vector<VertexPositionColor> &controlPoints,
                    const std::vector<Index> &indices, int lastPatchSize,
                    DirectX::XMVECTOR min, DirectX::XMVECTOR max,
                    const DirectX::XMMATRIX &mvp, DirectX::XMFLOAT4 colorOverride) override;

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

    mini::dx_ptr<ID3D11RenderTargetView> m_backBuffer;
    mini::dx_ptr<ID3D11DepthStencilView> m_depthBuffer;

    mini::dx_ptr<ID3D11Buffer> m_vertexBuffer;
    mini::dx_ptr<ID3D11Buffer> m_indexBuffer;

    mini::dx_ptr<ID3D11VertexShader> m_vertexShader;
    mini::dx_ptr<ID3D11HullShader> m_hullShader;
    mini::dx_ptr<ID3D11DomainShader> m_domainShader;
    mini::dx_ptr<ID3D11PixelShader> m_pixelShader;
    mini::dx_ptr<ID3D11InputLayout> m_layout;

    mini::dx_ptr<ID3D11Buffer> m_cbMVP;
    mini::dx_ptr<ID3D11Buffer> m_cbColor;
    mini::dx_ptr<ID3D11Buffer> m_cbTesselation;

    LARGE_INTEGER currentTicks, ticksPerSecond;

    const float CLEAR_COLOR[4] {0.2f, 0.2f, 0.2f, 1.0f};

    void init3D3();

    void setupViewport();

    template<typename T>
    void updateBuffer(const mini::dx_ptr<ID3D11Buffer> &buffer, const T &data);

    float frameTime();
};
