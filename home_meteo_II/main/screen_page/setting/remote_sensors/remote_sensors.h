/*
 * sensors.h
 *
 *  Created on: 15 мая 2024 г.
 *      Author: user
 */

#ifndef MAIN_SCREEN_PAGE_SETTING_REMOTE_SENSORS_SENSORS_H_
#define MAIN_SCREEN_PAGE_SETTING_REMOTE_SENSORS_SENSORS_H_

#include <lvgl.h>
#include "screen_page/setting/settingpage.h"
#include "screen_page/setting/elements.h"
#include "Local_Lib/local_lvgl_lib.h"

void create_remote_sensors_sub_page(lv_event_t *e);
void free_remote_sensors_sub_page(void);

#endif /* MAIN_SCREEN_PAGE_SETTING_REMOTE_SENSORS_SENSORS_H_ */
