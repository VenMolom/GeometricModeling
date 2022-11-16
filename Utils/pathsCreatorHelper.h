//
// Created by Molom on 14/11/2022.
//

#ifndef MG1_PATHSCREATORHELPER_H
#define MG1_PATHSCREATORHELPER_H

#include "Objects/objectFactory.h"
#include "Handlers/intersectHandler.h"

namespace PathsCreatorHelper {
    enum FlatteningSegment {
        HandleExternal,
        MainRight,
        MainLeft,
        Dziubek,
    };

    struct Textures {
        mini::dx_ptr<ID3D11ShaderResourceView> depthTexture;
        mini::dx_ptr<ID3D11DepthStencilView> depth;
        mini::dx_ptr<ID3D11UnorderedAccessView> allowedHeight;
        mini::dx_ptr<ID3D11Texture2D> allowedHeightStaging;
    };

    static constexpr int TEX_SIZE = 2048;

    static constexpr float BLOCK_SIZE_XY = 15.f;
    static constexpr float BLOCK_SIZE_Z = 5.f;
    static constexpr float BLOCK_BOTTOM = 1.5f;

    static constexpr float BLOCK_BOTTOM_LOCAL = BLOCK_BOTTOM * 10.f;
    static constexpr float BLOCK_END_LOCAL = BLOCK_SIZE_XY * 5.f;

    static constexpr float START_X = 88.f;
    static constexpr float START_Y = 88.f;
    static constexpr float START_Z = 66.f;

    void addPositionsOnLine(std::vector<DirectX::XMFLOAT3> &positions,
                            float *data, int dir,
                            float baseZ, float xSize,
                            float y, int texY);

    std::vector<DirectX::XMFLOAT3> createZigZagLines(const float *data, int toolSize);

    void zigZagLines(std::vector<DirectX::XMFLOAT3> &zigZag, const float *data, int yMoves, float lineDistance,
                     float &y, float &x, int &dir);

    std::vector<DirectX::XMFLOAT3>
    calculateToolDistantPath(ParametricObject<2> &patch, const std::vector<std::pair<float, float>> &parameters,
                             const std::vector<DirectX::XMFLOAT3> &points, int toolSize,
                             FlatteningSegment segment);

    void insertSlerpNormalPosition(std::vector<DirectX::XMFLOAT3> &path, DirectX::XMVECTOR position,
                                   DirectX::XMVECTOR normalStart, DirectX::XMVECTOR normalEnd, float normalLength);

    PathsCreatorHelper::Textures createDepthTextures(const DxDevice &device);

    void copyResource(const DxDevice &device, mini::dx_ptr<ID3D11UnorderedAccessView> &source,
                      mini::dx_ptr<ID3D11Texture2D> &target);

    std::vector<DirectX::XMFLOAT3> intersectAndCalculateToolDistant(IntersectHandler &intersect,
                                                                    Renderer &renderer,
                                                                    std::shared_ptr<ParametricObject<2>> patch,
                                                                    std::shared_ptr<ParametricObject<2>> object,
                                                                    const std::array<float, 4> &starting,
                                                                    FlatteningSegment segment,
                                                                    int toolSize);

    std::tuple<std::vector<DirectX::XMFLOAT3>::iterator, std::vector<DirectX::XMFLOAT3>::iterator, DirectX::XMFLOAT3>
    findIntersection(std::vector<DirectX::XMFLOAT3>::iterator path1, std::vector<DirectX::XMFLOAT3>::iterator path2);
}


#endif //MG1_PATHSCREATORHELPER_H
