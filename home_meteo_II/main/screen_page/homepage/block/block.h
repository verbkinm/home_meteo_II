/*
 * block.h
 *
 *  Created on: 24 мая 2024 г.
 *      Author: user
 */

#ifndef MAIN_SCREEN_PAGE_HOMEPAGE_BLOCK_BLOCK_H_
#define MAIN_SCREEN_PAGE_HOMEPAGE_BLOCK_BLOCK_H_

#include <lvgl.h>

enum {
	BLOCKTYPE_NONE,

	BLOCKTYPE_DATETIME_1_4,
	BLOCKTYPE_DATETIME_2_4_H,
	BLOCKTYPE_LOCALSENSOR_1_4,
	BLOCKTYPE_REMOTESENSOR_1_4,
	BLOCKTYPE_WEATHER_1_4,
	BLOCKTYPE_WEATHER_2_4_V,

	BLOCKTYPE_NUM
};
typedef int block_type_t;

struct Block {
	lv_obj_t *widget;
	void (*timer_handler_cb)(lv_event_t *);
	block_type_t type;
};
typedef struct Block block_t;

lv_obj_t *block_create(lv_obj_t *parent, block_type_t type);
void block_update(lv_obj_t *obj);

#endif /* MAIN_SCREEN_PAGE_HOMEPAGE_BLOCK_BLOCK_H_ */
