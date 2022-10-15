//
// Created by Molom on 15/10/2022.
//

#ifndef MG1_CNCTOOL_H
#define MG1_CNCTOOL_H

#include "Utils/fileParser.h"
#include "Objects/object.h"

class CNCTool: public Object {
public:
    CNCTool(DirectX::XMFLOAT3 position);

    CNCType endType() const { return _type; }

    void setType(CNCType type);

    int size() const { return _size; }

    void setSize(int size);

    void draw(Renderer &renderer, DrawType drawType) override;

    Type type() const override;

private:
    CNCType _type{static_cast<CNCType>(0)};
    int _size{1};
};


#endif //MG1_CNCTOOL_H
