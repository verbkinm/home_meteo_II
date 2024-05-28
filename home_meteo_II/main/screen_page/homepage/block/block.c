/*
 * block.c
 *
 *  Created on: 24 мая 2024 г.
 *      Author: user
 */

#include "block.h"
#include "Global_def.h"
#include "screen_page/page.h"
#include "Local_Lib/local_lvgl_lib.h"
#include "service/bme280/bme280.h"
#include "service/udp_server/udp_server.h"
#include "weather/weather.h"

#include <time.h>
#include <esp_log.h>

extern lv_font_t ubuntu_mono_14;
extern lv_font_t ubuntu_mono_26;
extern lv_font_t ubuntu_mono_48;
extern lv_font_t ubuntu_mono_128;

static const char *TAG = "BLCOK";

static lv_obj_t *create_block(lv_obj_t *parent, lv_coord_t w, lv_coord_t h);
static lv_obj_t *create_datetime_1_4(lv_obj_t *parent);
static lv_obj_t *create_datetime_2_4_h(lv_obj_t *parent);
static lv_obj_t *create_sensor_1_4(lv_obj_t *parent, block_type_t type);
static lv_obj_t *create_weather_1_4(lv_obj_t *parent);
static lv_obj_t *create_weather_2_4_v(lv_obj_t *parent);

static void update_datetime_1_4(lv_obj_t *obj);
static void update_datetime_2_4_h(lv_obj_t *obj);
static void update_sensor_1_4(lv_obj_t *obj, block_type_t type);

static void event_handler_block_clicked(lv_event_t *e);

static void update_datetime_1_4(lv_obj_t *obj)
{
	lv_obj_t *time_lbl = lv_obj_get_child(obj, 0);
	lv_obj_t *date_lbl = lv_obj_get_child(obj, 1);

	//	if (time_lbl == NULL || date_lbl == NULL)
	//		return;

	time_t now;
	struct tm timeinfo;
	time(&now);
	localtime_r(&now, &timeinfo);

	lv_label_set_text_fmt(time_lbl, "%.02d:%.02d",
			timeinfo.tm_hour,
			timeinfo.tm_min);
	lv_label_set_text_fmt(date_lbl, "%s. %02d.%.02d.%.04d",
			weekday_name_short(timeinfo.tm_wday),
			timeinfo.tm_mday,
			timeinfo.tm_mon + 1,
			timeinfo.tm_year + 1900);
}

static void update_datetime_2_4_h(lv_obj_t *obj)
{
	lv_obj_t *time_lbl = lv_obj_get_child(obj, 0);
	lv_obj_t *date_lbl = lv_obj_get_child(obj, 1);

	time_t now;
	struct tm timeinfo;
	time(&now);
	localtime_r(&now, &timeinfo);

	lv_label_set_text_fmt(time_lbl, "%.02d:%.02d:%.02d",
			timeinfo.tm_hour,
			timeinfo.tm_min,
			timeinfo.tm_sec);
	lv_label_set_text_fmt(date_lbl, "%s. %02d.%.02d.%.04d",
			weekday_name_full(timeinfo.tm_wday),
			timeinfo.tm_mday,
			timeinfo.tm_mon + 1,
			timeinfo.tm_year + 1900);
}

