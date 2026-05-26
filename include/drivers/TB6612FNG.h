#ifndef WHEEL2FIRMWARE_TB6612FNGCONTROLLER_H
#define WHEEL2FIRMWARE_TB6612FNGCONTROLLER_H

#include "MemoryMap.h"

namespace W2 {
    enum class MotorDirection {
        STOP = 0,
        FORWARD = 1,
        BACKWARD = -1
    };

    struct MotorStatus {
        MotorDirection direction = MotorDirection::FORWARD;
        unsigned int speed = 0u;
        // Fixed-point multiplier: 1000 represents 1.0x calibration
        int correction = 1000;
    };

    struct MotorPins {
        GPIO *port;
        unsigned int in1;
        unsigned int in2;
        TIMER *pwm_timer;
        TimerChannel channel;
    };

    class TB6612FNG {
    public:
        static constexpr int MAX_PWM = 1000;

        TB6612FNG(
            GPIO *portA_dir, unsigned int ain1, unsigned int ain2, TIMER *timerA, TimerChannel chA,
            GPIO *portB_dir, unsigned int bin1, unsigned int bin2, TIMER *timerB, TimerChannel chB,
            GPIO *stdby_port, unsigned int stdby_pin);

        /**
         * @brief Sets drift correction. 1000 = 1.0 multiplier.
         */
        void setCorrection(int left_motor_correction, int right_motor_correction);

        void setLeftMotor(int speed, MotorDirection direction);

        void setRightMotor(int speed, MotorDirection direction);

        void update() const;

        /**
         * @brief Calculates differential steering blending from joystick inputs.
         * @param x Steering (-1000 to 1000)
         * @param y Throttle (-1000 to 1000)
         */
        void update(int x, int y);

    private:
        MotorPins m_left_motor_pins;
        MotorPins m_right_motor_pins;

        GPIO *m_stdby_port;
        unsigned int m_stdby_pin;

        MotorStatus m_left_motor_status;
        MotorStatus m_right_motor_status;

        int vLeft_cur = 0;
        int vRight_cur = 0;

        static void setMotorSignals(const MotorPins &pins, const MotorStatus &status);
        static int clamp(int x, int a, int b);
        static int sign(int x);
    };
}

#endif // WHEEL2FIRMWARE_TB6612FNGCONTROLLER_H