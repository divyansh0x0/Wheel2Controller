/**
 * @file MemoryMap.h
 * @brief Bare-metal peripheral register memory mapping and hardware abstractions for STM32F103.
 * @details This file maps peripheral registers to C++ structure representations and provides
 * inline register manipulation functions for safe register access.
 */

#ifndef WHEEL2FIRMWARE_MEMORYMAP_H
#define WHEEL2FIRMWARE_MEMORYMAP_H

#include <cstdint>

namespace STM32::MemoryMap {
    inline constexpr unsigned int CPU_FREQUENCY = 72'000'000;

    /**
     * @brief Determines the internal pull-up/pull-down resistor connection for an input pin.
     */
    enum class GPIOPullState {
        Down = 0, ///< Configures input pin with internal pull-down resistor connected to VSS.
        Up = 1 ///< Configures input pin with internal pull-up resistor connected to VDD.
    };

    enum class BaudRate : unsigned int {
        Baud9600 = 9600,
        Baud19200 = 19200,
        Baud38400 = 38400,
        Baud57600 = 57600,
        Baud115200 = 115200,
        Baud230400 = 230400
    };

    /**
     * @brief Identifies peripherals connected to the High-Speed APB2 Bus.
     * @details Mapped to specific bit positions in the RCC_APB2ENR register.
     */
    enum class APB2Peripheral : unsigned int {
        AlternateFunctionIO = 0, ///< AFIO: Alternate function I/O configuration register clock enable.
        GPIOA = 2, ///< GPIO Port A clock enable.
        GPIOB = 3, ///< GPIO Port B clock enable.
        GPIOC = 4, ///< GPIO Port C clock enable.
        USART1 = 14 ///< USART1 clock enable.
    };

    /**
     * @brief Identifies peripherals connected to the Standard-Speed APB1 Bus.
     * @details Mapped to specific bit positions in the RCC_APB1ENR register.
     */
    enum class APB1Peripheral : unsigned int {
        TIM2 = 0, ///< TIM2 clock enable.
        TIM3 = 1, ///< TIM3 clock enable.
        TIM4 = 2, ///< TIM4 clock enable.
        I2C1 = 21, ///< I2C1 clock enable
        I2C2 = 22 ///< I2C2 clock enable
    };


    /**
     * @brief Identifies the 4 independent hardware channels inside a General Purpose Timer.
     */
    enum class TimerChannel : unsigned int {
        Channel1 = 1,
        Channel2 = 2,
        Channel3 = 3,
        Channel4 = 4
    };

    using register_t = unsigned int;

    /**
     * @brief General-purpose timer peripheral register map.
     * @details Represents the register layout for TIM2, TIM3, and TIM4.
     * Offsets map exactly to Section 15.4 of the RM0008 Reference Manual.
     * Read and write accesses must conform to the alignment and data width specifications
     * described in Section 3.1.
     */
    struct TIMER {
        volatile register_t CR1; ///< 0x00 TIMx Control Register 1 (TIMx_CR1).
        volatile register_t CR2; ///< 0x04 TIMx Control Register 2 (TIMx_CR2).
        volatile register_t SMCR; ///< 0x08 TIMx Slave Mode Control Register (TIMx_SMCR).
        volatile register_t DIER; ///< 0x0C TIMx DMA/Interrupt Enable Register (TIMx_DIER).
        volatile register_t SR; ///< 0x10 TIMx Status Register (TIMx_SR).
        volatile register_t EGR; ///< 0x14 TIMx Event Generation Register (TIMx_EGR).
        volatile register_t CCMR1; ///< 0x18 TIMx Capture/Compare Mode Register 1 (TIMx_CCMR1).
        volatile register_t CCMR2; ///< 0x1C TIMx Capture/Compare Mode Register 2 (TIMx_CCMR2).
        volatile register_t CCER; ///< 0x20 TIMx Capture/Compare Enable Register (TIMx_CCER).
        volatile register_t CNT; ///< 0x24 TIMx Counter Register (TIMx_CNT).
        volatile register_t PSC; ///< 0x28 TIMx Prescaler Register (TIMx_PSC).
        volatile register_t ARR; ///< 0x2C TIMx Auto-Reload Register (TIMx_ARR).
        volatile register_t RCR; ///< 0x30 TIMx Repetition Counter Register (TIMx_RCR).
        volatile register_t CCR1; ///< 0x34 TIMx Capture/Compare Register 1 (TIMx_CCR1).
        volatile register_t CCR2; ///< 0x38 TIMx Capture/Compare Register 2 (TIMx_CCR2).
        volatile register_t CCR3; ///< 0x3C TIMx Capture/Compare Register 3 (TIMx_CCR3).
        volatile register_t CCR4; ///< 0x40 TIMx Capture/Compare Register 4 (TIMx_CCR4).
        /**
         * @brief Configures the prescaler and auto-reload values of the timer.
         * @param prescaler Clock division factor (value written to PSC is prescaler - 1).
         * @param auto_reload Period count limit value (value written to ARR is auto_reload - 1).
         */
        // Target frequency in Hz, and your desired max value for 100% duty cycle
        void setFrequency(uint32_t target_hz, uint32_t resolution = 1000) {
            uint32_t system_clock = 72000000; // 72 MHz

            // Formula: PSC = SystemClock / (Frequency * ARR)
            // We subtract 1 because hardware registers are 0-indexed
            uint32_t psc_value = (system_clock / (target_hz * resolution)) - 1;
            uint32_t arr_value = resolution - 1;

            // Write to my actual hardware registers
            this->PSC = psc_value;
            this->ARR = arr_value;
        }

