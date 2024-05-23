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

static lv_obj_t *sensors_data_text[SENSOR_COUNT];

static lv_timer_t *timer = NULL;

static void timer_loop(lv_timer_t *timer);

static void timer_loop(lv_timer_t *timer)
{
	for (int i = 0; i < SENSOR_COUNT; ++i)
	{
		const remote_sensor_t *sensor = service_upd_server_get_sensor(i);

		lv_label_set_text_fmt(sensors_data_text[i],
				"%.02d:%.02d:%.02d  %02d.%.02d.%.04d\n"
				"%.2f %%\n"
				"%.2f °C\n"
				"%.2f %%\n"
				"%.2f мм рт.ст.\n"
				"%d",
				sensor->last_connect.tm_hour, sensor->last_connect.tm_min, sensor->last_connect.tm_sec,
				sensor->last_connect.tm_mday, sensor->last_connect.tm_mon + 1, sensor->last_connect.tm_year + 1900,
				sensor->battery,
				sensor->temperature,
				sensor->humidity,
				sensor->pressure,
				sensor->error);
	}
}

void create_remote_sensors_sub_page(lv_event_t *e)
{
	clear_all_sub_page_child();
	lv_obj_set_style_pad_hor(sub_remote_sensors_page, 20, 0);

	lv_obj_t *section = lv_menu_section_create(sub_remote_sensors_page);

	for (int i = 0; i < SENSOR_COUNT; ++i)
	{
		char buf[24] = {0};
		snprintf(buf, sizeof(buf) - 1, "Датчик № %d:", i + 1);

		create_text(section, NULL, buf, LV_MENU_ITEM_BUILDER_VAR_1);

		lv_obj_t *obj = lv_menu_cont_create(section);
		lv_obj_t *leftText = lv_label_create(obj);
		lv_obj_set_align(leftText, LV_ALIGN_LEFT_MID);
		lv_label_set_text_fmt(leftText,
				"Последнее подключение:\n"
				"Заряд аккумулятора:\n"
				"Температура:\n"
				"Влажность:\n"
				"Давление:\n"
				"Код ошибки:");

		sensors_data_text[i] = lv_label_create(obj);
		lv_obj_set_align(sensors_data_text[i], LV_ALIGN_RIGHT_MID);
	}

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
