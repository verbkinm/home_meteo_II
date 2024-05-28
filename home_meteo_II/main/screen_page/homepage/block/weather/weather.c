/*
 * weather.c
 *
 *  Created on: 27 мая 2024 г.
 *      Author: user
 */

#include "weather.h"

#include "Global_def.h"
#include "screen_page/page.h"
#include "Local_Lib/local_lvgl_lib.h"
#include "json/json_config.h"
#include "service/weather/weather.h"

extern lv_font_t ubuntu_mono_14;
extern lv_font_t ubuntu_mono_26;

lv_obj_t *block_weather_create_1_4(lv_obj_t *parent)
{
	// Город
	lv_obj_t *city_lbl = lv_label_create(parent);
	char *city_name = NULL;
	if (get_meteo_config_value(CITY_STR, &city_name) && (city_name != NULL))
	{
		lv_label_set_text_fmt(city_lbl, "%s", city_name);
		free(city_name);
	}
	else
		lv_label_set_text(city_lbl, "-");

	lv_obj_align(city_lbl, LV_ALIGN_TOP_RIGHT, -5, 0);
	lv_obj_set_style_text_color(city_lbl, lv_color_white(), 0);
	lv_obj_set_style_text_font(city_lbl, &ubuntu_mono_26, 0);

//	lv_obj_t *last_update_lbl =
			create_lbl_obj(parent, "Данные на:\n--.--.----\n--:--", parent, LV_ALIGN_TOP_RIGHT, -40, 40, lv_color_white(), &ubuntu_mono_14);

	lv_obj_t *pressure_img = create_img_obj(parent, PRESSURE32, parent, LV_ALIGN_TOP_LEFT, 32, 32, 32, 30);
//	lv_obj_t *pressure_lbl =
			create_lbl_obj(parent, "- мм.рт.cт.", pressure_img, LV_ALIGN_OUT_RIGHT_MID, 20, -5, lv_color_white(), &ubuntu_mono_26);

	lv_obj_t *temperature_img = create_img_obj(parent, TEMPERATURE32, pressure_img, LV_ALIGN_OUT_BOTTOM_LEFT, 32, 32, 0, 30);
	lv_obj_t *temperature_lbl = create_lbl_obj(parent, "-°C", temperature_img, LV_ALIGN_OUT_RIGHT_MID, 20, -5, lv_color_white(), &ubuntu_mono_26);

	lv_obj_t *humidity_img = create_img_obj(parent, HUMIDITY32, temperature_lbl, LV_ALIGN_OUT_RIGHT_MID, 32, 32, 40, 0);
//	lv_obj_t *humidity_lbl =
			create_lbl_obj(parent, "-%", humidity_img, LV_ALIGN_OUT_RIGHT_MID, 20, -5, lv_color_white(), &ubuntu_mono_26);

	lv_obj_t *wind_img = create_img_obj(parent, WIND, temperature_img, LV_ALIGN_OUT_BOTTOM_LEFT, 32, 32, 0, 30);
//	lv_obj_t *wind_speed_lbl =
			create_lbl_obj(parent, "- м/с", wind_img, LV_ALIGN_OUT_RIGHT_MID, 20, -5, lv_color_white(), &ubuntu_mono_26);

	return parent;
}

bool block_weather_update_1_4(lv_obj_t *parent)
{
	lv_obj_t *city_lbl = lv_obj_get_child(parent, 0);
	lv_obj_t *last_update_lbl = lv_obj_get_child(parent, 1);
	lv_obj_t *pressure_lbl = lv_obj_get_child(parent, 3);
	lv_obj_t *temperature_lbl = lv_obj_get_child(parent, 5);
	lv_obj_t *humidity_img = lv_obj_get_child(parent, 6);
	lv_obj_t *humidity_lbl = lv_obj_get_child(parent, 7);
	lv_obj_t *wind_speed_lbl = lv_obj_get_child(parent, 9);

	char *city = NULL;
	if (get_meteo_config_value(CITY_STR, &city) && city != NULL)
	{
		lv_label_set_text(city_lbl, city);
		free(city);
	}

	const open_meteo_data_t *open_meteo = service_weather_get_current_meteo_data();
	if (open_meteo == NULL)
		return false;

	struct tm timeinfo;
	localtime_r(&open_meteo->time, &timeinfo);
	lv_label_set_text_fmt(last_update_lbl, "Данные на:\n"
			"%.02d.%.02d.%.02d\n"
			"%.02d:%.02d",
			timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900,
			timeinfo.tm_hour, timeinfo.tm_min);

	lv_label_set_text_fmt(wind_speed_lbl, "%.02f м/с", open_meteo->wind_speed);

	lv_label_set_text_fmt(temperature_lbl, "%+.02f°C", open_meteo->temperature);

	lv_obj_align_to(humidity_img, temperature_lbl, LV_ALIGN_OUT_RIGHT_MID, 40, 0);
	lv_label_set_text_fmt(humidity_lbl, "%d%%", open_meteo->relative_humidity);
	lv_obj_align_to(humidity_lbl, humidity_img, LV_ALIGN_OUT_RIGHT_MID, 20, 0);

	lv_label_set_text_fmt(pressure_lbl, "%d мм.рт.cт.", (int)open_meteo->surface_pressure);

	return true;
}

