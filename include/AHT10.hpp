#ifndef AHT10_RTOS_HPP
#define AHT10_RTOS_HPP

#include "pico/stdlib.h"
#include "hardware/i2c.h"

class AHT10 {
public:
    // Construtor: Define qual I2C e endereço usar
    AHT10(i2c_inst_t* i2c_port, uint8_t address = 0x38);

    // Inicializa o sensor (envia comando de calibração)
    // Retorna true se sucesso
    bool begin();

    // Lê temperatura e umidade
    // Usa vTaskDelay para não travar o RTOS
    bool readSensor(float* humidity, float* temperature);

private:
    i2c_inst_t* _i2c;
    uint8_t _addr;
};

#endif