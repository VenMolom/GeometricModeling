//
// Created by Molom on 2022-03-19.
//

#ifndef MG1_POINT_H
#define MG1_POINT_H

#include <DirectXMath.h>
#include "Objects/object.h"

class Point : public Object {
public:
    Point(uint id, DirectX::XMFLOAT3 position);

    void draw(Renderer &renderer, DrawType drawType) override;

    Type type() const override;

    DirectX::BoundingOrientedBox boundingBox() const override;

    void setRotation(DirectX::XMFLOAT3 rotation) override {}

    void setScale(DirectX::XMFLOAT3 scale) override {}

private:
    static const std::vector<VertexPositionColor> pointVertices;
    static const std::vector<Index> pointIndices;

    DirectX::XMFLOAT3 size {0.20f, 0.20f, 0.20f};
    DirectX::XMFLOAT3 boundingBoxSize {0.4f, 0.4f, 0.4f};
    DirectX::XMFLOAT4 rot {0.0f, 0.0f, 0.0f, 1.0f};
};


#endif //MG1_POINT_H
