#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

struct SwapChainDescription : DXGI_SWAP_CHAIN_DESC {
    SwapChainDescription(HWND wndHwnd, SIZE wndSize);
};

struct Viewport : D3D11_VIEWPORT {
    explicit Viewport(SIZE size);
};

struct Texture2DDescription : D3D11_TEXTURE2D_DESC {
    explicit Texture2DDescription(UINT width = 0U, UINT height = 0U);

    static Texture2DDescription DepthStencilDescription(UINT width, UINT height);
};

struct Texture1DDescription: D3D11_TEXTURE1D_DESC {
    explicit Texture1DDescription(UINT width = 0U);
};

struct BufferDescription : D3D11_BUFFER_DESC {
    BufferDescription(UINT bindFlags, size_t byteWidth);

    static BufferDescription VertexBufferDescription(size_t byteWidth) { return {D3D11_BIND_VERTEX_BUFFER, byteWidth}; }

    static BufferDescription IndexBufferDescription(size_t byteWidth) { return {D3D11_BIND_INDEX_BUFFER, byteWidth}; }

    static BufferDescription ConstantBufferDescription(size_t byteWidth);
};

struct VertexPositionColor {
    DirectX::XMFLOAT3 position, color;
};

struct VertexPositionTexture {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 tex;
};

struct VertexPositionTex {
    DirectX::XMFLOAT2 position;
    DirectX::XMFLOAT2 tex;
};

struct VertexPositionNormalTex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 tex;
};

typedef unsigned int Index;

struct BlendDescription : D3D11_BLEND_DESC {
    BlendDescription();
};

struct DepthStencilDescription : D3D11_DEPTH_STENCIL_DESC {
    DepthStencilDescription();
};

struct RasterizerDescription : D3D11_RASTERIZER_DESC {
    RasterizerDescription();
};

struct SamplerDescription : D3D11_SAMPLER_DESC {
    SamplerDescription();
};

struct ShaderResourceViewDescription : D3D11_SHADER_RESOURCE_VIEW_DESC
{
    ShaderResourceViewDescription();
};

struct DepthStencilViewDescription : D3D11_DEPTH_STENCIL_VIEW_DESC
{
    DepthStencilViewDescription();
};

struct UnorderedAccessViewDescription : D3D11_UNORDERED_ACCESS_VIEW_DESC
{
    UnorderedAccessViewDescription();
};