static void update_sensor_1_4(lv_obj_t *obj, block_type_t type)
{
	if (type == BLOCKTYPE_LOCALSENSOR_1_4)
	{
		lv_obj_t *temperature_lbl = lv_obj_get_child(obj, 1);
		lv_obj_t *humidity_lbl = lv_obj_get_child(obj, 3);
		lv_obj_t *pressure_lbl = lv_obj_get_child(obj, 5);
		lv_obj_t *pressure_prefix_lbl = lv_obj_get_child(obj, 6);

		const struct THP *thp = service_BME280_get_value();
		lv_label_set_text_fmt(temperature_lbl, "%+.2f°C", thp->temperature);
		lv_label_set_text_fmt(humidity_lbl, "%.2f %%", thp->humidity);
		lv_label_set_text_fmt(pressure_lbl, "%.0f", thp->pressure);
		lv_obj_align_to(pressure_prefix_lbl, pressure_lbl, LV_ALIGN_OUT_RIGHT_MID, 25, 7);
	}
	else if (type == BLOCKTYPE_REMOTESENSOR_1_4)
	{
		lv_obj_t *last_conn_lbl = lv_obj_get_child(obj, 1);
		lv_obj_t *temperature_lbl = lv_obj_get_child(obj, 3);
		lv_obj_t *humidity_lbl = lv_obj_get_child(obj, 5);
		lv_obj_t *pressure_lbl = lv_obj_get_child(obj, 7);
		lv_obj_t *pressure_prefix_lbl = lv_obj_get_child(obj, 8);

		const remote_sensor_t *sensor = service_upd_server_get_sensor(0);

		lv_label_set_text_fmt(last_conn_lbl,
				"%.02d:%.02d:%.02d  %02d.%.02d.%.04d\n",
				sensor->last_connect.tm_hour, sensor->last_connect.tm_min, sensor->last_connect.tm_sec,
				sensor->last_connect.tm_mday, sensor->last_connect.tm_mon + 1, sensor->last_connect.tm_year + 1900);

		lv_label_set_text_fmt(temperature_lbl, "%+.2f°C", sensor->temperature);
		lv_label_set_text_fmt(humidity_lbl, "%.2f %%", sensor->humidity);
		lv_label_set_text_fmt(pressure_lbl, "%.0f", sensor->pressure);
		lv_obj_align_to(pressure_prefix_lbl, pressure_lbl, LV_ALIGN_OUT_RIGHT_MID, 20, 0);
	}
}

static lv_obj_t *create_block(lv_obj_t *parent, lv_coord_t w, lv_coord_t h)
{
	lv_obj_t *obj = lv_obj_create(parent);
	lv_obj_set_style_pad_all(obj, 0, 0);
	lv_obj_set_size(obj, w, h);
	lv_obj_set_style_border_width(obj, 1, 0);
	lv_obj_set_style_radius(obj, 0, 0);
	lv_obj_set_style_bg_color(obj, lv_color_black(), 0);
	lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
	lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

	return obj;
}

static lv_obj_t *create_datetime_1_4(lv_obj_t *parent)
{
	lv_obj_t *block = create_block(parent, LCD_H_RES / 2, (LCD_V_RES - LCD_PANEL_STATUS_H) / 2);
	block->user_data = malloc(sizeof(block_type_t));
	*(block_type_t *)block->user_data = BLOCKTYPE_DATETIME_1_4;

	lv_obj_t *time_lbl = create_lbl_obj(block, "00:00", block, LV_ALIGN_DEFAULT, 0, 0, lv_color_white(), &ubuntu_mono_128);
	lv_obj_align(time_lbl, LV_ALIGN_CENTER, 0, -30);
	lv_obj_t *date_lbl = create_lbl_obj(block, "пн. 00.00.0000", time_lbl, LV_ALIGN_DEFAULT, 0, 0, lv_color_white(), &ubuntu_mono_48);
	lv_obj_align(date_lbl, LV_ALIGN_BOTTOM_MID, 0, -15);

	lv_obj_add_event_cb(block, event_handler_block_clicked, LV_EVENT_CLICKED, block->user_data);

	return block;
}

static lv_obj_t *create_datetime_2_4_h(lv_obj_t *parent)
{
	lv_obj_t *block = create_block(parent, LCD_H_RES, (LCD_V_RES - LCD_PANEL_STATUS_H) / 2);
	block->user_data = malloc(sizeof(block_type_t));
	*(block_type_t *)block->user_data = BLOCKTYPE_DATETIME_2_4_H;

	lv_obj_t *time_lbl = create_lbl_obj(block, "00:00:00", block, LV_ALIGN_DEFAULT, 0, 0, lv_color_white(), &ubuntu_mono_128);
	lv_obj_align(time_lbl, LV_ALIGN_CENTER, 0, -30);
	lv_obj_t *date_lbl = create_lbl_obj(block, "понедельник. 00.00.0000", time_lbl, LV_ALIGN_DEFAULT, 0, 0, lv_color_white(), &ubuntu_mono_48);
	lv_obj_align(date_lbl, LV_ALIGN_BOTTOM_MID, 0, -15);

	lv_obj_add_event_cb(block, event_handler_block_clicked, LV_EVENT_CLICKED, block->user_data);

	return block;
}

