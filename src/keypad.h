#ifndef KEYPAD_H
#define KEYPAD_H
#include <zephyr/kernel.h>
void Keypad_Init(void);
char Keypad_GetKey(void);   /* Returns key char or 0 if none */
#endif
