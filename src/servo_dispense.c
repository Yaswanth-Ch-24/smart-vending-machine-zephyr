/**
 * @file  servo_dispense.c
 * @brief SG90 Servo PWM driver — Zephyr on STM32 F446RE
 *        Signal pin: PA0 (TIM1 CH1)
 *        50Hz PWM: 1ms pulse=0deg, 1.5ms=90deg
 */
#include "servo_dispense.h"
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(vending);

#define SERVO_PERIOD_NS   20000000U  /* 20ms = 50Hz */
#define SERVO_CLOSE_NS     1000000U  /* 1ms  = 0deg  */
#define SERVO_OPEN_NS      1500000U  /* 1.5ms= 90deg */

static const struct pwm_dt_spec servo = PWM_DT_SPEC_GET(DT_ALIAS(servo0));

void Servo_Init(void)
{
    if (!pwm_is_ready_dt(&servo)) {
        LOG_ERR("Servo PWM not ready");
        return;
    }
    Servo_Reset();
}

void Servo_Dispense(void)
{
    LOG_INF("[DISP]  Servo rotating to 90deg (dispense)...");
    pwm_set_dt(&servo, SERVO_PERIOD_NS, SERVO_OPEN_NS);
    k_msleep(800);
}

void Servo_Reset(void)
{
    LOG_INF("[DISP]  Servo reset to 0deg");
    pwm_set_dt(&servo, SERVO_PERIOD_NS, SERVO_CLOSE_NS);
    k_msleep(500);
}
