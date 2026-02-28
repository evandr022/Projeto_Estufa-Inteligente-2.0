#include "AHT10.hpp"
#include "FreeRTOS.h" // Necessário para vTaskDelay
#include "task.h"

// Construtor
AHT10::AHT10(i2c_inst_t* i2c_port, uint8_t address) {
    _i2c = i2c_port;
    _addr = address;
}

// Inicialização (Baseada na sua lógica manual que funcionou)
bool AHT10::begin() {
    uint8_t cmd_init[3] = {0xE1, 0x08, 0x00};
    
    // Envia comando de calibração
    int ret = i2c_write_blocking(_i2c, _addr, cmd_init, 3, false);
    
    // Delay não-bloqueante de 100ms para o sensor acordar
    vTaskDelay(pdMS_TO_TICKS(100)); 
    
    return (ret >= 0);
}

// Leitura (Lógica manual portada para a classe)
bool AHT10::readSensor(float* humidity, float* temperature) {
    uint8_t cmd[3] = {0xAC, 0x33, 0x00}; // Comando Trigger Measurement
    uint8_t data[6];

    // 1. Enviar comando de medição
    if (i2c_write_blocking(_i2c, _addr, cmd, 3, false) < 0) return false;

    // 2. Esperar 80ms (Usando RTOS para liberar CPU para o LCD)
    vTaskDelay(pdMS_TO_TICKS(80));

    // 3. Ler 6 bytes
    if (i2c_read_blocking(_i2c, _addr, data, 6, false) < 0) return false;

    // 4. Conversão Matemática (Datasheet AHT10)
    uint32_t rawHum = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | (data[3] >> 4);
    uint32_t rawTemp = (((uint32_t)data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];

    *humidity = (rawHum * 100.0f) / 1048576.0f;
    *temperature = ((rawTemp * 200.0f) / 1048576.0f) - 50.0f;

    return true;
}