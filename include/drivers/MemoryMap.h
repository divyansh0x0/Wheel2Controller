/**
 * @file MemoryMap.h
 * @brief Bare-metal memory map and zero-cost hardware abstractions for the STM32F103.
 * @details This file maps the physical silicon registers to C++ structs and provides 
 * inline hardware manipulation functions to ensure atomic, safe register access.
 */

#ifndef WHEEL2FIRMWARE_MEMORYMAP_H
#define WHEEL2FIRMWARE_MEMORYMAP_H

#include <cstdint>

namespace W2 {
    /**
     * @brief Determines the internal resistor connection for an input pin.
     */
    enum class GPIOPullState {
        Down = 0, ///< Connects the internal resistor to Ground (0V).
        Up = 1 ///< Connects the internal resistor to VDD (3.3V).
    };

    /**
         * @brief Identifies peripherals connected to the High-Speed APB2 Bus.
         * @details Mapped to specific bit positions in the RCC_APB2ENR register.
         */
    enum class APB2Peripheral : unsigned int {
        AlternateFunctionIO = 0, ///< AFIO: Required for external interrupts and some pin remapping.
        GPIOA = 2, ///< IO Port A
        GPIOB = 3, ///< IO Port B
        GPIOC = 4 ///< IO Port C (Often contains the built-in LED)
    };

    /**
     * @brief Identifies peripherals connected to the Standard-Speed APB1 Bus.
     * @details Mapped to specific bit positions in the RCC_APB1ENR register.
     */
    enum class APB1Peripheral : unsigned int {
        TIM2 = 0, ///< General Purpose Timer 2
        TIM3 = 1, ///< General Purpose Timer 3
        TIM4 = 2 ///< General Purpose Timer 4
    };

    /**
     * @brief Defines the exact 4-bit configuration for the GPIO MODE and CNF registers.
     * @details These binary values map directly to the hardware multiplexers in the STM32,
     * controlling the pin's physical direction, slew rate, and internal routing.
     */
    enum class GPIOMode: unsigned int {
        /* Input Modes */
        AnalogInput = 0b0000, ///< Bypasses the digital Schmitt trigger; used for ADC.
        FloatingInput = 0b0100, ///< Standard digital input; disconnected from internal resistors.
        PullUpPullDown = 0b1000, ///< Digital input; utilizes the ODR to activate internal resistors.

        /* Output Modes */
        SlowGeneralPurposePushPull = 0b0010, ///< Output at 2MHz max; actively drives High and Low.
        SlowGeneralPurposeOpenDrain = 0b0110, ///< Output at 2MHz max; drives Low, floats High (I2C style).
        SlowAlternatePushPull = 0b1010, ///< Output at 2MHz max; driven by internal hardware (e.g., Timer).
        SlowAlternateOpenDrain = 0b1110, ///< Output at 2MHz max; driven by hardware, floats High.

        MediumGeneralPurposePushPull = 0b0001, ///< Output at 10MHz max; actively drives High and Low.
        MediumGeneralPurposeOpenDrain = 0b0101, ///< Output at 10MHz max; drives Low, floats High.
        MediumAlternatePushPull = 0b1001, ///< Output at 10MHz max; driven by internal hardware.
        MediumAlternateOpenDrain = 0b1101, ///< Output at 10MHz max; driven by hardware, floats High.

