/*

  This is a library for the BH1750FVI Digital Light Sensor breakout board.

  The BH1750 board uses I2C for communication. Two pins are required to
  interface to the device. Configuring the I2C bus is expected to be done
  in user code. The BH1750 library doesn't do this automatically.

  Datasheet:
  http://www.elechouse.com/elechouse/images/product/Digital%20light%20Sensor/bh1750fvi-e.pdf

  Written by Christopher Laws, March, 2013.

*/

#include "BH1750.hpp"

// Construtor
BH1750::BH1750(i2c_inst_t* i2c_instance) {
  I2C = i2c_instance;
}

// Inicialização
bool BH1750::begin(Mode mode, uint8_t addr) {
  BH1750_I2CADDR = addr;
  // Configure sensor in specified mode
  return (configure(mode) && setMTreg(BH1750_DEFAULT_MTREG));
}

// Configura o Modo
bool BH1750::configure(Mode mode) {
  uint8_t data = (uint8_t)mode;
  
  // Tenta escrever no I2C. Retorna o numero de bytes escritos.
  // Se retornar PICO_ERROR_GENERIC ou diferente de 1, falhou.
  int result = i2c_write_blocking(I2C, BH1750_I2CADDR, &data, 1, false);

  sleep_ms(10); // Espera o sensor acordar

  if (result == 1) {
    BH1750_MODE = mode;
    return true;
  }
  
  printf("[BH1750] Erro ao configurar modo.\n");
  return false;
}

// Ajusta sensibilidade (MTreg)
bool BH1750::setMTreg(uint8_t MTreg) {
  if (MTreg < BH1750_MTREG_MIN || MTreg > BH1750_MTREG_MAX) {
    printf("[BH1750] MTreg fora do range\n");
    return false;
  }

  // High bit: 01000_MT[7,6,5]
  uint8_t byte1 = (0b01000 << 3) | (MTreg >> 5);
  // Low bit: 011_MT[4,3,2,1,0]
  uint8_t byte2 = (0b011 << 5) | (MTreg & 0b11111);
  uint8_t byte3 = BH1750_MODE;

  // Envia sequencia de comandos
  i2c_write_blocking(I2C, BH1750_I2CADDR, &byte1, 1, false);
  i2c_write_blocking(I2C, BH1750_I2CADDR, &byte2, 1, false);
  i2c_write_blocking(I2C, BH1750_I2CADDR, &byte3, 1, false);

  sleep_ms(10);
  
  BH1750_MTreg = MTreg;
  return true;
}

// Lê o valor em Lux
float BH1750::readLightLevel() {
  if (BH1750_MODE == UNCONFIGURED) {
    printf("[BH1750] Dispositivo nao configurado!\n");
    return -2.0;
  }

  uint8_t buffer[2];
  // Lê 2 bytes do sensor
  int result = i2c_read_blocking(I2C, BH1750_I2CADDR, buffer, 2, false);

  if (result == 2) {
    unsigned int tmp = 0;
    tmp = buffer[0];
    tmp <<= 8;
    tmp |= buffer[1];
    
    float level = tmp;

    // Ajusta pelo MTReg
    if (BH1750_MTreg != BH1750_DEFAULT_MTREG) {
       level *= (float)((uint8_t)BH1750_DEFAULT_MTREG / (float)BH1750_MTreg);
    }
    
    // Ajusta se estiver em modo de alta resolução 2
    if (BH1750_MODE == BH1750::ONE_TIME_HIGH_RES_MODE_2 ||
        BH1750_MODE == BH1750::CONTINUOUS_HIGH_RES_MODE_2) {
      level /= 2;
    }

    // Converte para Lux
    level /= BH1750_CONV_FACTOR;
    return level;
  }

  return -1.0; // Erro na leitura
}