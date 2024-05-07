/*
 * datetime.c
 *
 *  Created on: 7 мая 2024 г.
 *      Author: user
 */

#include "datetime3.h"

extern lv_font_t ubuntu_mono_26;

static lv_obj_t *meter;

static lv_meter_indicator_t *indic_sec;
static lv_meter_indicator_t *indic_min;
static lv_meter_indicator_t *indic_hour;

static lv_timer_t *timer = NULL;

static void timer_handler(lv_timer_t *timer);
static void event_handler(lv_event_t * e);

static void timer_handler(lv_timer_t *timer)
{
	if (glob_get_status_err())
	{
		lv_timer_pause(timer);
		return;
	}

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    int hour = tm->tm_hour % 12;
    int conv_min = convert_range(tm->tm_min, 0, 60, 0, 5);
    lv_meter_set_indicator_value(meter, indic_sec, tm->tm_sec);
    lv_meter_set_indicator_value(meter, indic_min, tm->tm_min);
    lv_meter_set_indicator_value(meter, indic_hour, hour * 5 + conv_min);
}

static void event_handler(lv_event_t * e)
{
	datetime1_page_init();
}

void datetime3_page_init(void)
{
	page_t *page = current_page();
	page->deinit();
	page->deinit = datetime3_page_deinit;
	page->title = page_title(DATETIME3_PAGE_TITLE);
	status_panel_update();

	lv_obj_t *widget = lv_obj_create(page->widget);
	lv_obj_set_size(widget, LCD_H_RES, LCD_V_RES - LCD_PANEL_STATUS_H);
	lv_obj_set_scroll_dir(widget, LV_DIR_NONE);
	lv_obj_set_style_pad_all(widget, 0, 0);
	lv_obj_add_style(widget, screenStyleDefault(), 0);

	lv_obj_add_event_cb(widget, event_handler, LV_EVENT_CLICKED, 0);

    meter = lv_meter_create(widget);
    lv_obj_set_style_text_font(meter, &ubuntu_mono_26, 0);
    lv_obj_set_size(meter, LCD_V_RES - LCD_PANEL_STATUS_H, LCD_V_RES - LCD_PANEL_STATUS_H);
    lv_obj_center(meter);
    lv_obj_add_event_cb(meter, event_handler, LV_EVENT_CLICKED, 0);

    lv_meter_scale_t * scale_sec = lv_meter_add_scale(meter);
    lv_meter_set_scale_ticks(meter, scale_sec, 61, 1, 10, lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_range(meter, scale_sec, 0, 60, 360, 270);

    lv_meter_scale_t * scale_min = lv_meter_add_scale(meter);
    lv_meter_set_scale_ticks(meter, scale_min, 61, 1, 10, lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_range(meter, scale_min, 0, 60, 360, 270);

    lv_meter_scale_t * scale_hour = lv_meter_add_scale(meter);
    lv_meter_set_scale_ticks(meter, scale_hour, 12, 0, 0, lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_major_ticks(meter, scale_hour, 1, 2, 15, lv_color_black(), 15);
    lv_meter_set_scale_range(meter, scale_hour, 1, 12, 330, 300);

    indic_sec = lv_meter_add_needle_line(meter, scale_sec, 3, lv_palette_main(LV_PALETTE_RED), 0);
    indic_min = lv_meter_add_needle_line(meter, scale_sec, 7, lv_color_black(), -30);
    indic_hour = lv_meter_add_needle_line(meter, scale_sec, 10, lv_color_black(), -60);

	timer = lv_timer_create(timer_handler, 1000, NULL);
	lv_timer_ready(timer);
}

void datetime3_page_deinit(void)
{
	default_page_deinit();
	lv_timer_del(timer);
}
