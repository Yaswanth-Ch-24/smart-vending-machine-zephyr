/**
 * @file  wifi_inventory.c
 * @brief ESP8266 Wi-Fi AT command driver — USART2 on STM32 F446RE
 *        TX=PA2, RX=PA3 at 115200 baud
 */
#include "wifi_inventory.h"
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_DECLARE(vending);

static const struct device *uart2 = DEVICE_DT_GET(DT_ALIAS(wifi_uart));

static void uart_send(const char *s)
{
    while (*s) uart_poll_out(uart2, *s++);
}

void WiFi_Init(void)
{
    if (!device_is_ready(uart2)) {
        LOG_ERR("WiFi UART not ready");
        return;
    }
    k_msleep(1000);
    uart_send("AT\r\n");     k_msleep(200);
    uart_send("AT+CWMODE=1\r\n"); k_msleep(200);
    /* Configure your WiFi SSID/password here */
    uart_send("AT+CWJAP=\"YOUR_SSID\",\"YOUR_PASSWORD\"\r\n");
    k_msleep(5000);
    LOG_INF("[WIFI]  ESP8266 initialized");
}

void WiFi_SendLog(const char *msg)
{
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%d\r\n", (int)strlen(msg));
    uart_send(cmd);
    k_msleep(100);
    uart_send(msg);
    LOG_INF("[WIFI]  Sent: %s", msg);
}
