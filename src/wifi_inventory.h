#ifndef WIFI_INVENTORY_H
#define WIFI_INVENTORY_H
#include <zephyr/kernel.h>
void WiFi_Init(void);
void WiFi_SendLog(const char *msg);
