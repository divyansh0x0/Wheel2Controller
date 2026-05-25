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
extern "C" unsigned int
        _estack, //contains value at the end of stack
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




// The ISRV Vector
isr_t isr_vector_table[ISRV_LENGTH] __attribute__((section(".isr_vector"))) = {
    reinterpret_cast<isr_t>(&_estack),
    Reset_Handler
};