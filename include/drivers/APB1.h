//
// Created by divyansh on 5/31/26.
//

#ifndef WHEEL2FIRMWARE_APB1_H
#define WHEEL2FIRMWARE_APB1_H

namespace STM32 {
    class APB1 {
        enum class ClockOutput {
            None = 0b0000,
            System = 0b0100,
            HSI= 0b0101,
            HSE= 0b0110,
            PLL= 0b0111,
            PLL2= 0b1000,
            PLL3= 0b1001,
            XT1= 0b1010,
            PLL3Ethernet= 0b1011,
        };
        enum class PLLMultiplier {
            Times4 = 2,
            Times5 = 3,
            Times6 = 4,
            Times7 = 5,
            Times8 = 6,
            Times9 = 7,
            Times6half = 0b1101

        };
        private:
        APB1() = default;
    };
}


#endif //WHEEL2FIRMWARE_APB1_H