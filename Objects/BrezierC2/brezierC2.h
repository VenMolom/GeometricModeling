//
// Created by Molom on 2022-04-01.
//

#ifndef MG1_BREZIERC2_H
#define MG1_BREZIERC2_H

#include "Objects/BrezierCurve/brezierCurve.h"

class BrezierC2 : public BrezierCurve {
public:
    explicit BrezierC2(std::vector<std::weak_ptr<Point>> &&points);

    Type type() const override;

protected:
    void updatePoints() override;
};


#endif //MG1_BREZIERC2_H
