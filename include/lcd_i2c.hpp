#ifndef LCD_I2C_HPP
#define LCD_I2C_HPP

#include "pico/stdlib.h"
#include "hardware/i2c.h"

class LcdI2C {
public:
    // MUDANÇA: addr foi para o final e ganhou um valor padrão
    LcdI2C(i2c_inst_t* i2c, uint8_t sda_pin, uint8_t scl_pin, uint8_t addr = 0x27);

    void init();
    void set_cursor(int line, int position);
    void print(const char* str);
    void clear();

private:
    i2c_inst_t* _i2c;
    uint8_t _addr;
    
    void lcd_send_cmd(uint8_t cmd);
    void lcd_send_data(uint8_t data);
    void lcd_send_byte(uint8_t val, int mode);
    void lcd_toggle_enable(uint8_t val);
};

#endif