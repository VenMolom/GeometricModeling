//
// Created by Molom on 2022-06-23.
//

#ifndef MG1_INTERSECTIONINSTANCE_H
#define MG1_INTERSECTIONINSTANCE_H

#include "DirectX/renderer.h"
#include "Objects/renderable.h"

class IntersectionInstance : public Renderable {
public:
    static constexpr uint SIZE = 256;

    IntersectionInstance(const std::vector<std::pair<float, float>> &parameters,
                         const std::array<std::tuple<float, float>, 2> &range, bool closed, Renderer &renderer);

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

    void floodFill(float *data);

    void createPixmap(float *data);
};


#endif //MG1_INTERSECTIONINSTANCE_H
