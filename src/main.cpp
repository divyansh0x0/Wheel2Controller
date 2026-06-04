#include "drivers/MemoryMap.h"
#include "drivers/TB6612FNG.h"

#define DEBUG
// Global instance of the TB6612FNG motor controller driver.
// Left Motor pins configuration: IN1=PA2, IN2=PA3, PWMA=PA1 (TIM2 Channel 2)
// Right Motor pins configuration: IN1=PA5, IN2=PA6, PWMB=PA7 (TIM3 Channel 2)
// Standby control pin configuration: STBY=PA4
#define ARR 1000

/**
 * @brief Soft delay loop for timing control.
 * @param count Number of iterations to spin.
 */
void delay(unsigned int count) {
    for (volatile unsigned int i = 0; i < count; i++) {}
}

[[noreturn]] int main() {
    // 1. Enable peripheral clocks via Reset and Clock Control (RCC) registers
    STM32::MemoryMap::RCC1->enableClock(STM32::APB2Peripheral::AlternateFunctionIO);
    STM32::RCC1->enableClock(STM32::APB2Peripheral::USART1);
    STM32::RCC1->enableClock(STM32::APB2Peripheral::GPIOA);
    STM32::RCC1->enableClock(STM32::APB1Peripheral::TIM2);
    STM32::RCC1->enableClock(STM32::APB1Peripheral::TIM3);

    // Read back RCC enable registers to ensure clock stabilization before accessing registers
    volatile unsigned int rcc_apb2 = STM32::RCC1->APB2ENR;
    volatile unsigned int rcc_apb1 = STM32::RCC1->APB1ENR;
    (void)rcc_apb2;
    (void)rcc_apb1;

    // 2. Configure Timer Frequencies (72 MHz internal clock divided by 72 = 1 MHz timebase, ARR=1000 yields 1 kHz PWM frequency)
    STM32::TIMER2->setFrequency(20000, ARR);
    STM32::TIMER3->setFrequency(20000, ARR);

    // 3. Configure peripheral GPIO pins
    // Set timer output channels and USART1 TX to Alternate Function Push-Pull mode
    STM32::GPIOA->setPinMode(1, STM32::GPIOMode::FastAlternatePushPull); // TIM2 Channel 2 (PWMA)
    STM32::GPIOA->setPinMode(7, STM32::GPIOMode::FastAlternatePushPull); // TIM3 Channel 2 (PWMB)
    STM32::GPIOA->setPinMode(9, STM32::GPIOMode::FastAlternatePushPull); // USART1 TX
    STM32::GPIOA->setPinMode(10, STM32::GPIOMode::FloatingInput); // USART1 RX (Floating Input mode)
    // W2::USART1->init(); // Legacy parameterless init call
    STM32::USART1->init(STM32::BaudRate::Baud115200, 72000000);

    // 4. Construct motor driver instance (sets direction and standby pins)
    STM32::TB6612FNG robotBase(
        STM32::GPIOA, 2, 3, STM32::TIMER2, STM32::TimerChannel::Channel2,
        STM32::GPIOA, 6, 5, STM32::TIMER3, STM32::TimerChannel::Channel2,
        STM32::GPIOA, 4
    );

    // 5. Enable the timer counters to start generating PWM signals (after configuration is complete)
    STM32::TIMER2->start();
    STM32::TIMER3->start();

    // 6. Main application control loop
    int i = 0;
    while (true) {
        // Drive forward decrementing duty cycle from 50%
        // Steering and throttle inputs mapped within range [-1000, 1000]
        robotBase.update(0, 500);
        i++;
    }
}