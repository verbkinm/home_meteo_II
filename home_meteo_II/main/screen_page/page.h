/*
 * page.h
 *
 *  Created on: 25 янв. 2024 г.
 *      Author: user
 */

#ifndef MAIN_SCREEN_PAGE_PAGE_H_
#define MAIN_SCREEN_PAGE_PAGE_H_

#include "lvgl.h"

enum {
	PAGE_TITLE_EMPTY = 0,
	PAGE_TITLE_MENU,
	PAGE_TITLE_MAIN,
	PAGE_TITLE_DATETIME1,
	PAGE_TITLE_DATETIME2,
	PAGE_TITLE_DATETIME3,
	PAGE_TITLE_SETTING,
	PAGE_TITLE_METEO_CHART,
	PAGE_TITLE_LOCAL_SENSOR,

	END_PAGE_TITLE
};
typedef int page_title_t;

// Переключение страниц
enum {
	PAGE_BLANK,
	PAGE_MENU,
	PAGE_MAIN,
	PAGE_DATETIME1,
	PAGE_DATETIME2,
	PAGE_DATETIME3,
	PAGE_SETTING,
	PAGE_METEO_CHART,
	PAGE_LOCAL_SENSOR,

	PAGE_SYS_SD_ERROR,
	PAGE_SYS_UPDATE_NOW,
	PAGE_SYS_UPDATE_DONE,
	PAGE_SYS_UPDATE_ERROR,

	PAGE_NUM
};
typedef int page_type_t;

typedef struct Page {
	char *title;
	lv_obj_t *widget;
	lv_obj_t *fullscreen;
	void (*deinit)(void);
	page_type_t num;
} page_t;

char *page_title(uint8_t num);

page_t *page_current(void);
void page_default_deinit(void);

page_type_t page_get_new_num(void);
void page_set_new_num(page_type_t num);

#endif /* MAIN_SCREEN_PAGE_PAGE_H_ */