static lv_obj_t *create_sensor_1_4(lv_obj_t *parent, block_type_t type)
{
	lv_obj_t *block = create_block(parent, LCD_H_RES / 2, (LCD_V_RES - LCD_PANEL_STATUS_H) / 2);
	block->user_data = malloc(sizeof(block_type_t));
	*(block_type_t *)block->user_data = type;

	if (type == BLOCKTYPE_LOCALSENSOR_1_4)
	{
		lv_obj_t *temperature_img = create_img_obj(block, TEMPERATURE, block, LV_ALIGN_TOP_LEFT, 64, 64, 30, 10);
		create_lbl_obj(block, "0.0°C", temperature_img, LV_ALIGN_OUT_RIGHT_TOP, 40, 5, lv_color_white(), &ubuntu_mono_48);

		lv_obj_t *humidity_img = create_img_obj(block, HUMIDITY, temperature_img, LV_ALIGN_OUT_BOTTOM_RIGHT, 64, 64, 0, 5);
		create_lbl_obj(block, "0 %", humidity_img, LV_ALIGN_OUT_RIGHT_TOP, 40, 5, lv_color_white(), &ubuntu_mono_48);

		lv_obj_t *pressure_img = create_img_obj(block, PRESSURE, humidity_img, LV_ALIGN_OUT_BOTTOM_RIGHT, 64, 64, 0, 5);
		lv_obj_t *pressure1_lbl = create_lbl_obj(block, "0", pressure_img, LV_ALIGN_OUT_RIGHT_TOP, 40, 5, lv_color_white(), &ubuntu_mono_48);
		create_lbl_obj(block, HG_STR, pressure1_lbl, LV_ALIGN_OUT_RIGHT_MID, 20, 7, lv_color_white(), &ubuntu_mono_26);
	}
	else if (type == BLOCKTYPE_REMOTESENSOR_1_4)
	{
		lv_obj_t *last_conn_lbl = create_lbl_obj(block, "Последнее подключение:",
				block, LV_ALIGN_TOP_LEFT, 32, 30, lv_color_white(), &ubuntu_mono_14);

		create_lbl_obj(block, "00:00:00 00.00.0000",
				last_conn_lbl, LV_ALIGN_OUT_RIGHT_MID, 10, 0, lv_color_white(), &ubuntu_mono_14);

		lv_obj_t *temperature_img = create_img_obj(block, TEMPERATURE32, last_conn_lbl, LV_ALIGN_OUT_BOTTOM_LEFT, 32, 32, 0, 20);
		create_lbl_obj(block, "0.0°C", temperature_img, LV_ALIGN_OUT_RIGHT_MID, 25, -5, lv_color_white(), &ubuntu_mono_26);

		lv_obj_t *humidity_img = create_img_obj(block, HUMIDITY32, temperature_img, LV_ALIGN_OUT_BOTTOM_LEFT, 32, 32, 0, 10);
		create_lbl_obj(block, "0 %", humidity_img, LV_ALIGN_OUT_RIGHT_MID, 25, -5, lv_color_white(), &ubuntu_mono_26);

		lv_obj_t *pressure_img = create_img_obj(block, PRESSURE32, humidity_img, LV_ALIGN_OUT_BOTTOM_RIGHT, 32, 32, 0, 10);
		lv_obj_t *pressure1_lbl = create_lbl_obj(block, "0", pressure_img, LV_ALIGN_OUT_RIGHT_MID, 25, -5, lv_color_white(), &ubuntu_mono_26);
		create_lbl_obj(block, HG_STR, pressure1_lbl, LV_ALIGN_OUT_RIGHT_MID, 20, -10, lv_color_white(), &ubuntu_mono_26);

		create_lbl_obj(block, "№1", block, LV_ALIGN_RIGHT_MID, -70, -5, lv_color_white(), &ubuntu_mono_48);
	}

	lv_obj_add_event_cb(block, event_handler_block_clicked, LV_EVENT_CLICKED, block->user_data);

	return block;
}

