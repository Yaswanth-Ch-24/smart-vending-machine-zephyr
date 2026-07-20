#ifndef SERVO_DISPENSE_H
#define SERVO_DISPENSE_H
#include <zephyr/kernel.h>
void Servo_Init(void);
void Servo_Dispense(void);  /* Rotate to 90deg — drop item */
void Servo_Reset(void);     /* Return to 0deg */
#endif
