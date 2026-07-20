/**
 * @file  ultrasonic_verify.c
 * @brief HC-SR04 delivery verification — Zephyr GPIO
 *        TRIG: PA1, ECHO: PA4 — STM32 F446RE
 */
#include "ultrasonic_verify.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(vending);

static const struct gpio_dt_spec trig = GPIO_DT_SPEC_GET(DT_ALIAS(ultrasonic_trig), gpios);
static const struct gpio_dt_spec echo = GPIO_DT_SPEC_GET(DT_ALIAS(ultrasonic_echo), gpios);

void Ultrasonic_Init(void)
{
    gpio_pin_configure_dt(&trig, GPIO_OUTPUT_LOW);
    gpio_pin_configure_dt(&echo, GPIO_INPUT);
}

uint32_t Ultrasonic_Measure(void)
{
    /* 10us trigger pulse */
    gpio_pin_set_dt(&trig, 1);
    k_busy_wait(10);
    gpio_pin_set_dt(&trig, 0);

    /* Wait for echo rising edge */
    uint32_t timeout = 100000;
    while (!gpio_pin_get_dt(&echo) && timeout--);

    /* Measure pulse width in microseconds */
    uint32_t start = k_cycle_get_32();
    timeout = 100000;
    while (gpio_pin_get_dt(&echo) && timeout--);
    uint32_t end = k_cycle_get_32();

    uint32_t cycles = end - start;
    uint32_t us     = k_cyc_to_us_near32(cycles);

    LOG_DBG("[VRFY]  Echo pulse = %dus", us);
    return us / 58;  /* cm */
}
