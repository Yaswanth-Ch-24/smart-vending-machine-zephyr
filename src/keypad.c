/**
 * @file  keypad.c
 * @brief 4×4 Matrix Keypad — Zephyr GPIO driver
 *        Rows: PB3,PB4,PB5,PB6 (Output)
 *        Cols: PC7,PA8,PA9,PA10 (Input Pull-up)
 */
#include "keypad.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(vending);

/* Key map */
static const char keymap[4][4] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

/* GPIO specs — adjust pin numbers to your board DTS */
static const struct gpio_dt_spec rows[4] = {
    GPIO_DT_SPEC_GET(DT_ALIAS(row0), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(row1), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(row2), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(row3), gpios),
};
static const struct gpio_dt_spec cols[4] = {
    GPIO_DT_SPEC_GET(DT_ALIAS(col0), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(col1), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(col2), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(col3), gpios),
};

void Keypad_Init(void)
{
    for (int i = 0; i < 4; i++) {
        gpio_pin_configure_dt(&rows[i], GPIO_OUTPUT_HIGH);
        gpio_pin_configure_dt(&cols[i], GPIO_INPUT | GPIO_PULL_UP);
    }
}

char Keypad_GetKey(void)
{
    for (int r = 0; r < 4; r++) {
        gpio_pin_set_dt(&rows[r], 0);  /* Pull row LOW */
        for (int c = 0; c < 4; c++) {
            if (gpio_pin_get_dt(&cols[c]) == 0) {
                k_msleep(20);  /* Debounce */
                if (gpio_pin_get_dt(&cols[c]) == 0) {
                    gpio_pin_set_dt(&rows[r], 1);
                    return keymap[r][c];
                }
            }
        }
        gpio_pin_set_dt(&rows[r], 1);  /* Release row */
    }
    return 0;
}
