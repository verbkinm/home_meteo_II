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

#include <time.h>
#include <esp_log.h>

extern lv_font_t ubuntu_mono_14;
extern lv_font_t ubuntu_mono_26;
extern lv_font_t ubuntu_mono_48;
extern lv_font_t ubuntu_mono_128;

static const char *TAG = "BLCOK";

static lv_obj_t *create_block_1_4(lv_obj_t *parent, lv_coord_t w, lv_coord_t h);
static lv_obj_t *create_datetime_1_4(lv_obj_t *parent);
static lv_obj_t *create_sensor_1_4(lv_obj_t *parent, block_type_t type);

static void update_datetime_1_4(lv_obj_t *obj);
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

static void update_sensor_1_4(lv_obj_t *obj, block_type_t type)
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

static lv_obj_t *create_block_1_4(lv_obj_t *parent, lv_coord_t w, lv_coord_t h)
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
	lv_obj_t *block = create_block_1_4(parent, LCD_H_RES / 2, (LCD_V_RES - LCD_PANEL_STATUS_H) / 2);
	block->user_data = malloc(sizeof(page_type_t));
	*(page_type_t *)block->user_data = PAGE_DATETIME1;

	lv_obj_t *time_lbl = create_lbl_obj(block, "00:00", block, LV_ALIGN_DEFAULT, 0, 0, lv_color_white(), &ubuntu_mono_128);
	lv_obj_align(time_lbl, LV_ALIGN_CENTER, 0, -30);
	lv_obj_t *date_lbl = create_lbl_obj(block, "пн. 00.00.0000", time_lbl, LV_ALIGN_DEFAULT, 0, 0, lv_color_white(), &ubuntu_mono_48);
	lv_obj_align(date_lbl, LV_ALIGN_BOTTOM_MID, 0, -15);

	lv_obj_add_event_cb(block, event_handler_block_clicked, LV_EVENT_CLICKED, block->user_data);

	return block;
}

static lv_obj_t *create_sensor_1_4(lv_obj_t *parent, block_type_t type)
{
	lv_obj_t *block = create_block_1_4(parent, LCD_H_RES / 2, (LCD_V_RES - LCD_PANEL_STATUS_H) / 2);
	block->user_data = malloc(sizeof(page_type_t));
	if (type == BLOCKTYPE_LOCALSENSOR_1_4)
		*(page_type_t *)block->user_data = PAGE_LOCAL_SENSOR;

	lv_obj_t *temperature_img = create_img_obj(block, TEMPERATURE, block, LV_ALIGN_TOP_LEFT, 64, 64, 30, 10);
	create_lbl_obj(block, "0.0°C", temperature_img, LV_ALIGN_OUT_RIGHT_TOP, 40, 5, lv_color_white(), &ubuntu_mono_48);

	lv_obj_t *humidity_img = create_img_obj(block, HUMIDITY, temperature_img, LV_ALIGN_OUT_BOTTOM_RIGHT, 64, 64, 0, 5);
	create_lbl_obj(block, "0 %", humidity_img, LV_ALIGN_OUT_RIGHT_TOP, 40, 5, lv_color_white(), &ubuntu_mono_48);

	lv_obj_t *pressure_img = create_img_obj(block, PRESSURE, humidity_img, LV_ALIGN_OUT_BOTTOM_RIGHT, 64, 64, 0, 5);
	lv_obj_t *pressure1_lbl = create_lbl_obj(block, "0", pressure_img, LV_ALIGN_OUT_RIGHT_TOP, 40, 5, lv_color_white(), &ubuntu_mono_48);
	create_lbl_obj(block, HG_STR, pressure1_lbl, LV_ALIGN_OUT_RIGHT_MID, 20, 7, lv_color_white(), &ubuntu_mono_26);

	lv_obj_add_event_cb(block, event_handler_block_clicked, LV_EVENT_CLICKED, block->user_data);

	return block;
}

static void event_handler_block_clicked(lv_event_t *e)
{
	if (e == NULL)
		return;

	if (e->user_data != NULL)
	{
		page_type_t pt = *(page_type_t *)e->user_data;
		page_set_new_num(pt);
	}
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
	case BLOCKTYPE_LOCALSENSOR_1_4:
	case BLOCKTYPE_REMOTESENSOR_1_4:
	{
		update_sensor_1_4(obj, type);
	}
	break;
	case BLOCKTYPE_WEATHER_1_4:
	{

	}
	break;
	case BLOCKTYPE_WEATHER_2_4:
	{

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
	case BLOCKTYPE_LOCALSENSOR_1_4:
	case BLOCKTYPE_REMOTESENSOR_1_4:
	{
		return create_sensor_1_4(parent, type);
	}
	break;
	case BLOCKTYPE_WEATHER_1_4:
	{

	}
	break;
	case BLOCKTYPE_WEATHER_2_4:
	{

	}
	break;
	}

	return NULL;
}