        FastGeneralPurposePushPull = 0b0011, ///< Output at 50MHz max; actively drives High and Low.
        FastGeneralPurposeOpenDrain = 0b0111, ///< Output at 50MHz max; drives Low, floats High.
        FastAlternatePushPull = 0b1011, ///< Output at 50MHz max; driven by internal hardware.
        FastAlternateOpenDrain = 0b1111, ///< Output at 50MHz max; driven by hardware, floats High.
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
     * @brief General-purpose timer peripheral memory layout.
     * @details Represents the hardware registers for TIM2, TIM3, and TIM4.
     * Offsets map exactly to Section 15.4 of the RM0008 Reference Manual.
     * The 32-bit peripheral registers have to be written by words (32 bits). All other peripheral
       registers have to be written by half-words (16 bits) or words (32 bits). Read accesses can be
       done by bytes (8 bits), half-words (16 bits) or words (32 bits)
     */
    struct TIMER {
        volatile register_t CR1; ///< 0x00 Control Register 1: Main timer configuration.
        volatile register_t CR2; ///< 0x04 Control Register 2: Advanced timer configuration.
        volatile register_t SMCR; ///< 0x08 Slave Mode Control Register: Synchronization and chaining.
        volatile register_t DIER; ///< 0x0C DMA/Interrupt Enable Register: Triggers interrupts.
        volatile register_t SR; ///< 0x10 Status Register: Hardware flags (e.g., update events).
        volatile register_t EGR; ///< 0x14 Event Generation Register: Force software events.
        volatile register_t CCMR1; ///< 0x18 Capture/Compare Mode Register 1: Configures Channels 1 and 2.
        volatile register_t CCMR2; ///< 0x1C Capture/Compare Mode Register 2: Configures Channels 3 and 4.
        volatile register_t CCER; ///< 0x20 Capture/Compare Enable Register: Connects channels to physical pins.
        volatile register_t CNT; ///< 0x24 Counter Value: The live, running timer clock tick.
        volatile register_t PSC; ///< 0x28 Prescaler: Divides the master clock frequency.
        volatile register_t ARR; ///< 0x2C Auto-Reload Register: Defines the PWM period (max counter value).
        volatile register_t RCR; ///< 0x30 Repetition Counter Register: (Unused in standard timers, keeps alignment).
        volatile register_t CCR1; ///< 0x34 Capture/Compare Register 1: Duty cycle limit for Channel 1.
        volatile register_t CCR2; ///< 0x38 Capture/Compare Register 2: Duty cycle limit for Channel 2.
        volatile register_t CCR3; ///< 0x3C Capture/Compare Register 3: Duty cycle limit for Channel 3.
        volatile register_t CCR4; ///< 0x40 Capture/Compare Register 4: Duty cycle limit for Channel 4.
        /**
                 * @brief Configures the base frequency of the timer.
                 * @param prescaler Divides the master clock (e.g., 72MHz / 72 = 1MHz).
                 * @param auto_reload The maximum counter value before resetting (Period).
                 */
        void setFrequency(unsigned int auto_reload, unsigned int prescaler = 1) {
            this->PSC = prescaler - 1; // Hardware is 0-indexed
            this->ARR = auto_reload - 1; // Hardware is 0-indexed
            constexpr unsigned int ARE_bit_number = 7;
            this->CR1 |= (1 << ARE_bit_number); // Enable Auto-Reload Preload (ARPE) for stable PWM
        }

        /**
         * @brief Configures a specific channel for PWM Mode 1 and enables its output.
         * @param channel The target timer channel (1 to 4).
         */
        /**
         * @brief Configures a specific channel for PWM Mode 1 and enables its output.
         * @param channel The target timer channel (1 to 4).
         */
        void enablePWM(TimerChannel channel) {
            switch (channel) {
                case TimerChannel::Channel1:
                    this->CCMR1 &= ~(0b11 << 0);  // Clear CC1S (Bits 1:0) -> Force Output Mode
                    this->CCMR1 &= ~(0b111 << 4); // Clear OC1M (Bits 6:4)
                    this->CCMR1 |= (0b110 << 4);  // Set OC1M to PWM Mode 1 (110)
                    this->CCMR1 |= (1 << 3);      // Enable Preload (OC1PE)
                    this->CCER  |= (1 << 0);      // Enable Output on Channel 1
                    break;
                case TimerChannel::Channel2:
                    this->CCMR1 &= ~(0b11 << 8);  // Clear CC2S (Bits 9:8) -> Force Output Mode
                    this->CCMR1 &= ~(0b111 << 12);// Clear OC2M (Bits 14:12)
                    this->CCMR1 |= (0b110 << 12); // Set OC2M to PWM Mode 1 (110)
                    this->CCMR1 |= (1 << 11);     // Enable Preload (OC2PE)
                    this->CCER  |= (1 << 4);      // Enable Output on Channel 2
                    break;
                case TimerChannel::Channel3:
                    this->CCMR2 &= ~(0b11 << 0);  // Clear CC3S (Bits 1:0) -> Force Output Mode
                    this->CCMR2 &= ~(0b111 << 4); // Clear OC3M (Bits 6:4)
                    this->CCMR2 |= (0b110 << 4);  // Set OC3M to PWM Mode 1 (110)
                    this->CCMR2 |= (1 << 3);      // Enable Preload (OC3PE)
                    this->CCER  |= (1 << 8);      // Enable Output on Channel 3
                    break;
                case TimerChannel::Channel4:
                    this->CCMR2 &= ~(0b11 << 8);  // Clear CC4S (Bits 9:8) -> Force Output Mode
                    this->CCMR2 &= ~(0b111 << 12);// Clear OC4M (Bits 14:12)
                    this->CCMR2 |= (0b110 << 12); // Set OC4M to PWM Mode 1 (110)
                    this->CCMR2 |= (1 << 11);     // Enable Preload (OC4PE)
                    this->CCER  |= (1 << 12);     // Enable Output on Channel 4
                    break;
            }
        }
        /**
         * @brief Updates the duty cycle for a specific channel.
         * @param channel The target timer channel.
         * @param value The active pulse width (must be <= ARR).
         */
        void setDutyCycle(TimerChannel channel, unsigned int value) {
            switch (channel) {
                case TimerChannel::Channel1: this->CCR1 = value; break;
                case TimerChannel::Channel2: this->CCR2 = value; break;
                case TimerChannel::Channel3: this->CCR3 = value; break;
                case TimerChannel::Channel4: this->CCR4 = value; break;
            }
        }