        /**
         * @brief Configures a specific channel for PWM Mode 1 and enables its output.
         * @param channel The target timer channel (1 to 4).
         */
        void enablePWM(TimerChannel channel) {
            switch (channel) {
                case TimerChannel::Channel1:
                    this->CCMR1 &= ~(0b11 << 0); // Configure channel 1 in output compare mode (CC1S = 00 in TIMx_CCMR1)
                    this->CCMR1 &= ~(0b111 << 4); // Clear output compare 1 mode configuration bits (OC1M)
                    this->CCMR1 |= (0b110 << 4); // Set OC1M to PWM Mode 1 (0b110)
                    this->CCMR1 |= (1 << 3); // Enable Output Compare 1 Preload (OC1PE)
                    this->CCER |= (1 << 0); // Enable Output Compare 1 output (CC1E in TIMx_CCER)
                    break;
                case TimerChannel::Channel2:
                    this->CCMR1 &= ~(0b11 << 8); // Configure channel 2 in output compare mode (CC2S = 00 in TIMx_CCMR1)
                    this->CCMR1 &= ~(0b111 << 12); // Clear output compare 2 mode configuration bits (OC2M)
                    this->CCMR1 |= (0b110 << 12); // Set OC2M to PWM Mode 1 (0b110)
                    this->CCMR1 |= (1 << 11); // Enable Output Compare 2 Preload (OC2PE)
                    this->CCER |= (1 << 4); // Enable Output Compare 2 output (CC2E in TIMx_CCER)
                    break;
                case TimerChannel::Channel3:
                    this->CCMR2 &= ~(0b11 << 0); // Configure channel 3 in output compare mode (CC3S = 00 in TIMx_CCMR2)
                    this->CCMR2 &= ~(0b111 << 4); // Clear output compare 3 mode configuration bits (OC3M)
                    this->CCMR2 |= (0b110 << 4); // Set OC3M to PWM Mode 1 (0b110)
                    this->CCMR2 |= (1 << 3); // Enable Output Compare 3 Preload (OC3PE)
                    this->CCER |= (1 << 8); // Enable Output Compare 3 output (CC3E in TIMx_CCER)
                    break;
                case TimerChannel::Channel4:
                    this->CCMR2 &= ~(0b11 << 8); // Configure channel 4 in output compare mode (CC4S = 00 in TIMx_CCMR2)
                    this->CCMR2 &= ~(0b111 << 12); // Clear output compare 4 mode configuration bits (OC4M)
                    this->CCMR2 |= (0b110 << 12); // Set OC4M to PWM Mode 1 (0b110)
                    this->CCMR2 |= (1 << 11); // Enable Output Compare 4 Preload (OC4PE)
                    this->CCER |= (1 << 12); // Enable Output Compare 4 output (CC4E in TIMx_CCER)
                    break;
            }
        }

