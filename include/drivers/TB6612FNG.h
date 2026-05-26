#ifndef WHEEL2FIRMWARE_TB6612FNGCONTROLLER_H
#define WHEEL2FIRMWARE_TB6612FNGCONTROLLER_H

#include "MemoryMap.h"

namespace W2 {
    enum class MotorDirection {
        STOP = 0,
        FORWARD = 1,
        BACKWARD = -1
    };

    /**
     * @brief Structure representing the current operational status of a motor.
     */
    struct MotorStatus {
        MotorDirection direction = MotorDirection::FORWARD; ///< Current direction of rotation.
        unsigned int speed = 0u; ///< Raw speed duty cycle value (0 to MAX_PWM).
        int correction = 1000; ///< Fixed-point scaling factor (1000 represents a 1.0 calibration coefficient).
    };

    /**
     * @brief Structure mapping the peripheral pins connected to the motor controller.
     */
    struct MotorPins {
        GPIO *port; ///< GPIO port base address for direction control pins.
        unsigned int in1; ///< GPIO pin number for IN1 control signal.
        unsigned int in2; ///< GPIO pin number for IN2 control signal.
        TIMER *pwm_timer; ///< TIMER peripheral base address for PWM speed control.
        TimerChannel channel; ///< TIMER channel configured for PWM output.
    };

    /**
     * @brief Driver class for the TB6612FNG motor driver peripheral.
     */
    class TB6612FNG {
    public:
        static constexpr int MAX_PWM = 1000; ///< Maximum duty cycle limit value.

        /**
         * @brief Constructs a new TB6612FNG driver instance and configures control GPIOs and PWM timers.
         * @param portA_dir GPIO port base address for left motor direction control.
         * @param ain1 GPIO pin number for left motor IN1 signal.
         * @param ain2 GPIO pin number for left motor IN2 signal.
         * @param timerA TIMER peripheral base address for left motor PWM.
         * @param chA TIMER channel for left motor PWM.
         * @param portB_dir GPIO port base address for right motor direction control.
         * @param bin1 GPIO pin number for right motor IN1 signal.
         * @param bin2 GPIO pin number for right motor IN2 signal.
         * @param timerB TIMER peripheral base address for right motor PWM.
         * @param chB TIMER channel for right motor PWM.
         * @param stdby_port GPIO port base address for standby signal control.
         * @param stdby_pin GPIO pin number for standby signal.
         */
        TB6612FNG(
            GPIO *portA_dir, unsigned int ain1, unsigned int ain2, TIMER *timerA, TimerChannel chA,
            GPIO *portB_dir, unsigned int bin1, unsigned int bin2, TIMER *timerB, TimerChannel chB,
            GPIO *stdby_port, unsigned int stdby_pin);

        /**
         * @brief Sets calibration scaling factors for the motors.
         * @param left_motor_correction Scaling factor for the left motor speed (1000 = 1.0).
         * @param right_motor_correction Scaling factor for the right motor speed (1000 = 1.0).
         */
        void setCorrection(int left_motor_correction, int right_motor_correction);

        /**
         * @brief Configures target speed and direction parameters for the left motor.
         * @param speed Speed value within range [0, MAX_PWM].
         * @param direction Motor direction enum value.
         */
        void setLeftMotor(int speed, MotorDirection direction);

        /**
         * @brief Configures target speed and direction parameters for the right motor.
         * @param speed Speed value within range [0, MAX_PWM].
         * @param direction Motor direction enum value.
         */
        void setRightMotor(int speed, MotorDirection direction);

        /**
         * @brief Writes direction signals and PWM duty cycle values to peripheral registers.
         */
        void update() const;

        /**
         * @brief Computes control inputs for differential steering blending.
         * @param x Steering factor within range [-1000, 1000].
         * @param y Throttle factor within range [-1000, 1000].
         */
        void update(int x, int y);

    private:
        MotorPins m_left_motor_pins; ///< Hardware connection configuration for the left motor.
        MotorPins m_right_motor_pins; ///< Hardware connection configuration for the right motor.

        GPIO *m_stdby_port; ///< GPIO port base address for the standby signal.
        unsigned int m_stdby_pin; ///< GPIO pin number for the standby signal.

        MotorStatus m_left_motor_status; ///< Target operational status parameters for the left motor.
        MotorStatus m_right_motor_status; ///< Target operational status parameters for the right motor.

        int vLeft_cur = 0; ///< Blended current left target speed parameter.
        int vRight_cur = 0; ///< Blended current right target speed parameter.

        /**
         * @brief Updates peripheral registers to apply configured speed and direction.
         * @param pins Reference to MotorPins hardware layout.
         * @param status Reference to MotorStatus target values.
         */
        static void setMotorSignals(const MotorPins &pins, const MotorStatus &status);

        /**
         * @brief Clamps integer values to specified limits.
         * @param x Value to clamp.
         * @param a Minimum limit.
         * @param b Maximum limit.
         * @return Clamped value.
         */
        static int clamp(int x, int a, int b);

        /**
         * @brief Returns sign multiplier of the value.
         * @param x Value to evaluate.
         * @return 1 if positive, -1 if negative, 0 if zero.
         */
        static int sign(int x);
    };
}

#endif // WHEEL2FIRMWARE_TB6612FNGCONTROLLER_H