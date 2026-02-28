#include "Servo.hpp"
#include <stdio.h>

Servo::Servo(uint pin) : _pin(pin) {
    gpio_set_function(_pin, GPIO_FUNC_PWM);
    _slice_num = pwm_gpio_to_slice_num(_pin);

    pwm_config config = pwm_get_default_config();
    // Clock do Pico é 125MHz. 125M / (125 * 20000) = 50Hz
    pwm_config_set_clkdiv(&config, 125.0f);
    pwm_config_set_wrap(&config, 20000); 

    pwm_init(_slice_num, &config, true);
}

void Servo::setAngle(int angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    // Converte 0-180 graus para 500us-2400us (valores comuns para SG90)
    // 500us em um ciclo de 20000 (50Hz) = 500
    // 2400us em um ciclo de 20000 (50Hz) = 2400
    uint16_t pulse_width = 500 + (angle * (2400 - 500) / 180);
    pwm_set_gpio_level(_pin, pulse_width);
}