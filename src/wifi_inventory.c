#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "wifi_inventory.h"

LOG_MODULE_REGISTER(wifi_inventory, CONFIG_LOG_DEFAULT_LEVEL);

int wifi_inventory_init(void)
{
	LOG_INF("Wi-Fi inventory initialized");
	return 0;
}

void wifi_inventory_task(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (1) {
		LOG_DBG("Syncing inventory state");
		k_sleep(K_SECONDS(1));
	}
}