lv_obj_t *block_weather_create_2_4_v(lv_obj_t *parent)
{
	// Город
	lv_obj_t *city_lbl = lv_label_create(parent);
	char *city_name = NULL;
	if (get_meteo_config_value(CITY_STR, &city_name) && (city_name != NULL))
	{
		lv_label_set_text_fmt(city_lbl, "%s", city_name);
		free(city_name);
	}
	else
		lv_label_set_text(city_lbl, "-");

	lv_obj_align(city_lbl, LV_ALIGN_TOP_RIGHT, -5, 0);
	lv_obj_set_style_text_color(city_lbl, lv_color_white(), 0);
	lv_obj_set_style_text_font(city_lbl, &ubuntu_mono_26, 0);

//	lv_obj_t *last_update_lbl =
			create_lbl_obj(parent, "Данные на:\n--.--.----\n--:--", parent, LV_ALIGN_TOP_RIGHT, -40, 40, lv_color_white(), &ubuntu_mono_14);

	lv_obj_t *cloud_codver_img = create_img_obj(parent, CLOUD_COVER_1, parent, LV_ALIGN_TOP_LEFT, 128, 128, 10, 10);
	lv_obj_t * precipitations_img = create_img_obj(parent, PRECIPITATION0, cloud_codver_img, LV_ALIGN_OUT_BOTTOM_MID, 128, 64, 0, 0);

	lv_obj_t * wind_direction_img = create_img_obj(parent, WIND_NO, cloud_codver_img, LV_ALIGN_OUT_RIGHT_MID, 128, 128, 20, 20);
	lv_obj_t *wind_img = create_img_obj(parent, WIND, wind_direction_img, LV_ALIGN_OUT_BOTTOM_LEFT, 32, 32, 20, 20);
//	lv_obj_t *wind_speed_lbl =
			create_lbl_obj(parent, "- м/с", wind_img, LV_ALIGN_OUT_RIGHT_MID, 20, -6, lv_color_white(), &ubuntu_mono_26);

	lv_obj_t *temperature_img = create_img_obj(parent, TEMPERATURE32, precipitations_img, LV_ALIGN_OUT_BOTTOM_MID, 32, 32, 0, 30);
//	lv_obj_t * temperature2_lbl =
			create_lbl_obj(parent, "-°C", temperature_img, LV_ALIGN_OUT_RIGHT_MID, 20, -7, lv_color_white(), &ubuntu_mono_26);

	lv_obj_t *humidity_img = create_img_obj(parent, HUMIDITY32, temperature_img, LV_ALIGN_CENTER, 32, 32, 200, 0);
//	lv_obj_t * humidity2_lbl =
			create_lbl_obj(parent, "-%", humidity_img, LV_ALIGN_OUT_RIGHT_MID, 20, -7, lv_color_white(), &ubuntu_mono_26);

	lv_obj_t *pressure_img = create_img_obj(parent, PRESSURE32, temperature_img, LV_ALIGN_OUT_BOTTOM_MID, 32, 32, 0, 10);
//	lv_obj_t *pressure2_lbl =
			create_lbl_obj(parent, "- мм.рт.cт.", pressure_img, LV_ALIGN_OUT_RIGHT_MID, 20, -6, lv_color_white(), &ubuntu_mono_26);

	lv_obj_t *apparent_temperature_lbl = create_lbl_obj(parent, "Температура\nощущается: -°C", pressure_img, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20, lv_color_white(), &ubuntu_mono_14);
	lv_obj_t *precipitation_lbl = create_lbl_obj(parent, "Осадки: - мм.", apparent_temperature_lbl, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5, lv_color_white(), &ubuntu_mono_14);
	lv_obj_t *rain_lbl = create_lbl_obj(parent, "Дождь: 0 мм.", precipitation_lbl, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5, lv_color_white(), &ubuntu_mono_14);
//	lv_obj_t *cloud_cover_lbl =
			create_lbl_obj(parent, "Облачность: -%", rain_lbl, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5, lv_color_white(), &ubuntu_mono_14);
	lv_obj_t *wind_gusts_lbl = create_lbl_obj(parent, "Порывы\nветра: - м/с", apparent_temperature_lbl, LV_ALIGN_BOTTOM_RIGHT, 70, 0, lv_color_white(), &ubuntu_mono_14);
	lv_obj_t *showers_lbl = create_lbl_obj(parent, "Ливни: - мм.", wind_gusts_lbl, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5, lv_color_white(), &ubuntu_mono_14);
	lv_obj_t *snow_lbl = create_lbl_obj(parent, "Снег: - см.", showers_lbl, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5, lv_color_white(), &ubuntu_mono_14);
//	lv_obj_t *wind_direction_lbl =
			create_lbl_obj(parent, "Ветер: -°", snow_lbl, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5, lv_color_white(), &ubuntu_mono_14);

	return parent;
}

