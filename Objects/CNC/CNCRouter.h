//
// Created by Molom on 14/10/2022.
//

#ifndef MG1_CNCROUTER_H
#define MG1_CNCROUTER_H

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
    static constexpr float TOOL_SPEED = 25.f;
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

    bool showPaths() const { return _showPaths; }

    void setShowPaths(bool show);

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

private:
    CNCPath routerPath;
    Linestrip drawPaths;

    DirectX::XMFLOAT3 _size{10, 10, 5};
    std::pair<int, int> _pointsDensity{512, 512};
    float _maxDepth{3};
    CNCTool tool;
    int _simulationSpeed{1};
    bool _showPaths{false}, fresh{true};
    QString _filename;
    QProperty<int> _progress{0};
    QProperty<RouterState> _state{static_cast<RouterState>(0)};

    void fillDrawPaths();

    void generateBlock();

    void generateFaceZ(bool ccw, float z, float normalZ, float deltaX, float deltaY, int pointsX, int pointsY);

    void generateFaceX(bool ccw, float x, float normalX, float deltaY, int pointsY);

    void generateFaceY(bool ccw, float y, float normalY, float deltaX, int pointsX);
};


#endif //MG1_CNCROUTER_H
