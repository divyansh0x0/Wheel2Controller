//
// Created by divyansh on 5/31/26.
//

#ifndef WHEEL2FIRMWARE_I2C_H
#define WHEEL2FIRMWARE_I2C_H
#include <cstdint>

#include "MemoryMap.h"

namespace STM32 {
    enum class I2CType : uint8_t {
        I2C1_B6B7,
        I2C1_B8B9,
        I2C2_B10B11,
    };

    class I2C {
        MemoryMap::I2C* _i2c_mm;
    public:
        explicit I2C(I2CType type);
    };
}


#endif //WHEEL2FIRMWARE_I2C_H
