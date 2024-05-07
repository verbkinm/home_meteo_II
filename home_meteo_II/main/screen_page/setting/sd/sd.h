/*
 * sd.h
 *
 *  Created on: 7 мая 2024 г.
 *      Author: user
 */

#ifndef MAIN_SCREEN_PAGE_SETTING_SD_SD_H_
#define MAIN_SCREEN_PAGE_SETTING_SD_SD_H_

#include <lvgl.h>
#include "Local_Lib/local_lib.h"
#include "screen_page/setting/settingpage.h"
#include "screen_page/setting/elements.h"
#include "sd/sd_spi.h"

void create_sd_sub_page(lv_event_t *e);
void free_sd_sub_page(void);

#endif /* MAIN_SCREEN_PAGE_SETTING_SD_SD_H_ */
