#ifndef SERVO_HPP
#define SERVO_HPP

#include "pico/stdlib.h"
#include "hardware/pwm.h"

class Servo {
public:
    Servo(uint pin);
    void setAngle(int angle); // Define ângulo de 0 a 180

private:
    uint _pin;
    uint _slice_num;
    const uint32_t _pwm_freq = 50; // Frequência padrão de servos (50Hz)
};

#endif