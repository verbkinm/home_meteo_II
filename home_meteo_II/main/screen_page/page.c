/*
 * page.c
 *
 *  Created on: 29 янв. 2024 г.
 *      Author: user
 */

#include "page.h"

#include "Local_Lib/local_lib.h"
#include <esp_log.h>

static const char *TAG = "PAGE";

static page_t glob_current_page;
static page_type_t new_page_num;
static char *page_title_str[] = {" ", "Меню", "Главный экран", "Цифровые часы", "Аналоговые часы", "Бинарные часы", "Настройки", "График погоды", "Погода в доме"};

char *page_title(uint8_t num)
{
	if (num >= END_PAGE_TITLE)
		return page_title_str[0];

	return page_title_str[num];
}

page_t *page_current(void)
{
	return &glob_current_page;
}

page_type_t page_get_new_num(void)
{
	return new_page_num;
}

void page_set_new_num(page_type_t num)
{
	new_page_num = inRange(num, PAGE_BLANK, PAGE_NUM - 1);
}

void page_default_deinit(void)
{
	page_t *page = page_current();
	lv_obj_clean(page->widget);
	lv_obj_clean(page->fullscreen);
	lv_obj_add_flag(page->fullscreen, LV_OBJ_FLAG_HIDDEN);
}
