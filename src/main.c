/**
 ******************************************************************************
 * @file    main.c
 * @brief   Smart Vending Machine — Zephyr RTOS
 *          Board  : STM32 F446RE (Nucleo-F446RE)
 *          Author : Chlliboina Yaswanth
 ******************************************************************************
 *
 * THREAD ARCHITECTURE
 * ───────────────────
 *  keypad_thread    — scans 4×4 matrix every 50ms, pushes to msgq_keypad
 *  dispense_thread  — waits on msgq_keypad, drives servo via PWM
 *  verify_thread    — fires HC-SR04 after dispense, confirms delivery
 *  wifi_thread      — receives inventory updates, sends via ESP8266
 *  display_thread   — updates I2C LCD on any state change
 *
 ******************************************************************************
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include <string.h>

#include "keypad.h"
#include "servo_dispense.h"
#include "ultrasonic_verify.h"
#include "wifi_inventory.h"
#include "lcd_i2c.h"

LOG_MODULE_REGISTER(vending, LOG_LEVEL_DBG);

/* ── Inventory ──────────────────────────────────── */
typedef struct {
    char     id[4];
    char     name[16];
    uint32_t price;
    uint8_t  stock;
} Product;

static Product inventory[] = {
    {"A1", "Chips",     20, 5},
    {"A2", "Water",     15, 3},
    {"A3", "Soda",      25, 2},
    {"B1", "Biscuits",  10, 4},
    {"B2", "Juice",     30, 1},
};
#define NUM_PRODUCTS 5

/* ── Message Queues ─────────────────────────────── */
K_MSGQ_DEFINE(msgq_keypad,    sizeof(char[4]), 4, 4);  /* Product ID  */
K_MSGQ_DEFINE(msgq_inventory, sizeof(uint8_t), 8, 1);  /* Product idx */

/* ── Semaphores ─────────────────────────────────── */
K_SEM_DEFINE(sem_verify,  0, 1);
K_SEM_DEFINE(sem_display, 0, 1);

/* ── Shared state (protected by mutex) ──────────── */
K_MUTEX_DEFINE(mtx_state);
static char    currentProduct[4]  = "---";
static char    displayLine1[17]   = "Select Product  ";
static char    displayLine2[17]   = "                ";
static uint8_t pendingIdx         = 0xFF;

/* ── Find product by ID ─────────────────────────── */
static int8_t find_product(const char *id)
{
    for (uint8_t i = 0; i < NUM_PRODUCTS; i++)
        if (strncmp(inventory[i].id, id, 2) == 0) return i;
    return -1;
}

/* ── Print inventory ────────────────────────────── */
static void print_inventory(void)
{
    printk("\r\n  Inventory:\r\n");
    for (uint8_t i = 0; i < NUM_PRODUCTS; i++)
        printk("    %s: %-10s Rs.%2d  Stock:%d\r\n",
               inventory[i].id, inventory[i].name,
               inventory[i].price, inventory[i].stock);
    printk("\r\n");
}

/* ────────────────────────────────────────────────
 * THREAD 1 — Keypad Scanner
 * ──────────────────────────────────────────────── */
#define KEYPAD_STACK 512
K_THREAD_STACK_DEFINE(keypad_stack, KEYPAD_STACK);
struct k_thread keypad_tid;

void keypad_thread(void *a, void *b, void *c)
{
    Keypad_Init();
    char sel[4] = {0};
    uint8_t pos = 0;

    while (1)
    {
        char key = Keypad_GetKey();
        if (key != 0)
        {
            if (key == '#' && pos == 2)
            {
                /* Confirm selection */
                sel[2] = '\0';
                LOG_INF("[KEY] Confirmed: %s", sel);
                k_msgq_put(&msgq_keypad, sel, K_NO_WAIT);
                pos = 0;
                memset(sel, 0, sizeof(sel));
            }
            else if (key == '*')
            {
                /* Cancel */
                pos = 0;
                memset(sel, 0, sizeof(sel));
                LOG_INF("[KEY] Cancelled");
            }
            else if (pos < 2)
            {
                sel[pos++] = key;
                LOG_INF("[KEY] Input: %c (pos=%d)", key, pos);
            }
        }
        k_msleep(50);
    }
}

