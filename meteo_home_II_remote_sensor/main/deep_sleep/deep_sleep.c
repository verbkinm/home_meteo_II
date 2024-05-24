/*
 * deep_sleep.c
 *
 *  Created on: 8 мая 2024 г.
 *      Author: user
 */

#include "deep_sleep.h"

#include <esp_log.h>
#include <esp_sleep.h>
#include <stdio.h>

#include "Global_def.h"

void deep_sleep(uint16_t sec)
{
	printf("Enabling timer wakeup, %ds\n", sec);
	ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(sec * 1000000));
	esp_deep_sleep_start();
}
