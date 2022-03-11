#pragma once

#include "DirectX/DXDevice/dxDevice.h"
#include "Scene/scene.h"
#include <QWheelEvent>
#include <windowsx.h>
#include <string>
#include <basetsd.h>

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

#undef max
#undef min

#define CLEAR_COLOR {0.5f, 0.5f, 1.0f, 1.0f}

class DxRenderer : public QWidget, public Renderer {
Q_OBJECT

public:
    explicit DxRenderer(QWidget *parent);

    void renderScene();

    void setScene(std::shared_ptr<Scene> scenePtr);

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

    QPointF lastMousePos;
    bool mouseButtonPressed;
    bool moveButtonPressed;

    DxDevice m_device;

    mini::dx_ptr<ID3D11RenderTargetView> m_backBuffer;
    mini::dx_ptr<ID3D11DepthStencilView> m_depthBuffer;

    mini::dx_ptr<ID3D11Buffer> m_vertexBuffer;
    mini::dx_ptr<ID3D11Buffer> m_indexBuffer;

    mini::dx_ptr<ID3D11VertexShader> m_vertexShader;
    mini::dx_ptr<ID3D11PixelShader> m_pixelShader;
    mini::dx_ptr<ID3D11InputLayout> m_layout;

    mini::dx_ptr<ID3D11Buffer> m_cbMVP;

    LARGE_INTEGER currentTicks, ticksPerSecond;

    void init3D3();

    void setupViewport();

    float getFrameTime();
};