        /**
         * @brief Turns on the timer, starting the counter.
         */
        void start() {
            this->CR1 |= (1 << 0); // Set CEN (Counter Enable) bit
        }
    };

    static_assert(sizeof(TIMER) == 0x44);

    /**
     * @brief Reset and Clock Control (RCC) peripheral memory layout.
     * @details The RCC acts as the central gatekeeper for power and clock distribution
     * across the entire STM32 die. Peripherals must be enabled here before use.
     */
    struct RCC {
        volatile register_t CR; ///< 0x00 Clock Control Register: Manages main oscillators (HSE, HSI).
        volatile register_t CFGR; ///< 0x04 Clock Configuration Register: Manages system clock muxing.
        volatile register_t CIR; ///< 0x08 Clock Interrupt Register.
        volatile register_t APB2RSTR; ///< 0x0C APB2 Peripheral Reset Register.
        volatile register_t APB1RSTR; ///< 0x10 APB1 Peripheral Reset Register.
        volatile register_t AHBENR; ///< 0x14 AHB Peripheral Clock Enable: Enables high-speed modules like DMA.
        volatile register_t APB2ENR; ///< 0x18 APB2 Peripheral Clock Enable: Enables GPIO ports and ADC.
        volatile register_t APB1ENR; ///< 0x1C APB1 Peripheral Clock Enable: Enables standard timers (TIM2/3/4).
        volatile register_t BDCR; ///< 0x20 Backup Domain Control Register.
        volatile register_t CSR; ///< 0x24 Control/Status Register.
        /**
         * @brief Enables the clock for a high-speed peripheral on the APB2 bus.
         * @param peripheral The target APB2 peripheral (e.g., Alternate Function IO,GPIOA, GPIOB, GPIOC).
         */
        void enableClock(APB2Peripheral peripheral) {
            this->APB2ENR |= (1 << static_cast<unsigned int>(peripheral));
        }

        /**
         * @brief Enables the clock for a standard-speed peripheral on the APB1 bus.
         * @param peripheral The target APB1 peripheral (e.g., TIM2, TIM3, TIM4).
         */
        void enableClock(APB1Peripheral peripheral) {
            this->APB1ENR |= (1 << static_cast<unsigned int>(peripheral));
        }
    };

    static_assert(sizeof(RCC) == 0x28);

