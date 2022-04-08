//
// Created by Molom on 2022-03-19.
//

#ifndef MG1_POINT_H
#define MG1_POINT_H

#include <DirectXMath.h>
#include "Objects/Object/object.h"

class Point : public Object {
public:
    Point(DirectX::XMFLOAT3 position);

    void draw(Renderer &renderer, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, DrawType drawType) override;

    Type type() const override;

    DirectX::BoundingOrientedBox boundingBox() const override;

private:
    static const std::vector<VertexPositionColor> pointVertices;
    static const std::vector<Index> pointIndices;

    DirectX::XMFLOAT3 size {0.1f, 0.1f, 0.1f};
    DirectX::XMFLOAT3 boundingBoxSize {0.2f, 0.2f, 0.2f};
    DirectX::XMFLOAT4 rot {0.0f, 0.0f, 0.0f, 1.0f};
};


#endif //MG1_POINT_H