static void event_handler_block_clicked(lv_event_t *e)
{
	if (e == NULL)
		return;

	if (e->user_data != NULL)
	{
		block_type_t bt = *(block_type_t *)e->user_data;
		page_type_t pt = PAGE_BLANK;

		if (bt == BLOCKTYPE_DATETIME_1_4 || bt == BLOCKTYPE_DATETIME_2_4_H)
			pt = PAGE_DATETIME1;
		else if (bt == BLOCKTYPE_LOCALSENSOR_1_4)
			pt = PAGE_LOCAL_SENSOR;
		else if (bt == BLOCKTYPE_WEATHER_1_4 || bt == BLOCKTYPE_WEATHER_2_4_V)
			pt = PAGE_METEO_CHART;

		page_set_new_num(pt);
	}
}

static lv_obj_t *create_weather_1_4(lv_obj_t *parent)
{
	lv_obj_t *block = create_block(parent, LCD_H_RES / 2, (LCD_V_RES - LCD_PANEL_STATUS_H) / 2);
	block->user_data = malloc(sizeof(block_type_t));
	*(block_type_t *)block->user_data = BLOCKTYPE_WEATHER_1_4;

	block = block_weather_create_1_4(block);
	lv_obj_add_event_cb(block, event_handler_block_clicked, LV_EVENT_CLICKED, block->user_data);

	return block;
}

static lv_obj_t *create_weather_2_4_v(lv_obj_t *parent)
{
	lv_obj_t *block = create_block(parent, LCD_H_RES / 2, LCD_V_RES - LCD_PANEL_STATUS_H);

	block->user_data = malloc(sizeof(block_type_t));
	*(block_type_t *)block->user_data = BLOCKTYPE_WEATHER_2_4_V;

	block = block_weather_create_2_4_v(block);
	lv_obj_add_event_cb(block, event_handler_block_clicked, LV_EVENT_CLICKED, block->user_data);

	return block;
}

void block_update(lv_obj_t *obj)
{
	if (obj == NULL || obj->user_data == NULL)
		return;

	block_type_t type = *(block_type_t *)obj->user_data;
	switch(type)
	{
	case BLOCKTYPE_DATETIME_1_4:
	{
		update_datetime_1_4(obj);
	}
	break;
	case BLOCKTYPE_DATETIME_2_4_H:
	{
		update_datetime_2_4_h(obj);
	}
	break;

	case BLOCKTYPE_LOCALSENSOR_1_4:
	case BLOCKTYPE_REMOTESENSOR_1_4:
	{
		update_sensor_1_4(obj, type);
	}
	break;
	case BLOCKTYPE_WEATHER_1_4:
	{
		block_weather_update_1_4(obj);
	}
	break;
	case BLOCKTYPE_WEATHER_2_4_V:
	{
		block_weather_update_2_4_v(obj);
	}
	break;
	}
}

lv_obj_t *block_create(lv_obj_t * parent, block_type_t type)
{
	if (parent == NULL)
		return NULL;

	switch(type)
	{
	case BLOCKTYPE_DATETIME_1_4:
	{
		return create_datetime_1_4(parent);
	}
	break;
	case BLOCKTYPE_DATETIME_2_4_H:
	{
		return create_datetime_2_4_h(parent);
	}
	break;
	case BLOCKTYPE_LOCALSENSOR_1_4:
	case BLOCKTYPE_REMOTESENSOR_1_4:
	{
		return create_sensor_1_4(parent, type);
	}
	break;
	case BLOCKTYPE_WEATHER_1_4:
	{
		return create_weather_1_4(parent);
	}
	break;
	case BLOCKTYPE_WEATHER_2_4_V:
	{
		return create_weather_2_4_v(parent);
	}
	break;
	}

	return NULL;
}
