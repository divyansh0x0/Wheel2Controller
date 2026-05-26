#include "drivers/MemoryMap.h"
#include "drivers/TB6612FNG.h"

#define DEBUG
// Define our global motor controller
// Left: IN1=PA2, IN2=PA3, PWMA=PA1 (TIM2 CH2)
// Right: IN1=PA5, IN2=PA6, PWMB=PA7 (TIM3 CH2)
// STBY: PA4


// Simple loop delay (approx 0.02s / 50Hz)
void delay(unsigned int count) {
    for (volatile unsigned int i = 0; i < count; i++) {}
}

[[noreturn]] int main() {
    // 1. Enable Clocks (The "Holy Trinity")
    W2::RCC1->enableClock(W2::APB2Peripheral::GPIOA);
    W2::RCC1->enableClock(W2::APB1Peripheral::TIM2);
    W2::RCC1->enableClock(W2::APB1Peripheral::TIM3);

    // 2. Configure Timer Frequencies (72MHz / 72 = 1MHz, ARR=1000 -> 1kHz PWM)
    W2::TIMER2->setFrequency(72, 1000);
    W2::TIMER3->setFrequency(72, 1000);

    // 3. Configure PWM pins to Alternate Function mode
    // These must be set to AF Push-Pull so the Timers can override the GPIO output
    W2::GPIOA->setPinMode(1, W2::GPIOMode::FastAlternatePushPull); // PWMA
    W2::GPIOA->setPinMode(7, W2::GPIOMode::FastAlternatePushPull); // PWMB

    // 5. Start the Timers
    W2::TIMER2->start();
    W2::TIMER3->start();
    W2::TB6612FNG robotBase(
        W2::GPIOA, 2, 3, W2::TIMER2, W2::TimerChannel::Channel2,
        W2::GPIOA, 5, 6, W2::TIMER3, W2::TimerChannel::Channel2,
        W2::GPIOA, 4
    );
    // 6. Main Control Loop
    int i = 0;
    while (true) {
        // Example: drive at half speed forward (500/1000 = 50%)
        // Joystick inputs should be mapped to the range [-1000, 1000]
        robotBase.update(0, 500 - i);
        if (i >= 500) {
            i = 0;
        }
        delay(100000); // 50Hz control loop timing
        i++;
    }
}