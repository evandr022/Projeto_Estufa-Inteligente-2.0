#include "lcd_i2c.hpp"

// Comandos do LCD
const int LCD_CLEARDISPLAY = 0x01;
const int LCD_RETURNHOME = 0x02;
const int LCD_ENTRYMODESET = 0x04;
const int LCD_DISPLAYCONTROL = 0x08;
const int LCD_FUNCTIONSET = 0x20;
const int LCD_SETDDRAMADDR = 0x80;

// Flags para controle do display
const int LCD_ENTRYLEFT = 0x02;
const int LCD_DISPLAYON = 0x04;
const int LCD_BACKLIGHT = 0x08;
const int LCD_ENABLE_BIT = 0x04;

// Modos
const int LCD_CHARACTER = 1;
const int LCD_COMMAND = 0;

LcdI2C::LcdI2C(i2c_inst_t* i2c, uint8_t sda_pin, uint8_t scl_pin, uint8_t addr) 
    : _i2c(i2c), _addr(addr) {
    
    // Inicializa o I2C no Pico
    i2c_init(_i2c, 100 * 1000); // 100kHz
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);
}

void LcdI2C::init() {
    sleep_ms(50);
    // Sequência de inicialização para modo 4-bits
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x02, LCD_COMMAND);

    lcd_send_cmd(LCD_ENTRYMODESET | LCD_ENTRYLEFT);
    lcd_send_cmd(LCD_FUNCTIONSET | 0x00); // 2 linhas
    lcd_send_cmd(LCD_DISPLAYCONTROL | LCD_DISPLAYON);
    clear();
}

void LcdI2C::clear() {
    lcd_send_cmd(LCD_CLEARDISPLAY);
    sleep_ms(2); // Limpar demora um pouco mais
}

void LcdI2C::set_cursor(int line, int position) {
    int val = (line == 0) ? 0x80 + position : 0xC0 + position;
    lcd_send_cmd(val);
}

void LcdI2C::print(const char* str) {
    while (*str) {
        lcd_send_data(*str++);
    }
}

void LcdI2C::lcd_toggle_enable(uint8_t val) {
    sleep_us(600);
    i2c_write_blocking(_i2c, _addr, &val, 1, false);
    sleep_us(600);
    val |= LCD_ENABLE_BIT;
    i2c_write_blocking(_i2c, _addr, &val, 1, false);
    sleep_us(600);
    val &= ~LCD_ENABLE_BIT;
    i2c_write_blocking(_i2c, _addr, &val, 1, false);
    sleep_us(600);
}

void LcdI2C::lcd_send_byte(uint8_t val, int mode) {
    uint8_t high = mode | (val & 0xF0) | LCD_BACKLIGHT;
    uint8_t low = mode | ((val << 4) & 0xF0) | LCD_BACKLIGHT;

    lcd_toggle_enable(high);
    lcd_toggle_enable(low);
}

void LcdI2C::lcd_send_cmd(uint8_t cmd) {
    lcd_send_byte(cmd, 0);
}

void LcdI2C::lcd_send_data(uint8_t data) {
    lcd_send_byte(data, 1);
}