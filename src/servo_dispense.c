#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "servo_dispense.h"

LOG_MODULE_REGISTER(servo_dispense, CONFIG_LOG_DEFAULT_LEVEL);

int servo_dispense_init(void)
{
	LOG_INF("Servo dispense initialized");
	return 0;
}

void servo_dispense_task(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (1) {
		LOG_DBG("Servo task idle");
		k_sleep(K_MSEC(500));
	}
}