    /**
     * @brief General Purpose Input/Output (GPIO) peripheral memory layout.
     * @details Provides a zero-cost abstraction for configuring and driving external pins.
     */
    struct GPIO {
        volatile register_t CRL; ///< 0x00 Configuration Low: Sets the mode for Pins 0 to 7.
        volatile register_t CRH; ///< 0x04 Configuration High: Sets the mode for Pins 8 to 15.
        volatile register_t IDR; ///< 0x08 Input Data Register: Reads the current physical state of the pins.
        volatile register_t ODR; ///< 0x0C Output Data Register: Dictates the output state or pull-resistor state.
        volatile register_t BSRR; ///< 0x10 Bit Set/Reset Register: Atomically sets ODR bits without reading.
        volatile register_t BRR; ///< 0x14 Bit Reset Register: Atomically clears ODR bits without reading.
        volatile register_t LCKR; ///< 0x18 Lock Register: Locks pin configurations to prevent accidental changes.

        /**
         * @brief Atomically sets a specific output pin to HIGH (3.3V).
         * @note Utilizes the BSRR hardware to bypass read-modify-write race conditions.
         * Bits written as 0 are completely ignored by the hardware.
         * @param bit_number The pin number (0 to 15) to set.
         */
        void setOutputBit(unsigned int bit_number) {
            this->BSRR = (1 << bit_number);
        }

        /**
         * @brief Atomically clears a specific output pin to LOW (Ground).
         * @note Utilizes the BRR hardware to bypass read-modify-write race conditions.
         * Bits written as 0 are completely ignored by the hardware.
         * @param bit_number The pin number (0 to 15) to clear.
         */
        void clearOutputBit(unsigned int bit_number) {
            this->BRR = (1 << bit_number);
        }

        /**
         * @brief Configures the hardware mode, speed, and pull state of a specific pin.
         * @details Safely masks and updates the CRL or CRH register, and handles the
         * hardware quirk of activating pull-resistors via the ODR.
         * @param pin The target pin number (0 to 15).
         * @param mode The target GPIOMode (e.g., Push-Pull, Analog, Floating).
         * @param pull The target pull resistor state (defaults to Pull-Up). Ignored if not in PullUpPullDown mode.
         */
        void setPinMode(unsigned int pin, GPIOMode mode, GPIOPullState pull = GPIOPullState::Up) {
            // 1. Calculate bit-shift offsets based on pin number
            const unsigned int shift = (pin % 8) * 4;
            const auto mode_bits = static_cast<unsigned int>(mode);

            // 2. Safely apply the 4-bit configuration into the correct register
            if (pin < 8) {
                this->CRL &= ~(0b1111 << shift); // Clear the 4 target bits
                this->CRL |= (mode_bits << shift); // Drop in our new enum value
            } else if (pin < 16) {
                this->CRH &= ~(0b1111 << shift);
                this->CRH |= (mode_bits << shift);
            }

            // 3. Address the internal resistor connection quirk via the ODR
            if (mode == GPIOMode::PullUpPullDown) {
                if (pull == GPIOPullState::Up) {
                    this->setOutputBit(pin); // Anchors pin to 3.3V via internal pull-up
                } else {
                    this->clearOutputBit(pin); // Anchors pin to Ground via internal pull-down
                }
            }
        }
    };

    static_assert(sizeof(GPIO) == 0x1C);

    // =========================================================================
    // Peripheral Base Addresses (Mapped to Silicon Memory Boundaries)
    // =========================================================================

    /** @brief General Purpose Timer 2 on the APB1 Bus. */
    inline const auto TIMER2 = reinterpret_cast<TIMER *>(0x40000000u);

    /** @brief General Purpose Timer 3 on the APB1 Bus. */
    inline const auto TIMER3 = reinterpret_cast<TIMER *>(0x40000400u);

    /** @brief General Purpose Timer 4 on the APB1 Bus. */
    inline const auto TIMER4 = reinterpret_cast<TIMER *>(0x40000800u);

    /** @brief Reset and Clock Control module on the AHB Bus. */
    inline const auto RCC1 = reinterpret_cast<RCC *>(0x40021000u);

    /** @brief GPIO Port A on the APB2 Bus. */
    inline const auto GPIOA = reinterpret_cast<GPIO *>(0x40010800u);

    /** @brief GPIO Port B on the APB2 Bus. */
    inline const auto GPIOB = reinterpret_cast<GPIO *>(0x40010C00u);

    /** @brief GPIO Port C on the APB2 Bus. */
    inline const auto GPIOC = reinterpret_cast<GPIO *>(0x40011000u);
}

#endif // WHEEL2FIRMWARE_MEMORYMAP_H
