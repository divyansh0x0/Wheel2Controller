/**
* @brief Cortex-M vector table.
 *
 * The vector table contains:
 * - The initial stack pointer value.
 * - Exception and interrupt handler addresses.
 *
 * On reset, the processor loads the initial stack pointer
 * from the first entry of the table and the reset handler
 * address from the second entry.
 *
 * Reference:
 * https://developer.arm.com/documentation/dui0552/a/the-cortex-m3-processor/exception-model/vector-table
 */
/**
 * @brief Function pointer type for C++ static constructor initializers.
 */
using init_func_t = void (*)();

/**
 * @brief Total length of the Interrupt Service Routine Vector (ISRV) table.
 */
constexpr unsigned int ISRV_LENGTH = 48;

/**
 * @brief Function pointer type for Interrupt Service Routines (ISRs).
 */
using isr_t = void (*)();

/**
 * @brief Initial Stack Pointer address defined by the linker script.
 */
extern "C" void _estack(void);

extern "C" unsigned int
        _sidata, ///< Start address of the initialization values for the .data section in Flash.
        _sdata,  ///< Start address of the .data section in SRAM.
        _edata,  ///< End address of the .data section in SRAM.
        _sbss,   ///< Start address of the .bss section in SRAM.
        _ebss;   ///< End address of the .bss section in SRAM.

extern int main(void);

extern "C" init_func_t _sinit; ///< Start address of the static constructor initializer list.
extern "C" init_func_t _einit; ///< End address of the static constructor initializer list.

/**
 * @brief Configure system clock to 72 MHz using the external 8 MHz HSE crystal.
 */
static void initSystemClock() {
    volatile auto* const FLASH_ACR = reinterpret_cast<volatile unsigned int*>(0x40022000u);
    volatile auto* const RCC_CR    = reinterpret_cast<volatile unsigned int*>(0x40021000u);
    volatile auto* const RCC_CFGR  = reinterpret_cast<volatile unsigned int*>(0x40021004u);

    // 1. Flash: 2 wait states + prefetch buffer enable
    *FLASH_ACR = (1u << 4u) | 2u;

    // 2. Enable HSE
    *RCC_CR |= (1u << 16u);
    while (!(*RCC_CR & (1u << 17u))) {}

    // 3. Configure PLL and bus prescalers (PLLMUL = x9, PLLSRC = HSE, PPRE1 = /2)
    *RCC_CFGR = (0b0111u << 18u) | (1u << 16u) | (0b100u << 8u);

    // 4. Enable PLL
    *RCC_CR |= (1u << 24u);
    while (!(*RCC_CR & (1u << 25u))) {}

    // 5. Select PLL as system clock source
    *RCC_CFGR |= 0b10u;
    while ((*RCC_CFGR & (0b11u << 2u)) != (0b10u << 2u)) {}
}

/**
 * @brief Reset handler called on processor reset.
 * @details Initializes the data segment in SRAM from Flash, clears the BSS segment,
 * executes static constructor initialization functions, and invokes the main program.
 */
extern "C" [[noreturn]] void Reset_Handler(void) {
    initSystemClock();

    unsigned int* src = &_sidata;
    unsigned int* dst = &_sdata;
    while (dst < &_edata) {
        *dst= *src;  // Copy initialization values from Flash to SRAM
        // Increment pointers to process subsequent words
        src++;
        dst++;
    }

    src = &_sbss;
    while (src < &_ebss) {
        *src = 0;
        src++;
    }

    init_func_t* src_func = &_sinit; // Pointer to the static constructor initializer list
    while (src_func < &_einit) {
        (*src_func)();
        src_func++;
    }
    main();
    while (1) {}
}


/**
 * @brief Default handler for unhandled exceptions and interrupts.
 * @details Provides an infinite loop fallback to halt execution for inspection.
 */
extern "C" void Default_Handler(void) {
    while (1) {
        // Halt execution to allow debugger attachment
    }
}

/**
 * @brief Hard fault exception handler.
 * @details Entered upon hardware execution errors such as bus, memory access, or usage faults.
 */
extern "C" void HardFault_Handler(void) {
    while (1) {
        // Halt execution to preserve CPU register state for GDB inspection
    }
}
__attribute__((section(".isr_vector"), used))
isr_t isr_vector_table[ISRV_LENGTH] = {
    _estack,             //  0: Initial Stack Pointer
    Reset_Handler,       //  1: Reset Vector
    Default_Handler,     //  2: NMI (Non-Maskable Interrupt)
    HardFault_Handler,   //  3: Hard Fault
    Default_Handler,     //  4: Memory Management Fault
    Default_Handler,     //  5: Bus Fault
    Default_Handler,     //  6: Usage Fault
    nullptr,             //  7: Reserved
    nullptr,             //  8: Reserved
    nullptr,             //  9: Reserved
    nullptr,             // 10: Reserved
    Default_Handler,     // 11: SVCall
    Default_Handler,     // 12: Debug Monitor
    nullptr,             // 13: Reserved
    Default_Handler,     // 14: PendSV
    Default_Handler,     // 15: SysTick
    // Device-specific interrupts (Interrupt Requests 0 to 31)
    // Configured to Default_Handler for fallback handling of unhandled interrupts.
    Default_Handler,     // 16: WWDG
    Default_Handler,     // 17: PVD
    Default_Handler,     // 18: TAMPER
    Default_Handler,     // 19: RTC
    Default_Handler,     // 20: FLASH
    Default_Handler,     // 21: RCC
    Default_Handler,     // 22: EXTI0
    Default_Handler,     // 23: EXTI1
    Default_Handler,     // 24: EXTI2
    Default_Handler,     // 25: EXTI3
    Default_Handler,     // 26: EXTI4
    Default_Handler,     // 27: DMA1_Channel1
    Default_Handler,     // 28: DMA1_Channel2
    Default_Handler,     // 29: DMA1_Channel3
    Default_Handler,     // 30: DMA1_Channel4
    Default_Handler,     // 31: DMA1_Channel5
    Default_Handler,     // 32: DMA1_Channel6
    Default_Handler,     // 33: DMA1_Channel7
    Default_Handler,     // 34: ADC1_2
    Default_Handler,     // 35: USB_HP_CAN_TX
    Default_Handler,     // 36: USB_LP_CAN_RX0
    Default_Handler,     // 37: CAN_RX1
    Default_Handler,     // 38: CAN_SCE
    Default_Handler,     // 39: EXTI9_5
    Default_Handler,     // 40: TIM1_BRK
    Default_Handler,     // 41: TIM1_UP
    Default_Handler,     // 42: TIM1_TRG_COM
    Default_Handler,     // 43: TIM1_CC
    Default_Handler,     // 44: TIM2
    Default_Handler,     // 45: TIM3
    Default_Handler,     // 46: TIM4
    Default_Handler,     // 47: I2C1_EV
};
