#ifndef LCD_I2C_H
#define LCD_I2C_H
#include <zephyr/kernel.h>
void LCD_Init(void);
void LCD_Print(uint8_t row, const char *text);  /* row: 0 or 1 */
void LCD_Clear(void);
#endif
