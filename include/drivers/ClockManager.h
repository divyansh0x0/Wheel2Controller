#ifndef WHEEL2FIRMWARE_CLOCKMANAGER_H
#define WHEEL2FIRMWARE_CLOCKMANAGER_H

#include <cstdint>
#include "drivers/MemoryMap.h"

namespace STM32::ClockManager {
    // Dynamic query of CPU System Clock frequency (SYSCLK)
    inline uint32_t getSYSCLK() {
        // Read SWS bits (System clock switch status, bits 3:2 of RCC_CFGR)
        const uint32_t sws = (MemoryMap::RCC1->CFGR >> 2) & 0b11;
        
        if (sws == 0b00) {
            // HSI is selected
            return 8000000;
        } else if (sws == 0b01) {
            // HSE is selected
            return 8000000; // Standard 8 MHz external crystal
        } else if (sws == 0b10) {
            // PLL is selected as system clock source
            // Read PLL Source (PLLSRC, bit 16 of RCC_CFGR)
            const bool pll_src_hse = (MemoryMap::RCC1->CFGR & (1 << 16)) != 0;
            uint32_t pll_input = pll_src_hse ? 8000000 : 4000000; // HSE (8MHz) or HSI/2 (4MHz)
            
            // Read PLL Multiplication factor (PLLMUL, bits 21:18 of RCC_CFGR)
            // 0b0000: PLL input clock x 2
            // 0b0001: PLL input clock x 3
            // ...
            // 0b1110: PLL input clock x 16
            const uint32_t pll_mul = ((MemoryMap::RCC1->CFGR >> 18) & 0b1111) + 2;
            return pll_input * pll_mul;
        }
        
        return 8000000; // Fallback to HSI
    }

    // Dynamic query of AHB clock frequency (HCLK)
    inline uint32_t getHCLK() {
        const uint32_t sysclk = getSYSCLK();
        
        // Read HPRE bits (AHB prescaler, bits 7:4 of RCC_CFGR)
        const uint32_t hpre = (MemoryMap::RCC1->CFGR >> 4) & 0b1111;
        if (hpre < 0b1000) {
            return sysclk; // No division
        }
        
        // Prescaler division: 2, 4, 8, 16, 64, 128, 256, 512
        const uint32_t shift = (hpre & 0b0111) + 1; // 1 to 8
        if (hpre >= 0b1100) {
            // Skip 32, division maps to:
            // 0b1100 -> /64 (shift 6)
            // 0b1101 -> /128 (shift 7)
            // 0b1110 -> /256 (shift 8)
            // 0b1111 -> /512 (shift 9)
            return sysclk >> (shift + 1); 
        }
        return sysclk >> shift;
    }

    // Dynamic query of APB1 clock frequency (PCLK1)
    inline uint32_t getPCLK1() {
        const uint32_t hclk = getHCLK();
        
        // Read PPRE1 bits (APB1 prescaler, bits 10:8 of RCC_CFGR)
        const uint32_t ppre1 = (MemoryMap::RCC1->CFGR >> 8) & 0b111;
        if (ppre1 < 0b100) {
            return hclk; // No division
        }
        
        // Division by 2, 4, 8, 16
        const uint32_t shift = (ppre1 & 0b011) + 1;
        return hclk >> shift;
    }

    // Dynamic query of APB2 clock frequency (PCLK2)
    inline uint32_t getPCLK2() {
        const uint32_t hclk = getHCLK();
        
        // Read PPRE2 bits (APB2 prescaler, bits 13:11 of RCC_CFGR)
        const uint32_t ppre2 = (MemoryMap::RCC1->CFGR >> 11) & 0b111;
        if (ppre2 < 0b100) {
            return hclk; // No division
        }
        
        // Division by 2, 4, 8, 16
        const uint32_t shift = (ppre2 & 0b011) + 1;
        return hclk >> shift;
    }
}

#endif // WHEEL2FIRMWARE_CLOCKMANAGER_H