        /**
         * @brief Updates the duty cycle value for a specific timer channel.
         * @param channel The target timer channel (1 to 4).
         * @param value The compare value written to the Capture/Compare Register (CCR).
         */
        void setDutyCycle(TimerChannel channel, unsigned int value) {
            switch (channel) {
                case TimerChannel::Channel1: this->CCR1 = value;
                    break;
                case TimerChannel::Channel2: this->CCR2 = value;
                    break;
                case TimerChannel::Channel3: this->CCR3 = value;
                    break;
                case TimerChannel::Channel4: this->CCR4 = value;
                    break;
            }
        }

        /**
         * @brief Enables the timer counter.
         */
        void start() {
            this->CR1 |= (1 << 0); // Set CEN (Counter Enable) bit in TIMx_CR1
        }
    };

    static_assert(sizeof(TIMER) == 0x44);

    /**
     * @brief Reset and Clock Control (RCC) peripheral memory map.
     * @details Mapped to physical memory offsets specified in Section 7.3 of the RM0008 Reference Manual.
     * Manages peripheral reset state and clock enable/disable settings.
     */
    struct RCC {
        volatile register_t CR; ///< 0x00 Clock Control Register (RCC_CR).
        volatile register_t CFGR; ///< 0x04 Clock Configuration Register (RCC_CFGR).
        volatile register_t CIR; ///< 0x08 Clock Interrupt Register (RCC_CIR).
        volatile register_t APB2RSTR; ///< 0x0C APB2 Peripheral Reset Register (RCC_APB2RSTR).
        volatile register_t APB1RSTR; ///< 0x10 APB1 Peripheral Reset Register (RCC_APB1RSTR).
        volatile register_t AHBENR; ///< 0x14 AHB Peripheral Clock Enable Register (RCC_AHBENR).
        volatile register_t APB2ENR; ///< 0x18 APB2 Peripheral Clock Enable Register (RCC_APB2ENR).
        volatile register_t APB1ENR; ///< 0x1C APB1 Peripheral Clock Enable Register (RCC_APB1ENR).
        volatile register_t BDCR; ///< 0x20 Backup Domain Control Register (RCC_BDCR).
        volatile register_t CSR; ///< 0x24 Control/Status Register (RCC_CSR).
        /**
         * @brief Enables the clock for a peripheral on the APB2 bus.
         * @param peripheral The target APB2 peripheral bit offset value.
         */
        void enableClock(APB2Peripheral peripheral) {
            this->APB2ENR |= (1 << static_cast<unsigned int>(peripheral));
        }

        /**
         * @brief Enables the clock for a peripheral on the APB1 bus.
         * @param peripheral The target APB1 peripheral bit offset value.
         */
        void enableClock(APB1Peripheral peripheral) {
            this->APB1ENR |= (1 << static_cast<unsigned int>(peripheral));
        }
    };

    static_assert(sizeof(RCC) == 0x28);

    /**
     * @brief General Purpose Input/Output (GPIO) peripheral memory map.
     * @details Mapped to physical memory offsets specified in Section 9.2 of the RM0008 Reference Manual.
     */
    struct GPIO {
        volatile register_t CRL; ///< 0x00 Port Configuration Register Low (GPIOx_CRL) for pins 0 to 7.
        volatile register_t CRH; ///< 0x04 Port Configuration Register High (GPIOx_CRH) for pins 8 to 15.
        volatile register_t IDR; ///< 0x08 Port Input Data Register (GPIOx_IDR).
        volatile register_t ODR; ///< 0x0C Port Output Data Register (GPIOx_ODR).
        volatile register_t BSRR; ///< 0x10 Port Bit Set/Reset Register (GPIOx_BSRR).
        volatile register_t BRR; ///< 0x14 Port Bit Reset Register (GPIOx_BRR).
        volatile register_t LCKR; ///< 0x18 Port Configuration Lock Register (GPIOx_LCKR).
    /**
     * @brief Defines the 4-bit configuration for the GPIO MODE and CNF register bitfields.
     * @details Mapped to the CNF[1:0] and MODE[1:0] bitfields in the GPIOx_CRL and GPIOx_CRH registers.
     */
    enum class Mode: unsigned int {
        /* Input Modes */
        AnalogInput = 0b0000, ///< Analog input configuration (CNF=00, MODE=00).
        FloatingInput = 0b0100, ///< Floating input configuration (CNF=01, MODE=00).
        PullUpPullDown = 0b1000, ///< Input with pull-up / pull-down configuration (CNF=10, MODE=00).

