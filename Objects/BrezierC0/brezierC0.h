//
// Created by Molom on 2022-03-25.
//

#ifndef MG1_BREZIERC0_H
#define MG1_BREZIERC0_H

#include "Objects/BrezierCurve/brezierCurve.h"

class BrezierC0 : public BrezierCurve {
public:
    explicit BrezierC0(std::vector<std::weak_ptr<Point>> &&points);

    Type type() const override;
};


#endif //MG1_BREZIERC0_H
