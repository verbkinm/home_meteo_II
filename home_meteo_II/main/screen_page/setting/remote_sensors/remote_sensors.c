/*
 * sensors.c
 *
 *  Created on: 9 февр. 2024 г.
 *      Author: user
 */

#include "remote_sensors.h"
#include "service/udp_server/udp_server.h"

//extern lv_font_t ubuntu_mono_14;
extern lv_obj_t *sub_remote_sensors_page;

static char *txt = "Удалённый датчик температуры и влажности:";

static lv_obj_t *lbl_cur_status;
static lv_obj_t *lbl_cur_temp;
static lv_obj_t *lbl_cur_hum;
static lv_obj_t *lbl_cur_battery;

static lv_timer_t *timer = NULL;

static void timer_loop(lv_timer_t *timer);
static void update(lv_event_t *e);

static void timer_loop(lv_timer_t *timer)
{
//	if (ble_client_is_connect())
//	{
//		lv_label_set_text(lbl_cur_status, "Подключён");
//		lv_obj_set_style_text_color(lbl_cur_status, lv_palette_main(LV_PALETTE_GREEN), 0);
//	}
//	else
//	{
//		lv_label_set_text(lbl_cur_status, "Не подключён");
//		lv_obj_set_style_text_color(lbl_cur_status, lv_palette_main(LV_PALETTE_RED), 0);
//	}

	lv_label_set_text_fmt(lbl_cur_temp, "%.2f °C", udp_server_temperature());
	lv_label_set_text_fmt(lbl_cur_hum, "%.2f %%", udp_server_humidity());
	lv_label_set_text_fmt(lbl_cur_battery, "%.2f %%", udp_server_battery());
}

static void update(lv_event_t *e)
{
	lv_timer_ready(timer);
}

void create_remote_sensors_sub_page(lv_event_t *e)
{
	clear_all_sub_page_child();
	lv_obj_set_style_pad_hor(sub_remote_sensors_page, 20, 0);

	lv_obj_t *section = lv_menu_section_create(sub_remote_sensors_page);

	create_text(section, NULL, txt, LV_MENU_ITEM_BUILDER_VAR_1);

	lv_obj_t *cont = create_text(section, NULL, "Статус:", LV_MENU_ITEM_BUILDER_VAR_2);
	lbl_cur_status = lv_label_create(lv_obj_get_child(cont, 0));
	lv_obj_set_align(lbl_cur_status, LV_ALIGN_CENTER);

	cont = create_text(section, NULL, "Температура:", LV_MENU_ITEM_BUILDER_VAR_1);
	lbl_cur_temp = lv_label_create(lv_obj_get_child(cont, 0));
	lv_obj_set_align(lbl_cur_temp, LV_ALIGN_CENTER);

	cont = create_text(section, NULL, "Влажность:", LV_MENU_ITEM_BUILDER_VAR_2);
	lbl_cur_hum = lv_label_create(lv_obj_get_child(cont, 0));
	lv_obj_set_align(lbl_cur_hum, LV_ALIGN_CENTER);

	cont = create_text(section, NULL, "Батарея:", LV_MENU_ITEM_BUILDER_VAR_2);
	lbl_cur_battery = lv_label_create(lv_obj_get_child(cont, 0));
	lv_obj_set_align(lbl_cur_battery, LV_ALIGN_CENTER);

	lv_obj_t *btn_update = NULL;
	create_button(section, UPDATE_RU_STR, 128, 40, &btn_update);
	lv_obj_add_event_cb(btn_update, update, LV_EVENT_CLICKED, 0);

	timer = lv_timer_create(timer_loop, 5000, 0);
	lv_timer_ready(timer);
}

void free_remote_sensors_sub_page(void)
{
	if (timer != NULL)
	{
		lv_timer_del(timer);
		timer = NULL;
	}
}