/* ────────────────────────────────────────────────
 * THREAD 2 — Dispense Controller
 * ──────────────────────────────────────────────── */
#define DISPENSE_STACK 512
K_THREAD_STACK_DEFINE(dispense_stack, DISPENSE_STACK);
struct k_thread dispense_tid;

void dispense_thread(void *a, void *b, void *c)
{
    Servo_Init();

    while (1)
    {
        char id[4];
        if (k_msgq_get(&msgq_keypad, id, K_FOREVER) == 0)
        {
            int8_t idx = find_product(id);

            k_mutex_lock(&mtx_state, K_FOREVER);
            strncpy(currentProduct, id, 3);

            if (idx < 0)
            {
                LOG_WRN("[DISP] Unknown product: %s", id);
                snprintf(displayLine1, 17, "Unknown: %s", id);
                snprintf(displayLine2, 17, "Try again       ");
                k_mutex_unlock(&mtx_state);
                k_sem_give(&sem_display);
                continue;
            }

            if (inventory[idx].stock == 0)
            {
                LOG_WRN("[DISP] %s OUT OF STOCK", id);
                snprintf(displayLine1, 17, "%s: OUT STOCK", id);
                snprintf(displayLine2, 17, "                ");
                k_mutex_unlock(&mtx_state);
                k_sem_give(&sem_display);

                /* Notify wifi thread of low stock */
                uint8_t pidx = (uint8_t)idx;
                k_msgq_put(&msgq_inventory, &pidx, K_NO_WAIT);
                continue;
            }

            snprintf(displayLine1, 17, "%s: %s", id, inventory[idx].name);
            snprintf(displayLine2, 17, "Rs.%d Dispensing", inventory[idx].price);
            k_mutex_unlock(&mtx_state);
            k_sem_give(&sem_display);

            LOG_INF("[DISP] Dispensing %s (%s)", id, inventory[idx].name);

            /* Servo: rotate to dispense position */
            Servo_Dispense();
            k_msleep(500);
            Servo_Reset();

            /* Signal verify thread */
            pendingIdx = (uint8_t)idx;
            k_sem_give(&sem_verify);
        }
    }
}

/* ────────────────────────────────────────────────
 * THREAD 3 — Delivery Verifier
 * ──────────────────────────────────────────────── */
#define VERIFY_STACK 512
K_THREAD_STACK_DEFINE(verify_stack, VERIFY_STACK);
struct k_thread verify_tid;

void verify_thread(void *a, void *b, void *c)
{
    Ultrasonic_Init();

    while (1)
    {
        k_sem_take(&sem_verify, K_FOREVER);

        k_msleep(300); /* Wait for item to settle */
        uint32_t dist = Ultrasonic_Measure();
        uint8_t idx   = pendingIdx;

        k_mutex_lock(&mtx_state, K_FOREVER);

        if (dist < 30) /* Item detected at collection tray */
        {
            LOG_INF("[VRFY] DELIVERED — dist=%dcm", dist);
            inventory[idx].stock--;
            snprintf(displayLine1, 17, "Enjoy! Take item");
            snprintf(displayLine2, 17, "%s remaining: %d ",
                     inventory[idx].id, inventory[idx].stock);

            /* Notify wifi to sync inventory */
            k_msgq_put(&msgq_inventory, &idx, K_NO_WAIT);
        }
        else
        {
            LOG_ERR("[VRFY] DELIVERY FAILED — dist=%dcm", dist);
            snprintf(displayLine1, 17, "Error! No item  ");
            snprintf(displayLine2, 17, "Please retry    ");
        }

        k_mutex_unlock(&mtx_state);
        k_sem_give(&sem_display);
    }
}

