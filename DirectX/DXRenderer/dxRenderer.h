#pragma once

#include "DirectX/DXDevice/dxDevice.h"
#include "Handlers/inputHandler.h"
#include <windowsx.h>
#include <string>
#include <basetsd.h>

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <DirectXMath.h>

#undef max
#undef min

#define CLEAR_COLOR {0.5f, 0.5f, 0.5f, 1.0f}

class DxRenderer : public QWidget, public Renderer {
Q_OBJECT

public:
    explicit DxRenderer(QWidget *parent);

    void renderScene();

    void setScene(std::shared_ptr<Scene> scenePtr);

    void drawLines(const std::vector<VertexPositionColor> &vertices,
                   const std::vector<Index> &indices,
                   const DirectX::XMMATRIX &mvp, bool selected) override;

    void drawCursor(const DirectX::XMMATRIX &mvp) override;

    void drawPoint(const DirectX::XMMATRIX &mvp, bool selected) override;

    void setDrawColor(DirectX::XMFLOAT4 color);

    QPaintEngine *paintEngine() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

private:
    std::shared_ptr<Scene> scene;
    InputHandler inputHandler;

    DxDevice m_device;

    mini::dx_ptr<ID3D11RenderTargetView> m_backBuffer;
    mini::dx_ptr<ID3D11DepthStencilView> m_depthBuffer;

    mini::dx_ptr<ID3D11Buffer> m_vertexBuffer;
    mini::dx_ptr<ID3D11Buffer> m_indexBuffer;

    mini::dx_ptr<ID3D11Buffer> m_pointVertexBuffer;
    mini::dx_ptr<ID3D11Buffer> m_pointIndexBuffer;
    size_t pointBufferSize;

    mini::dx_ptr<ID3D11Buffer> m_cursorBuffer;
    size_t cursorBufferSize;

    mini::dx_ptr<ID3D11VertexShader> m_vertexShader;
    mini::dx_ptr<ID3D11PixelShader> m_pixelShader;
    mini::dx_ptr<ID3D11InputLayout> m_layout;

    mini::dx_ptr<ID3D11Buffer> m_cbMVP;
    mini::dx_ptr<ID3D11Buffer> m_cbColor;

    LARGE_INTEGER currentTicks, ticksPerSecond;

    const DirectX::XMFLOAT4 SELECTED_COLOR {1.0f, 0.4f, 0.0f, 1.0f};
    const DirectX::XMFLOAT4 DEFAULT_COLOR {0.0f, 0.0f, 0.0f, 0.0f};

    void init3D3();

    void setupViewport();

    void mapShaderMatrix(const DirectX::XMMATRIX &matrix);

    float frameTime();
};
