#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "ultrasonic_verify.h"

LOG_MODULE_REGISTER(ultrasonic_verify, CONFIG_LOG_DEFAULT_LEVEL);

int ultrasonic_verify_init(void)
{
	LOG_INF("Ultrasonic verification initialized");
	return 0;
}

void ultrasonic_verify_task(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (1) {
		LOG_DBG("Checking ultrasonic sensor");
		k_sleep(K_MSEC(500));
	}
}
