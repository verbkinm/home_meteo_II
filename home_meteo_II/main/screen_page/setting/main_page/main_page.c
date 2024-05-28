/*
 * main_page.c
 *
 *  Created on: 27 мая 2024 г.
 *      Author: user
 */

#include "main_page.h"

#include "screen_page/setting/settingpage.h"
#include "screen_page/setting/elements.h"
#include "screen_page/homepage/block/block.h"
#include "Local_Lib/local_lvgl_lib.h"
#include "service/bme280/bme280.h"
#include "json/json_config.h"

extern lv_font_t ubuntu_mono_14;
extern lv_obj_t *sub_main_page;

static const char *TAG = "MAIN_PAGE";

static const char *blocks_str[] = {BLOCK_0_STR, BLOCK_1_STR, BLOCK_2_STR, BLOCK_3_STR};
static const char *txt = "Выбор видов блоков на главном экране:";

static void save_handler(lv_event_t *e);
static void dropdown_handler(lv_event_t *e);
static void dropdown_select_blank(const lv_obj_t *target, const lv_obj_t *obj, lv_obj_t *obj_select, block_type_t cmp_type);

static void dropdown_select_blank(const lv_obj_t *target, const lv_obj_t *obj, lv_obj_t *obj_select, block_type_t cmp_type)
{
	if (target == obj && lv_dropdown_get_selected(target) == cmp_type)
			lv_dropdown_set_selected(obj_select, BLOCKTYPE_NONE);
}

static void dropdown_handler(lv_event_t *e)
{
	lv_obj_t *dropdown_list_0 = lv_obj_get_child(e->user_data, 2);
	lv_obj_t *dropdown_list_1 = lv_obj_get_child(e->user_data, 3);
	lv_obj_t *dropdown_list_2 = lv_obj_get_child(e->user_data, 4);
	lv_obj_t *dropdown_list_3 = lv_obj_get_child(e->user_data, 5);

	dropdown_select_blank(e->target, dropdown_list_0, dropdown_list_2, BLOCKTYPE_WEATHER_2_4_V);
	dropdown_select_blank(e->target, dropdown_list_2, dropdown_list_0, BLOCKTYPE_WEATHER_2_4_V);
	dropdown_select_blank(e->target, dropdown_list_1, dropdown_list_3, BLOCKTYPE_WEATHER_2_4_V);
	dropdown_select_blank(e->target, dropdown_list_3, dropdown_list_1, BLOCKTYPE_WEATHER_2_4_V);

	dropdown_select_blank(e->target, dropdown_list_0, dropdown_list_1, BLOCKTYPE_DATETIME_2_4_H);
	dropdown_select_blank(e->target, dropdown_list_1, dropdown_list_2, BLOCKTYPE_DATETIME_2_4_H);
	dropdown_select_blank(e->target, dropdown_list_2, dropdown_list_3, BLOCKTYPE_DATETIME_2_4_H);
	dropdown_select_blank(e->target, dropdown_list_3, dropdown_list_2, BLOCKTYPE_DATETIME_2_4_H);
}

static void save_handler(lv_event_t *e)
{
	for (int i = 0; i < 4; ++i)
	{
		lv_obj_t *dropdown_list = lv_obj_get_child(e->user_data, i + 2);
		block_type_t bt = lv_dropdown_get_selected(dropdown_list);

		char buf[8] = {0};
		snprintf(buf, sizeof(buf) - 1, "%d", bt);

		set_main_page_config_value(blocks_str[i], buf);
	}
}

void create_main_sub_page(lv_event_t *e)
{
	clear_all_sub_page_child();
	lv_obj_set_style_pad_hor(sub_main_page, 20, 0);

	lv_obj_t *section = lv_menu_section_create(sub_main_page);

	create_text(section, NULL, txt, LV_MENU_ITEM_BUILDER_VAR_1);

	lv_obj_t *obj = lv_menu_cont_create(section);
	lv_obj_set_scroll_dir(obj, LV_DIR_NONE);
	lv_obj_t *wrap = lv_obj_create(obj);
	lv_obj_set_size(wrap, 495, 250);
	lv_obj_set_style_pad_all(wrap, 0, 0);

	// Линии
	obj = lv_line_create(wrap);
	lv_obj_set_style_line_width(obj, 5, 0);
	lv_obj_set_style_line_color(obj, lv_color_black(), 0);
	static lv_point_t line_points[] = { {0, 0}, {0, 230} };
	lv_line_set_points(obj, line_points, 2);
	lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);

	obj = lv_line_create(wrap);
	lv_obj_set_style_line_width(obj, 5, 0);
	lv_obj_set_style_line_color(obj, lv_color_black(), 0);
	static lv_point_t line_points_2[] = { {0, 0}, {420, 0} };
	lv_line_set_points(obj, line_points_2, 2);
	lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);

	// drop-down list opt
	static const char *buf_up = "Нет\n"
			"Дата время 1/4\n"
			"Дата время 2/4_H\n"
			"Локальный датчик 1/4\n"
			"Удалённый датчик 1/4\n"
			"Погода 1/4\n"
			"Погода 2/4_V";

	const lv_align_t blocks_align[] = {LV_ALIGN_LEFT_MID, LV_ALIGN_RIGHT_MID, LV_ALIGN_LEFT_MID, LV_ALIGN_RIGHT_MID};
	const int x[] = {40, -50, 40, -50};
	const int y[] = {-65, -65, 65, 65};
	for (int i = 0; i < 4; ++i)
	{
		lv_obj_t *dropdown_list = create_drop_down_list(wrap, buf_up, 160, 40, blocks_align[i], x[i], y[i], &ubuntu_mono_14);
		lv_obj_add_event_cb(dropdown_list, dropdown_handler, LV_EVENT_VALUE_CHANGED, wrap);
		char *obj = NULL;
		if (get_main_page_config_value(blocks_str[i], &obj) && obj != NULL)
		{
			block_type_t bt = strtol(obj, NULL, 10);
			lv_dropdown_set_selected(dropdown_list, bt);
			free(obj);
		}
	}

	lv_obj_t *btn_save = NULL;
	create_button(section, SAVE_STR, 128, 40, &btn_save);
	lv_obj_add_event_cb(btn_save, save_handler, LV_EVENT_CLICKED, wrap);
}

void free_main_sub_page(void)
{

}
