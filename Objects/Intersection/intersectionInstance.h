//
// Created by Molom on 2022-06-23.
//

#ifndef MG1_INTERSECTIONINSTANCE_H
#define MG1_INTERSECTIONINSTANCE_H

#include "DirectX/renderer.h"
#include "Objects/renderable.h"

class IntersectionInstance : public Renderable {
public:
    static constexpr uint SIZE = 512;

    IntersectionInstance(const std::vector<std::pair<float, float>> &parameters,
                         const std::array<std::tuple<float, float>, 2> &range,
                         const std::array<bool, 2> &looped, bool closed, Renderer &renderer);

    IntersectionInstance(const std::vector<std::pair<float, float>> &firstParameters,
                         const std::vector<std::pair<float, float>> &secondParameters,
                         const std::array<std::tuple<float, float>, 2> &range,
                         const std::array<bool, 2> &looped, bool closed, Renderer &renderer);

    const mini::dx_ptr<ID3D11RenderTargetView> &target() const { return _target; }

    const mini::dx_ptr<ID3D11ShaderResourceView> &texture() const { return _texture; }

    const QPixmap pixmap() const { return _pixmap; }

    void setActive(bool active) { _active = active; }

    void setFirst(bool first) { _first = first; }

    bool active() const { return _active; }

    bool first() const { return _first; }

private:
    mini::dx_ptr<ID3D11ShaderResourceView> _texture;
    mini::dx_ptr<ID3D11RenderTargetView> _target;
    QPixmap _pixmap;

    bool _active{true}, _first{true};

    mini::dx_ptr<ID3D11Texture2D> createTargetAndTexture(const DxDevice &device);

    void mapAndFill(const std::array<bool, 2> &looped, const DxDevice &device,
                    const mini::dx_ptr<ID3D11Texture2D> &stagingTex, bool withPixmap = true);

    static void floodFill(float *data, const std::array<bool, 2> &looped);

    void createPixmap(float *data);

    void fillBuffer(const std::vector<std::pair<float, float>> &parameters, const std::array<bool, 2> &looped,
                    bool closed, DirectX::XMMATRIX transform);
};


#endif //MG1_INTERSECTIONINSTANCE_H
