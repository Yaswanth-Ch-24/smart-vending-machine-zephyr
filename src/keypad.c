#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "keypad.h"

LOG_MODULE_REGISTER(keypad, CONFIG_LOG_DEFAULT_LEVEL);

int keypad_init(void)
{
	LOG_INF("Keypad initialized");
	return 0;
}

void keypad_task(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (1) {
		LOG_DBG("Scanning keypad input");
		k_sleep(K_MSEC(200));
	}
}
