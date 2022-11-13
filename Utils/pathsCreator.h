//
// Created by Molom on 10/11/2022.
//

#ifndef MG1_PATHSCREATOR_H
#define MG1_PATHSCREATOR_H

#include <filesystem>
#include "Objects/object.h"
#include "Objects/objectFactory.h"

class PathsCreator {
    static constexpr float BLOCK_SIZE_XY = 15.f;
    static constexpr float BLOCK_SIZE_Z = 5.f;
    static constexpr float BLOCK_BOTTOM = 1.5f;

    static constexpr float BLOCK_BOTTOM_LOCAL = 15.f;

    static constexpr float START_X = 88.f;
    static constexpr float START_Y = 88.f;
    static constexpr float START_Z = 66.f;

public:
    static constexpr int TEX_SIZE = 2048;

    static void create(const std::filesystem::path &directory,
                       std::vector<std::shared_ptr<Object>> objects,
                       ObjectFactory &factory,
                       Renderer &renderer);

private:
    struct Textures {
        mini::dx_ptr<ID3D11ShaderResourceView> depthTexture;
        mini::dx_ptr<ID3D11DepthStencilView> depth;
        mini::dx_ptr<ID3D11UnorderedAccessView> allowedHeight;
        mini::dx_ptr<ID3D11Texture2D> allowedHeightStaging;
    };

    explicit PathsCreator(std::filesystem::path basePath, std::vector<std::shared_ptr<Object>> objects);

    void createRoughPaths(int toolSize, Renderer &renderer);

    void createFlatteningPaths(int toolSize, Renderer &renderer, ObjectFactory &factory);

    void createDetailPaths(int toolSize);

    void addPositionsOnLine(std::vector<DirectX::XMFLOAT3> &positions,
                            float *data, int dir,
                            float baseZ, float xSize,
                            float y, int texY);

    std::vector<DirectX::XMFLOAT3>
    calculateToolDistantPath(ParametricObject<2> &patch, Intersection &intersection, int toolSize);

    static PathsCreator::Textures createDepthTextures(const DxDevice &device);

    static void copyResource(const DxDevice &device, mini::dx_ptr<ID3D11UnorderedAccessView> &source,
                             mini::dx_ptr<ID3D11Texture2D> &target);

    std::filesystem::path basePath;
    std::vector<std::shared_ptr<Object>> objects;
};


#endif //MG1_PATHSCREATOR_H
