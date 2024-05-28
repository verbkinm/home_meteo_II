/*
 * weather.h
 *
 *  Created on: 27 мая 2024 г.
 *      Author: user
 */

#ifndef MAIN_SCREEN_PAGE_HOMEPAGE_BLOCK_WEATHER_WEATHER_H_
#define MAIN_SCREEN_PAGE_HOMEPAGE_BLOCK_WEATHER_WEATHER_H_

#include <lvgl.h>

lv_obj_t *block_weather_create_1_4(lv_obj_t *parent);
bool block_weather_update_1_4(lv_obj_t *parent);

lv_obj_t *block_weather_create_2_4_v(lv_obj_t *parent);
bool block_weather_update_2_4_v(lv_obj_t *parent);

#endif /* MAIN_SCREEN_PAGE_HOMEPAGE_BLOCK_WEATHER_WEATHER_H_ */
