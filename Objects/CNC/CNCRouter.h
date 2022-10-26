//
// Created by Molom on 14/10/2022.
//

#ifndef MG1_CNCROUTER_H
#define MG1_CNCROUTER_H

#include <QMessageBox>
#include "CNCTool.h"
#include "Objects/updatable.h"
#include "Objects/linestrip.h"

enum RouterState {
    Created,
    FirstPathLoaded,
    NextPathLoaded,
    Started,
    Skipped,
    Finished
};

class CNCRouter : public Object, public Updatable {
    static constexpr float TOOL_SPEED = 15.f;
    static constexpr int PATHS_PER_FRAME_SKIP = 10;
    static const DirectX::XMFLOAT3 NEUTRAL_TOOL_POSITION;

public:
    CNCRouter(uint id, DirectX::XMFLOAT3 position);

    void setPosition(DirectX::XMFLOAT3 position) override;

    void setRotation(DirectX::XMFLOAT3 rotation) override {}

    void setScale(DirectX::XMFLOAT3 scale) override {}

    void draw(Renderer &renderer, DrawType drawType) override;

    Type type() const override;

    void update(Renderer &renderer, float frameTime) override;

    void loadPath(CNCPath &&path);

    DirectX::XMFLOAT3 size() const { return _size; }

    void setSize(DirectX::XMFLOAT3 size);

    std::pair<int, int> pointsDensity() const { return _pointsDensity; }

    void setPointsDensity(std::pair<int, int> density);

    float maxDepth() const { return _maxDepth; }

    void setMaxDepth(float depth);

    CNCType toolType() const { return tool.endType(); }

    void setToolType(CNCType type);

    int toolSize() const { return tool.size(); }

    void setToolSize(int size);

    int toolWorkingHeight() const { return _workingHeight; }

    void setToolWorkingHeight(int height) { _workingHeight = height; }

    bool showPaths() const { return _showPaths; }

    void setShowPaths(bool show);

    bool wireframe() const { return _wireframe; }

    void setWireframe(bool wire) { _wireframe = wire; }

    RouterState state() const { return _state; }

    QBindable<RouterState> bindableState() { return &_state; }

    int simulationSpeed() const { return _simulationSpeed; }

    void setSimulationSpeed(int speed);

    int progress() const { return _progress; }

    QBindable<int> bindableProgress() { return &_progress; }

    QString filename() const { return _filename; }

    void start();

    void skip();

    void reset();

    void stop();

    const mini::dx_ptr<ID3D11DepthStencilView> &depth() const { return _depth; }
    const mini::dx_ptr<ID3D11ShaderResourceView> &depthTexture() const { return _depthTexture; }
    const mini::dx_ptr<ID3D11ShaderResourceView> &prevDepthTexture() const { return _prevDepthTexture; }

    const mini::dx_ptr<ID3D11ShaderResourceView> &normal() const { return _normal; }
    const mini::dx_ptr<ID3D11UnorderedAccessView> &normalUnordered() const { return _normalUnordered; }

    const mini::dx_ptr<ID3D11UnorderedAccessView> &errorUnordered() const { return _errorUnordered; }

private:
    mini::dx_ptr<ID3D11ShaderResourceView> _depthTexture;
    mini::dx_ptr<ID3D11ShaderResourceView> _prevDepthTexture;
    mini::dx_ptr<ID3D11DepthStencilView> _depth;

    mini::dx_ptr<ID3D11ShaderResourceView> _normal;
    mini::dx_ptr<ID3D11UnorderedAccessView> _normalUnordered;

    mini::dx_ptr<ID3D11Texture1D> _errorStaging;
    mini::dx_ptr<ID3D11UnorderedAccessView> _errorUnordered;


    CNCPath routerPath;
    Linestrip drawPaths;

    DirectX::XMFLOAT3 _size{18.f, 18.f, 4.6f};
    std::pair<int, int> _pointsDensity{4096, 4096};
    float _maxDepth{3.5};
    CNCTool tool;
    int _workingHeight{30};
    int _simulationSpeed{1};
    bool _showPaths{false}, fresh{true}, _wireframe{false};
    QString _filename;
    QProperty<int> _progress{0};
    QProperty<RouterState> _state{static_cast<RouterState>(0)};

    Mesh textureDisk, textureSquare;
    Mesh textureDome, textureHalfCylinder;
    DirectX::XMFLOAT4X4 pathToTexture, toolScale;

    QMessageBox *errorBox;
    int currentLine{2};

    void fillDrawPaths();

    void generateBlock();

    void generateFaceZ(bool ccw, float z, float normalZ, float deltaX, float deltaY, int pointsX, int pointsY);

    void generateFaceX(bool ccw, float x, float normalX, float deltaY, int pointsY);

    void generateFaceY(bool ccw, float y, float normalY, float deltaX, int pointsX);

    void createDepthAndTexture(const DxDevice &device);

    void clearDepth(const DxDevice &device);

    void clearErrorMap(const DxDevice &device);

    void copyDepth(const DxDevice &device);

    void copyErrorMap(const DxDevice &device);

    void carvePaths(std::vector<std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3>> paths, Renderer &renderer);

    void checkForErrors(const DxDevice &device);

    void showErrorAndFinish(const QString &text);

    void calculatePathToTexture();
};


#endif //MG1_CNCROUTER_H
