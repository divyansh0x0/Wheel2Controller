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
//type for constructors of c++ objects
using init_func_t = void (*)();
// Interrupt service routine vector (ISRV) length
constexpr unsigned int ISRV_LENGTH = 48;

// Interrupt service routine vector stores function pointers which point to functions which return void and take void arguments
using isr_t = void (*)();
// Trick C++ into thinking this memory address is a function so we can avoid casting it!
extern "C" void _estack(void);

extern "C" unsigned int
        _sidata, // Store the uint at source address of .data in Flash
        _sdata, // stores the uint at start address in SRAM
        _edata,// stores the uint Destination end address in SRAM
        _sbss, // start of block started by symbol (bss)
        _ebss; // end of block started by symbol (bss)

extern int main(void);

extern "C" init_func_t _sinit; //  first c++ initializer functions
extern "C" init_func_t _einit; // last c++ initializer function

extern "C" [[noreturn]] void Reset_Handler(void) {
    unsigned int* src = &_sidata;
    unsigned int* dst = &_sdata;
    while (dst < &_edata) {
        *dst= *src;  //then write src in dst
        // increment src and increment dst pointers to next location
        src++;
        dst++;
    }

    src = &_sbss;
    while (src < &_ebss) {
        *src = 0;
        src++;
    }

    init_func_t* src_func = &_sinit; // pointer to the first initializer function
    while (src_func < &_einit) {
        (*src_func)();
        src_func++;
    }
    main();
    while (1) {}
}


// A generic catch-all for any interrupt we haven't written a specific handler for
extern "C" void Default_Handler(void) {
    while (1) {
        // Spin infinitely so we can catch it with a debugger
    }
}

// The specific handler for fatal system crashes
extern "C" void HardFault_Handler(void) {
    while (1) {
        // If we end up here, our code did something highly illegal (like dereferencing a bad pointer).
        // Spin infinitely so GDB can inspect the scene of the crime.
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
    // IRQ 0-31 — STM32F103 device-specific interrupts
    // All default to Default_Handler so unhandled IRQs spin safely
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
