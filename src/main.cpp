// 1. The Delay function stays outside
void delay(int count) {
    for (volatile int i = 0; i < count; i++) {
        // Do nothing
    }
}

int main() {
    // 2. MOVE THESE INSIDE MAIN!
    // Now they are initialized safely on the stack as local variables.
    volatile unsigned int* RCC_APB2ENR = (volatile unsigned int*)(0x40021000u + 0x18u);
    volatile unsigned int* GPIOC_CRH   = (volatile unsigned int*)(0x40011004u);
    volatile unsigned int* GPIOC_ODR   = (volatile unsigned int*)(0x4001100Cu);

    // 3. Turn on the clock for Port C (Bit 4)
    *RCC_APB2ENR |= (1 << 4);

    // 4. Configure Pin 13 as Output Push-Pull (Bits 20-23)
    *GPIOC_CRH &= ~(15 << 20); // Clear the 4 bits for Pin 13
    *GPIOC_CRH |=  (3 << 20);  // Set to '0011' (Output 50MHz)

    // 5. The Infinite Loop
    while(1) {
        // Pull PC13 LOW (0) -> LED ON
        *GPIOC_ODR &= ~(1 << 13);
        delay(500000);

        // Push PC13 HIGH (1) -> LED OFF
        *GPIOC_ODR |= (1 << 13);
        delay(500000);
    }

    return 0;
}