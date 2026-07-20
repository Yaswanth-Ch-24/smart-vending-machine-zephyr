/**
 * @file  lcd_i2c.c
 * @brief 16x2 I2C LCD (PCF8574 backpack) driver — Zephyr I2C1
 *        SDA=PB7, SCL=PB8 on STM32 F446RE
 *        I2C address: 0x27
 */
#include "lcd_i2c.h"
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_DECLARE(vending);

#define LCD_ADDR   0x27
#define LCD_COLS   16

static const struct device *i2c1 = DEVICE_DT_GET(DT_ALIAS(lcd_i2c));

static void lcd_send_nibble(uint8_t nibble, uint8_t rs)
{
    uint8_t data = (nibble << 4) | (rs ? 0x01 : 0x00) | 0x08; /* BL=1 */
    uint8_t en_high = data | 0x04;
    uint8_t en_low  = data & ~0x04;
    i2c_write(i2c1, &en_high, 1, LCD_ADDR);
    k_busy_wait(1);
    i2c_write(i2c1, &en_low,  1, LCD_ADDR);
    k_busy_wait(50);
}

static void lcd_send_byte(uint8_t byte, uint8_t rs)
{
    lcd_send_nibble(byte >> 4, rs);
    lcd_send_nibble(byte & 0x0F, rs);
}

void LCD_Init(void)
{
    if (!device_is_ready(i2c1)) { LOG_ERR("LCD I2C not ready"); return; }
    k_msleep(50);
    lcd_send_nibble(0x03, 0); k_msleep(5);
    lcd_send_nibble(0x03, 0); k_busy_wait(150);
    lcd_send_nibble(0x03, 0);
    lcd_send_nibble(0x02, 0);
    lcd_send_byte(0x28, 0);  /* 4-bit, 2 lines */
    lcd_send_byte(0x0C, 0);  /* Display ON, cursor OFF */
    lcd_send_byte(0x06, 0);  /* Entry mode */
    LCD_Clear();
}

void LCD_Clear(void) { lcd_send_byte(0x01, 0); k_msleep(2); }

void LCD_Print(uint8_t row, const char *text)
{
    uint8_t addr = (row == 0) ? 0x80 : 0xC0;
    lcd_send_byte(addr, 0);
    for (uint8_t i = 0; i < LCD_COLS && text[i]; i++)
        lcd_send_byte((uint8_t)text[i], 1);
}