        /* Output Modes */
        SlowGeneralPurposePushPull = 0b0010, ///< General purpose output push-pull, max speed 2 MHz (CNF=00, MODE=10).
        SlowGeneralPurposeOpenDrain = 0b0110, ///< General purpose output open-drain, max speed 2 MHz (CNF=01, MODE=10).
        SlowAlternatePushPull = 0b1010, ///< Alternate function output push-pull, max speed 2 MHz (CNF=10, MODE=10).
        SlowAlternateOpenDrain = 0b1110, ///< Alternate function output open-drain, max speed 2 MHz (CNF=11, MODE=10).

        MediumGeneralPurposePushPull = 0b0001, ///< General purpose output push-pull, max speed 10 MHz (CNF=00, MODE=01).
        MediumGeneralPurposeOpenDrain = 0b0101, ///< General purpose output open-drain, max speed 10 MHz (CNF=01, MODE=01).
        MediumAlternatePushPull = 0b1001, ///< Alternate function output push-pull, max speed 10 MHz (CNF=10, MODE=01).
        MediumAlternateOpenDrain = 0b1101, ///< Alternate function output open-drain, max speed 10 MHz (CNF=11, MODE=01).

        FastGeneralPurposePushPull = 0b0011, ///< General purpose output push-pull, max speed 50 MHz (CNF=00, MODE=11).
        FastGeneralPurposeOpenDrain = 0b0111, ///< General purpose output open-drain, max speed 50 MHz (CNF=01, MODE=11).
        FastAlternatePushPull = 0b1011, ///< Alternate function output push-pull, max speed 50 MHz (CNF=10, MODE=11).
        FastAlternateOpenDrain = 0b1111, ///< Alternate function output open-drain, max speed 50 MHz (CNF=11, MODE=11).
    };
        /**
         * @brief Configures a pin output state to HIGH (VDD) by writing to the BSRR.
         * @details Avoids read-modify-write race conditions by writing directly to BSRR.
         * @param bit_number The pin number (0 to 15) to set.
         */
        void setOutputBit(unsigned int bit_number) {
            this->BSRR = (1 << bit_number);
        }

        /**
         * @brief Configures a pin output state to LOW (VSS) by writing to the BRR.
         * @details Avoids read-modify-write race conditions by writing directly to BRR.
         * @param bit_number The pin number (0 to 15) to clear.
         */
        void clearOutputBit(unsigned int bit_number) {
            this->BRR = (1 << bit_number);
        }

        /**
         * @brief Configures the mode and pull-up/pull-down resistor state of a specific pin.
         * @details Updates configuration bits in GPIOx_CRL or GPIOx_CRH, and sets ODR
         * to configure the pull-up/pull-down state if PullUpPullDown mode is selected.
         * @param pin The target pin number (0 to 15).
         * @param mode The pin configuration mode (analog, floating, pull-up/down, or output modes).
         * @param pull Selects pull-up (Up) or pull-down (Down) resistor configuration. Only applies if mode is GPIOMode::PullUpPulLDown
         */
        void setPinMode(unsigned int pin, Mode mode, GPIOPullState pull = GPIOPullState::Up) {
            // 1. Calculate bit-shift configuration offsets based on pin number
            const unsigned int shift = (pin % 8) * 4;
            const auto mode_bits = static_cast<unsigned int>(mode);

            // 2. Write configuration bits to CRL or CRH register
            if (pin < 8) {
                this->CRL &= ~(0b1111u << shift); // Clear target configuration bits
                this->CRL |= (mode_bits << shift); // Set new configuration bits
            } else if (pin < 16) {
                this->CRH &= ~(0b1111u << shift);
                this->CRH |= (mode_bits << shift);
            }

            // 3. Configure the pull-up/pull-down resistor state via the ODR register
            if (mode == Mode::PullUpPullDown) {
                if (pull == GPIOPullState::Up) {
                    this->setOutputBit(pin); // Enable pull-up configuration
                } else {
                    this->clearOutputBit(pin); // Enable pull-down configuration
                }
            }
        }
    };

