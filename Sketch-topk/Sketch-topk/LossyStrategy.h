#ifndef LOSSYSTRATEGY_H
#define LOSSYSTRATEGY_H

#include <stdint.h>

namespace Lossy {

class BaseStrategy {
public:
    virtual void operator()(uint32_t &num) = 0;
};

class MinusOneStrategy : public BaseStrategy {
public:
    void operator()(uint32_t &num) override {
        if (num > 0) {
            --num;
        }
    }
};
} // namespace Lossy

#endif