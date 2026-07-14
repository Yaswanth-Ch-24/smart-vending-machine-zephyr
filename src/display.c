#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "display.h"

LOG_MODULE_REGISTER(display, CONFIG_LOG_DEFAULT_LEVEL);

int display_init(void)
{
	LOG_INF("Display initialized");
	return 0;
}

void display_task(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (1) {
		LOG_DBG("Refreshing user display");
		k_sleep(K_MSEC(300));
	}
}