bool block_weather_update_2_4_v(lv_obj_t *parent)
{
	if ( !(glob_get_status_reg() & STATUS_METEO_ON))
		return false;

	lv_obj_t *city_lbl = lv_obj_get_child(parent,  0);
	lv_obj_t *last_update_lbl = lv_obj_get_child(parent,  1);

	lv_obj_t *cloud_codver_img = lv_obj_get_child(parent,  2);
	lv_obj_t * precipitations_img = lv_obj_get_child(parent,  3);

	lv_obj_t * wind_direction_img = lv_obj_get_child(parent,  4);
//	lv_obj_t *wind_img = lv_obj_get_child(parent,  5);
	lv_obj_t *wind_speed_lbl = lv_obj_get_child(parent,  6);

//	lv_obj_t *temperature_img = lv_obj_get_child(parent,  7);
	lv_obj_t * temperature2_lbl = lv_obj_get_child(parent,  8);

//	lv_obj_t *humidity_img = lv_obj_get_child(parent,  9);
	lv_obj_t * humidity2_lbl = lv_obj_get_child(parent,  10);

//	lv_obj_t *pressure_img = lv_obj_get_child(parent,  11);
	lv_obj_t *pressure2_lbl = lv_obj_get_child(parent,  12);

	lv_obj_t *apparent_temperature_lbl = lv_obj_get_child(parent,  13);
	lv_obj_t *precipitation_lbl = lv_obj_get_child(parent,  14);
	lv_obj_t *rain_lbl = lv_obj_get_child(parent,  15);
	lv_obj_t *cloud_cover_lbl = lv_obj_get_child(parent,  16);
	lv_obj_t *wind_gusts_lbl = lv_obj_get_child(parent,  17);
	lv_obj_t *showers_lbl = lv_obj_get_child(parent,  18);
	lv_obj_t *snow_lbl = lv_obj_get_child(parent,  19);
	lv_obj_t *wind_direction_lbl = lv_obj_get_child(parent,  20);

	char *city = NULL;
	if (get_meteo_config_value(CITY_STR, &city) && city != NULL)
	{
		lv_label_set_text(city_lbl, city);
		free(city);
	}

	const open_meteo_data_t *open_meteo = service_weather_get_current_meteo_data();
	if (open_meteo == NULL)
		return false;

	lv_label_set_text_fmt(wind_speed_lbl, "%.02f м/с", open_meteo->wind_speed);

	lv_label_set_text_fmt(temperature2_lbl, "%+.02f°C", open_meteo->temperature);

	lv_label_set_text_fmt(humidity2_lbl, "%d%%", open_meteo->relative_humidity);
	lv_label_set_text_fmt(pressure2_lbl, "%d мм.рт.cт.", (int)open_meteo->surface_pressure);

	lv_label_set_text_fmt(apparent_temperature_lbl, "Температура\nощущается: %+.02f°C", open_meteo->apparent_temperature);

	lv_label_set_text_fmt(precipitation_lbl, "Осадки: %.02f мм.", open_meteo->precipitation);
	lv_label_set_text_fmt(rain_lbl, "Дождь: %.02f мм.", open_meteo->rain);
	lv_label_set_text_fmt(wind_gusts_lbl, "Порывы\nветра: %.02f м/c", open_meteo->wind_gusts);
	lv_label_set_text_fmt(showers_lbl, "Ливни: %.02f мм.", open_meteo->showers);
	lv_label_set_text_fmt(snow_lbl, "Снег: %.02f cм.", open_meteo->snowfall);
	lv_label_set_text_fmt(cloud_cover_lbl, "Облачность: %d%%", open_meteo->cloud_cover);
	lv_label_set_text_fmt(wind_direction_lbl, "Ветер: %d°", open_meteo->wind_direction);

	struct tm timeinfo;
	localtime_r(&open_meteo->time, &timeinfo);
	lv_label_set_text_fmt(last_update_lbl, "Данные на:\n"
			"%.02d.%.02d.%.02d\n"
			"%.02d:%.02d",
			timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900,
			timeinfo.tm_hour, timeinfo.tm_min);

	// draw cloud cower
	if (open_meteo->cloud_cover <= 25)
		lv_img_set_src(cloud_codver_img, CLOUD_COVER_1);
	else if (open_meteo->cloud_cover <= 50)
		lv_img_set_src(cloud_codver_img, CLOUD_COVER_2);
	else if (open_meteo->cloud_cover <= 75)
		lv_img_set_src(cloud_codver_img, CLOUD_COVER_3);
	else
		lv_img_set_src(cloud_codver_img, CLOUD_COVER_4);

	// draw precipitations
	if ( (open_meteo->rain > 0 || open_meteo->showers > 0) && open_meteo->snowfall > 0)
	{
		lv_img_set_src(precipitations_img, SNOW_AND_RAIN);
		return false;
	}

	if (open_meteo->rain > 0)
	{
		if (open_meteo->rain <= 5)
			lv_img_set_src(precipitations_img, RAIN_1);
		else if (open_meteo->rain <= 20)
			lv_img_set_src(precipitations_img, RAIN_2);
		else if (open_meteo->rain <= 40)
			lv_img_set_src(precipitations_img, RAIN_3);
		else
			lv_img_set_src(precipitations_img, RAIN_4);
	}

	if (open_meteo->snowfall >= 0.20)
		lv_img_set_src(precipitations_img, SNOW_4);
	else if (open_meteo->snowfall >= 0.15)
		lv_img_set_src(precipitations_img, SNOW_3);
	else if (open_meteo->snowfall >= 0.10)
		lv_img_set_src(precipitations_img, SNOW_2);
	else if (open_meteo->snowfall > 0)
		lv_img_set_src(precipitations_img, SNOW_1);
	else
		lv_img_set_src(precipitations_img, PRECIPITATION0);

	// draw wind direction
	if (open_meteo->wind_direction >= 338  || (open_meteo->wind_direction <= 22) )
		lv_img_set_src(wind_direction_img, WIND_0);
	else if (open_meteo->wind_direction >= 23 && open_meteo->wind_direction <= 67)
		lv_img_set_src(wind_direction_img, WIND_45);
	else if (open_meteo->wind_direction >= 68 && open_meteo->wind_direction <= 112)
		lv_img_set_src(wind_direction_img, WIND_90);
	else if (open_meteo->wind_direction >= 113 && open_meteo->wind_direction <= 167)
		lv_img_set_src(wind_direction_img, WIND_135);
	else if (open_meteo->wind_direction >= 168 && open_meteo->wind_direction <= 202)
		lv_img_set_src(wind_direction_img, WIND_180);
	else if (open_meteo->wind_direction >= 203 && open_meteo->wind_direction <= 247)
		lv_img_set_src(wind_direction_img, WIND_225);
	else if (open_meteo->wind_direction >= 248 && open_meteo->wind_direction <= 292)
		lv_img_set_src(wind_direction_img, WIND_270);
	else if (open_meteo->wind_direction >= 293 && open_meteo->wind_direction <= 337)
		lv_img_set_src(wind_direction_img, WIND_315);
	else
		lv_img_set_src(wind_direction_img, WIND_NO);

	return true;
}
