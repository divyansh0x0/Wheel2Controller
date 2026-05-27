#include "drivers/MemoryMap.h"
#include "drivers/TB6612FNG.h"

#define DEBUG
// Global instance of the TB6612FNG motor controller driver.
// Left Motor pins configuration: IN1=PA2, IN2=PA3, PWMA=PA1 (TIM2 Channel 2)
// Right Motor pins configuration: IN1=PA5, IN2=PA6, PWMB=PA7 (TIM3 Channel 2)
// Standby control pin configuration: STBY=PA4


/**
 * @brief Soft delay loop for timing control.
 * @param count Number of iterations to spin.
 */
void delay(unsigned int count) {
    for (volatile unsigned int i = 0; i < count; i++) {}
}

[[noreturn]] int main() {
    // 1. Enable peripheral clocks via Reset and Clock Control (RCC) registers
    W2::RCC1->enableClock(W2::APB2Peripheral::USART1);
    W2::RCC1->enableClock(W2::APB2Peripheral::GPIOA);
    W2::RCC1->enableClock(W2::APB1Peripheral::TIM2);
    W2::RCC1->enableClock(W2::APB1Peripheral::TIM3);

    // 2. Configure Timer Frequencies (72 MHz internal clock divided by 72 = 1 MHz timebase, ARR=1000 yields 1 kHz PWM frequency)
    W2::TIMER2->setFrequency(1000, 72);
    W2::TIMER3->setFrequency(1000, 72);

    // 3. Configure peripheral GPIO pins
    // Set timer output channels and USART1 TX to Alternate Function Push-Pull mode
    W2::GPIOA->setPinMode(1, W2::GPIOMode::FastAlternatePushPull); // TIM2 Channel 2 (PWMA)
    W2::GPIOA->setPinMode(7, W2::GPIOMode::FastAlternatePushPull); // TIM3 Channel 2 (PWMB)
    W2::GPIOA->setPinMode(9, W2::GPIOMode::FastAlternatePushPull); // USART1 TX
    W2::GPIOA->setPinMode(10, W2::GPIOMode::FloatingInput); // USART1 RX (Floating Input mode)
    // W2::USART1->init(); // Legacy parameterless init call
    W2::USART1->init(W2::BaudRate::Baud115200, 72000000);
    // 5. Enable the timer counters to start generating PWM signals
    W2::TIMER2->start();
    W2::TIMER3->start();
    W2::TB6612FNG robotBase(
        W2::GPIOA, 2, 3, W2::TIMER2, W2::TimerChannel::Channel2,
        W2::GPIOA, 5, 6, W2::TIMER3, W2::TimerChannel::Channel2,
        W2::GPIOA, 4
    );
    // 6. Main application control loop
    int i = 0;
    while (true) {
        // Drive forward decrementing duty cycle from 50%
        // Steering and throttle inputs mapped within range [-1000, 1000]
        robotBase.update(0, 500);
        // delay(100000); // 50 Hz control loop timing loop delay
        i++;
    }
}