/* ────────────────────────────────────────────────
 * THREAD 4 — WiFi Inventory Sync
 * ──────────────────────────────────────────────── */
#define WIFI_STACK 768
K_THREAD_STACK_DEFINE(wifi_stack, WIFI_STACK);
struct k_thread wifi_tid;

void wifi_thread(void *a, void *b, void *c)
{
    WiFi_Init();

    while (1)
    {
        uint8_t idx;
        if (k_msgq_get(&msgq_inventory, &idx, K_FOREVER) == 0)
        {
            char msg[64];
            if (inventory[idx].stock == 0)
            {
                snprintf(msg, sizeof(msg),
                         "LOW_STOCK:%s=%d", inventory[idx].id, inventory[idx].stock);
                LOG_WRN("[WIFI] LOW STOCK ALERT: %s=0 >> Sent to server", inventory[idx].id);
            }
            else
            {
                snprintf(msg, sizeof(msg),
                         "UPDATE:%s=%d", inventory[idx].id, inventory[idx].stock);
                LOG_INF("[WIFI] Inventory update sent: %s=%d remaining",
                        inventory[idx].id, inventory[idx].stock);
            }
            WiFi_SendLog(msg);
        }
    }
}

/* ────────────────────────────────────────────────
 * THREAD 5 — LCD Display Updater
 * ──────────────────────────────────────────────── */
#define DISPLAY_STACK 512
K_THREAD_STACK_DEFINE(display_stack, DISPLAY_STACK);
struct k_thread display_tid;

void display_thread(void *a, void *b, void *c)
{
    LCD_Init();
    LCD_Print(0, "Smart Vending   ");
    LCD_Print(1, "STM32 F446RE    ");
    k_msleep(2000);
    LCD_Print(0, "Select Product  ");
    LCD_Print(1, "A1 A2 A3 B1 B2  ");

    while (1)
    {
        k_sem_take(&sem_display, K_FOREVER);
        k_mutex_lock(&mtx_state, K_FOREVER);
        LCD_Print(0, displayLine1);
        LCD_Print(1, displayLine2);
        k_mutex_unlock(&mtx_state);
        k_msleep(3000);
        LCD_Print(0, "Select Product  ");
        LCD_Print(1, "A1 A2 A3 B1 B2  ");
    }
}

/* ────────────────────────────────────────────────
 * MAIN — Spawn all threads
 * ──────────────────────────────────────────────── */
int main(void)
{
    printk("\r\n=========================================\r\n");
    printk("  Smart Vending Machine - Zephyr RTOS\r\n");
    printk("  STM32 F446RE | Yaswanth Chlliboina\r\n");
    printk("=========================================\r\n");
    printk("[RTOS]  Spawning 5 threads...\r\n");

    print_inventory();

    k_thread_create(&keypad_tid,   keypad_stack,   KEYPAD_STACK,
                    keypad_thread,   NULL, NULL, NULL, 5, 0, K_NO_WAIT);
    k_thread_create(&dispense_tid, dispense_stack, DISPENSE_STACK,
                    dispense_thread, NULL, NULL, NULL, 4, 0, K_NO_WAIT);
    k_thread_create(&verify_tid,   verify_stack,   VERIFY_STACK,
                    verify_thread,   NULL, NULL, NULL, 4, 0, K_NO_WAIT);
    k_thread_create(&wifi_tid,     wifi_stack,     WIFI_STACK,
                    wifi_thread,     NULL, NULL, NULL, 6, 0, K_NO_WAIT);
    k_thread_create(&display_tid,  display_stack,  DISPLAY_STACK,
                    display_thread,  NULL, NULL, NULL, 7, 0, K_NO_WAIT);

    printk("[RTOS]  5 threads spawned — System running\r\n");
    printk("[LCD]   \"Select Product\"\r\n\r\n");

    return 0;
}
