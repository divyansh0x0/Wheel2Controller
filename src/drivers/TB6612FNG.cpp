//
// Created by divyansh on 5/26/26.
//

#include "drivers/TB6612FNG.h"

#include <cstdlib>

static W2::MotorDirection getDirection(int val) {
    return val >= 0
               ? W2::MotorDirection::FORWARD
               : W2::MotorDirection::BACKWARD;
}

namespace W2 {

    TB6612FNG::TB6612FNG(
        GPIO* portA_dir, unsigned int ain1, unsigned int ain2, TIMER* timerA, TimerChannel chA,
        GPIO* portB_dir, unsigned int bin1, unsigned int bin2, TIMER* timerB, TimerChannel chB,
        GPIO* stdby_port, unsigned int stdby_pin) :
        m_left_motor_pins{portA_dir, ain1, ain2, timerA, chA},
        m_right_motor_pins{portB_dir, bin1, bin2, timerB, chB}
    {
        m_stdby_port = stdby_port;
        m_stdby_pin = stdby_pin;

        // Note: correction defaults to 1000 (1.0x)
        m_left_motor_status  = {MotorDirection::STOP, 0, 1000};
        m_right_motor_status = {MotorDirection::STOP, 0, 1000};

        m_left_motor_pins.port->setPinMode(ain1, GPIOMode::MediumGeneralPurposePushPull);
        m_left_motor_pins.port->setPinMode(ain2, GPIOMode::MediumGeneralPurposePushPull);

        m_right_motor_pins.port->setPinMode(bin1, GPIOMode::MediumGeneralPurposePushPull);
        m_right_motor_pins.port->setPinMode(bin2, GPIOMode::MediumGeneralPurposePushPull);

        m_stdby_port->setPinMode(stdby_pin, GPIOMode::MediumGeneralPurposePushPull);

        m_left_motor_pins.pwm_timer->enablePWM(m_left_motor_pins.channel);
        m_right_motor_pins.pwm_timer->enablePWM(m_right_motor_pins.channel);

        m_stdby_port->setOutputBit(m_stdby_pin);
    }

    void TB6612FNG::setMotorSignals(const MotorPins& pins, const MotorStatus& status) {
        switch (status.direction) {
            case MotorDirection::STOP:
                pins.port->clearOutputBit(pins.in1);
                pins.port->clearOutputBit(pins.in2);
                break;
            case MotorDirection::BACKWARD:
                pins.port->clearOutputBit(pins.in1);
                pins.port->setOutputBit(pins.in2);
                break;
            case MotorDirection::FORWARD:
                pins.port->setOutputBit(pins.in1);
                pins.port->clearOutputBit(pins.in2);
                break;
        }

        // Apply fixed-point correction (e.g., speed * 1000 / 1000)
        unsigned int final_speed = (status.correction * status.speed) / 1000;
        pins.pwm_timer->setDutyCycle(pins.channel, final_speed);
    }

    void TB6612FNG::setLeftMotor(const int speed, const MotorDirection direction) {
        m_left_motor_status.direction = direction;
        m_left_motor_status.speed = clamp(speed, 0, MAX_PWM);
    }

    void TB6612FNG::setRightMotor(const int speed, const MotorDirection direction) {
        m_right_motor_status.direction = direction;
        m_right_motor_status.speed = clamp(speed, 0, MAX_PWM);
    }

    void TB6612FNG::update() const {
        setMotorSignals(m_left_motor_pins, m_left_motor_status);
        setMotorSignals(m_right_motor_pins, m_right_motor_status);
    }

    void TB6612FNG::setCorrection(int left_motor_correction, int right_motor_correction) {
        m_left_motor_status.correction = left_motor_correction;
        m_right_motor_status.correction = right_motor_correction;
    }

    int TB6612FNG::clamp(int x, int a, int b) {
        if (x < a) return a;
        if (x > b) return b;
        return x;
    }

    int TB6612FNG::sign(int x) {
        if (x > 0) return 1;
        if (x < 0) return -1;
        return 0;
    }

    void TB6612FNG::update(int x, int y) {
        // assume x,y ∈ [-1000, 1000]
        x = clamp(x, -MAX_PWM, MAX_PWM);
        y = clamp(y, -MAX_PWM, MAX_PWM);

        int vL_arc, vR_arc;
        int vL_tank, vR_tank;

        int abs_x = abs(x);
        int abs_y = abs(y);

        // ===== ARC MODE =====
        int mag = abs_y;

        int denominator = abs_x + abs_y;
        int t = 0; // scaled 0 to 1000

        if (denominator > 0) {
            // Multiply before dividing to retain precision!
            t = (abs_x * 1000) / denominator;
        }

        int vFast = mag;
        int vSlow = (mag * (1000 - t)) / 1000;

        if (x > 0) {
            vL_arc = vFast;
            vR_arc = vSlow;
        } else {
            vL_arc = vSlow;
            vR_arc = vFast;
        }

        int dir = sign(y);
        vL_arc *= dir;
        vR_arc *= dir;

        // ===== TANK MODE =====
        vL_tank = x;
        vR_tank = -x;

        // ===== BLENDING =====
        int alpha = clamp(abs_y * 2, 0, 1000);

        // Fixed-point blending math
        int vL_target = (alpha * vL_arc + (1000 - alpha) * vL_tank) / 1000;
        int vR_target = (alpha * vR_arc + (1000 - alpha) * vR_tank) / 1000;

        vLeft_cur = vL_target;
        vRight_cur = vR_target;

        // ===== PWM OUTPUT =====
        // Since input was already 1000-based, our targets are naturally scaled to MAX_PWM.
        // We preserve your original cross-assignment (left gets right, right gets left).
        setLeftMotor(abs(vRight_cur), getDirection(vRight_cur));
        setRightMotor(abs(vLeft_cur), getDirection(vLeft_cur));

        update(); // Push to silicon
    }
}