    static_assert(sizeof(GPIO) == 0x1C);


    /***
     * @brief Universal Synchronous Asynchronous Receiver Transmitter (USART) peripheral memory map.
     * @details Mapped to physical memory offsets specified in Section 27.6 of the RM0008 Reference Manual.
     */
    struct USART {
        volatile register_t SR; ///< 0x00 USART Status Register (USART_SR).
        volatile register_t DR; ///< 0x04 USART Data Register (USART_DR).
        volatile register_t BRR; ///< 0x08 USART Baud Rate Register (USART_BRR).
        volatile register_t CR1; ///< 0x0C USART Control Register 1 (USART_CR1).
        volatile register_t CR2; ///< 0x10 USART Control Register 2 (USART_CR2).
        volatile register_t CR3; ///< 0x14 USART Control Register 3 (USART_CR3).
        volatile register_t GTPR; ///< 0x18 USART Guard Time and Prescaler Register (USART_GTPR).

        /**
         * @brief Initializes USART parameters, baud rate division, and enables receiver and transmitter.
         * @param baud Selected transmission BaudRate enum.
         * @param peripheral_clock Frequency of the clock source feeding the USART peripheral.
         */
        void init(BaudRate baud, unsigned int peripheral_clock) {
            // 1. Convert BaudRate enum to integer value
            const auto baud_val = static_cast<unsigned int>(baud);

            // 2. Compute the baud rate division values per Section 27.3.4 of the RM0008 Reference Manual
            const unsigned int usartdiv = (peripheral_clock * 10) / (16 * baud_val);
            const unsigned int mantissa = usartdiv / 10;
            const unsigned int fraction = ((usartdiv % 10) * 16 + 5) / 10;

            this->BRR = (mantissa << 4) | (fraction & 0x0F);

            // 3. Set UE, TE, and RE bits in USART_CR1 register to enable peripheral operations
            this->CR1 = (1 << 13) | (1 << 3) | (1 << 2);
        }

        /**
         * @brief Reads a single received byte. Block until data register not empty.
         * @return Received character byte.
         */
        unsigned char readByte() const {
            while (!(this->SR & (1 << 5))); // Wait until RXNE (Read Data Register Not Empty) bit is set
            return static_cast<unsigned char>(this->DR & 0xFF);
        }

        /**
         * @brief Transmits a single byte. Blocks until transmission register empty.
         * @param data Byte value to transmit.
         */
        void transmit(uint8_t data) {
            while (!(this->SR & (1 << 7))); // Wait until TXE (Transmit Data Register Empty) bit is set
            this->DR = data;
        }

        /**
         * @brief Checks if received data is available to read.
         * @return True if RXNE flag is set, false otherwise.
         */
        bool hasData() const {
            return (this->SR & (1 << 5)); // Check RXNE flag status in USART_SR
        }

        /**
         * @brief Reads the byte currently in the data register without blocking.
         * @return Data register byte value.
         */
        uint8_t receive() const {
            return static_cast<uint8_t>(this->DR & 0xFF);
        }
    };

    struct I2C {
        volatile register_t CR1;
        volatile register_t CR2;
        volatile register_t OAR1;
        volatile register_t OAR2;
        volatile register_t DR;
        volatile register_t SR1;
        volatile register_t SR2;
        volatile register_t CCR;
        volatile register_t TRISE;
    };
    struct DMAChannel {
        volatile register_t CCR; //!< DMA channel x configuration register
        volatile register_t CNDTR; //!< DMA channel x number of data register
        volatile register_t CPAR; //!< DMA channel x peripheral address register
        volatile register_t CMAR; //!< DMA channel x memory address register
        register_t RESERVED; //!< Reserved for alignment. DO NOT WRITE TO IT (13.4.7 Register Map)

