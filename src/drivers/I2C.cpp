//
// Created by divyansh on 5/31/26.
//

#include "drivers/I2C.h"

#include "drivers/ClockManager.h"
#include "drivers/MemoryMap.h"

namespace STM32 {
    I2C::I2C(I2CType type):
    _i2c_mm(reinterpret_cast<MemoryMap::I2C*>(type == I2CType::I2C2_B10B11 ? 0x4000'5800 : 0x4000'5400)) {
        uint8_t scl{};
        uint8_t sda{};
        MemoryMap::APB1Peripheral i2cPeripheral{};

        switch (type) {
            case I2CType::I2C1_B6B7:
                scl = 6;
                sda = 7;
                i2cPeripheral = MemoryMap::APB1Peripheral::I2C1;
                break;

            case I2CType::I2C1_B8B9:
                scl = 8;
                sda = 9;
                i2cPeripheral = MemoryMap::APB1Peripheral::I2C1;
                break;
            default:
            case I2CType::I2C2_B10B11:
                scl = 10;
                sda = 11;
                i2cPeripheral = MemoryMap::APB1Peripheral::I2C2;
                break;
        }

        MemoryMap::RCC1->enablePeripheral(MemoryMap::APB2Peripheral::GPIOB);
        MemoryMap::RCC1->enablePeripheral(i2cPeripheral);

        MemoryMap::GPIOB->setPinMode(scl, MemoryMap::GPIO::Mode::SlowAlternateOpenDrain);
        MemoryMap::GPIOB->setPinMode(sda, MemoryMap::GPIO::Mode::SlowAlternateOpenDrain);

        const uint32_t freq_mhz = ClockManager::getPCLK1()/1000'000;
        // Reset the I2C bus
        constexpr uint32_t peripheral_enable_bit = 0;
        _i2c_mm->CR1 &= ~(1 << peripheral_enable_bit);
        constexpr  uint32_t freq_bit = 0;
        //reset the frequency bit
        _i2c_mm->CR2 &= ~(0b111'111 << freq_bit);
        // Set frequency
        _i2c_mm->CR2 |= (freq_mhz << freq_bit);
        // Set SCL frequency to slow mode
        constexpr uint32_t fs_bit= 15;
        _i2c_mm->CCR &= ~(0b1 << fs_bit); //reset
        _i2c_mm->CCR |= (0b0 << fs_bit); //set standard mode bit
        // We are using standard mode i2c, so the SCL will be at 100kHz or 0.1Mhz which is 10us of scl period.
        // Time period of SCL = CCR * Time period of APB1 clock
        constexpr uint32_t scl_bit = 0;
        _i2c_mm->CCR &= ~(0b1111'1111'1111 << scl_bit); //reset
        //for 100khz, T=10us = CCR * T_PCLK1 => CCR = T/T_PCLK1
        _i2c_mm->CCR |= (ClockManager::getPCLK1()/100'000/2<< scl_bit); //set

        // Maximum allowed rise time is 1000ns and trise is 1000ns*freq_ghz
        _i2c_mm->TRISE &= ~(0b111'111); //clear trise
        _i2c_mm->TRISE |= freq_mhz + 1; //set trise
        // enable the I2C bus
        _i2c_mm->CR1 |= 1 << peripheral_enable_bit;
    }



}
