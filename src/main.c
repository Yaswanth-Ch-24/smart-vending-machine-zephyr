#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "keypad.h"
#include "servo_dispense.h"
#include "ultrasonic_verify.h"
#include "wifi_inventory.h"
#include "display.h"

LOG_MODULE_REGISTER(main_app, CONFIG_LOG_DEFAULT_LEVEL);

#define TASK_STACK_SIZE 1024
#define TASK_PRIORITY 5

K_THREAD_STACK_DEFINE(keypad_stack, TASK_STACK_SIZE);
K_THREAD_STACK_DEFINE(dispense_stack, TASK_STACK_SIZE);
K_THREAD_STACK_DEFINE(verify_stack, TASK_STACK_SIZE);
K_THREAD_STACK_DEFINE(wifi_stack, TASK_STACK_SIZE);
K_THREAD_STACK_DEFINE(display_stack, TASK_STACK_SIZE);

static struct k_thread keypad_thread;
static struct k_thread dispense_thread;
static struct k_thread verify_thread;
static struct k_thread wifi_thread;
static struct k_thread display_thread;

int main(void)
{
	int rc = 0;

	rc |= keypad_init();
	rc |= servo_dispense_init();
	rc |= ultrasonic_verify_init();
	rc |= wifi_inventory_init();
	rc |= display_init();

	if (rc != 0) {
		LOG_ERR("Module initialization failed");
		return rc;
	}

	k_thread_create(&keypad_thread, keypad_stack, K_THREAD_STACK_SIZEOF(keypad_stack),
			keypad_task, NULL, NULL, NULL, TASK_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&keypad_thread, "keypad_task");

	k_thread_create(&dispense_thread, dispense_stack, K_THREAD_STACK_SIZEOF(dispense_stack),
			servo_dispense_task, NULL, NULL, NULL, TASK_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&dispense_thread, "dispense_task");

	k_thread_create(&verify_thread, verify_stack, K_THREAD_STACK_SIZEOF(verify_stack),
			ultrasonic_verify_task, NULL, NULL, NULL, TASK_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&verify_thread, "verify_task");

	k_thread_create(&wifi_thread, wifi_stack, K_THREAD_STACK_SIZEOF(wifi_stack),
			wifi_inventory_task, NULL, NULL, NULL, TASK_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&wifi_thread, "wifi_task");

	k_thread_create(&display_thread, display_stack, K_THREAD_STACK_SIZEOF(display_stack),
			display_task, NULL, NULL, NULL, TASK_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&display_thread, "display_task");

	LOG_INF("Smart vending machine started");

	while (1) {
		k_sleep(K_SECONDS(1));
	}

	return 0;
}