        enum class DMAPriorityLevel : unsigned char {
            LOW = 0b00,
            MEDIUM = 0b01,
            HIGH = 0b10,
            VERY_HIGH = 0b11
        };
        enum class DMAMemorySize : unsigned char {
            BYTE = 0b00,
            HALF_WORD = 0b01,
            WORD = 0b10,
        };
        enum class TransferDirection : unsigned char {
            FROM_PERIPHERAL_TO_MEMORY = 0b0,
            FROM_MEMORY_TO_PERIPHERAL = 0b1,
        };
        bool isEnabled() const {
            return CCR & 0b1;
        }
        void setPriorityLevel(DMAPriorityLevel level) {

            constexpr int bit_start = 12;
            // Clear last level
            CCR &= ~(0b11 << bit_start);
            // Set level
            CCR |= (static_cast<register_t>(level) << bit_start);
        }
        void setSize(DMAMemorySize memory_size, DMAMemorySize peripheral_size) {
            // set memory size
            constexpr int m_bit_start = 10;
            CCR &= ~(0b11 << m_bit_start);
            // Set level
            CCR |= (static_cast<register_t>(memory_size) << m_bit_start);

            // set peripheral size
            constexpr int p_bit_start = 8;
            CCR &= ~(0b11 << p_bit_start);
            // Set level
            CCR |= (static_cast<register_t>(peripheral_size) << p_bit_start);
        }
        void setPeripheralAddress(const uintptr_t* peripheral) {
            if (isEnabled())
                return;
            CPAR = reinterpret_cast<register_t>(peripheral);
        }
        void setMemoryAddress(const uintptr_t* memory) {
            if (isEnabled())
                return;
            CMAR = reinterpret_cast<register_t>(memory);
        }
        void setDataTransferMode(const TransferDirection dir ) {
            constexpr unsigned int bit_start = 4;
            CCR &= ~(0b1 << bit_start);
            CCR |= (static_cast<register_t>(dir) << bit_start);
        }
        void enableTransferCompleteInterrupt(bool enabled) {
            constexpr unsigned int bit_start = 1;
            CCR &= ~(0b1 << bit_start);
            CCR |= (static_cast<register_t>(enabled) << bit_start);
        }
    };

    struct DMA1MemoryMap {
        volatile register_t ISR;//!< DMA interrupt status register
        volatile register_t IFCR;//!< DMA interrupt flag clear register

        // Channel array (DMA1 has 7 channels)
        DMAChannel CH[7];
    };

    struct DMA2MemoryMap {
        volatile register_t ISR;//!< DMA interrupt status register
        volatile register_t IFCR;//!< DMA interrupt flag clear register
        // Channel array (DMA2 has 5 channels)
        DMAChannel CH[5];
    };

    // =========================================================================
    // Peripheral Base Addresses
    // =========================================================================
    inline const auto DMA1 = reinterpret_cast<DMA1MemoryMap *>(0x4002'0000u);
    inline const auto DMA2 = reinterpret_cast<DMA2MemoryMap *>(0x4002'0400u);
    inline const auto USART1 = reinterpret_cast<USART *>(0x40013800u);
    /** @brief General Purpose Timer 2 on the APB1 Bus (TIM2 base address). */
    inline const auto TIMER2 = reinterpret_cast<TIMER *>(0x40000000u);

    /** @brief General Purpose Timer 3 on the APB1 Bus (TIM3 base address). */
    inline const auto TIMER3 = reinterpret_cast<TIMER *>(0x40000400u);

    /** @brief General Purpose Timer 4 on the APB1 Bus (TIM4 base address). */
    inline const auto TIMER4 = reinterpret_cast<TIMER *>(0x40000800u);

    /** @brief Reset and Clock Control module on the AHB Bus (RCC base address). */
    inline const auto RCC1 = reinterpret_cast<RCC *>(0x40021000u);

    /** @brief GPIO Port A on the APB2 Bus (GPIOA base address). */
    inline const auto GPIOA = reinterpret_cast<GPIO *>(0x40010800u);

    /** @brief GPIO Port B on the APB2 Bus (GPIOB base address). */
    inline const auto GPIOB = reinterpret_cast<GPIO *>(0x40010C00u);

    /** @brief GPIO Port C on the APB2 Bus (GPIOC base address). */
    inline const auto GPIOC = reinterpret_cast<GPIO *>(0x40011000u);
}

#endif // WHEEL2FIRMWARE_MEMORYMAP_H
