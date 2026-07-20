#ifndef ULTRASONIC_VERIFY_H
#define ULTRASONIC_VERIFY_H
#include <zephyr/kernel.h>
#include <stdint.h>
void     Ultrasonic_Init(void);
uint32_t Ultrasonic_Measure(void);  /* Returns distance in cm */
#endif
