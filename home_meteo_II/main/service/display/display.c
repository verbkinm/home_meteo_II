/*
 * display.c
 *
 *  Created on: 25 дек. 2023 г.
 *      Author: user
 */

#include "display.h"

static const char *TAG = "DISPLAY";

static void check_display_conf_file(void);
static void read_display_conf(void);

service_display_t display_service_data;

const service_display_t *service_display_conf(void)
{
	return &display_service_data;
}

void service_display_read_conf(void)
{
	read_display_conf();
}

void service_display_task(void *pvParameters)
{
	printf("%s task start\n", TAG);

	check_display_conf_file();
	read_display_conf();

	for( ;; )
	{
		if (glob_get_status_err()
				|| (glob_get_update_reg() & UPDATE_NOW)
				|| (glob_get_update_reg() & UPDATE_SD_NOW))
			break;

		if (glob_get_status_reg() & STATUS_DISPLAY_NIGHT_MODE_ON)
		{
			time_t now;
			struct tm timeinfo = { 0 };
			time(&now);
			localtime_r(&now, &timeinfo);

			int cur_sec = timeinfo.tm_sec + timeinfo.tm_min * 60 + timeinfo.tm_hour * 3600;
			int day_sec = display_service_data.day_begin_s + display_service_data.day_begin_m * 60 + display_service_data.day_begin_h * 3600;
			int night_sec = display_service_data.night_begin_s + display_service_data.night_begin_m * 60 + display_service_data.night_begin_h * 3600;

			if ( (cur_sec < night_sec && cur_sec < day_sec)
					|| (cur_sec > night_sec && cur_sec < day_sec)
					|| ((cur_sec > night_sec && cur_sec > day_sec) && day_sec < night_sec))
			{
				ledc_set_fade_with_time(LEDC_MODE, LEDC_CHANNEL, display_service_data.brightness_night, 1000);
				ledc_fade_start(LEDC_MODE, LEDC_CHANNEL, LEDC_FADE_NO_WAIT);
				TFT_set_display_brightness(display_service_data.brightness_night);
			}
			else
			{
				ledc_set_fade_with_time(LEDC_MODE, LEDC_CHANNEL, display_service_data.brightness_day, 1000);
				ledc_fade_start(LEDC_MODE, LEDC_CHANNEL, LEDC_FADE_NO_WAIT);
				TFT_set_display_brightness(display_service_data.brightness_day);
			}
		}

		vTaskDelay(SERVICE_PERIOD_DISPLAY / portTICK_PERIOD_MS);
	}
	printf("%s task stop\n", TAG);
	vTaskDelete(NULL);
}

static void read_display_conf(void)
{
	char *buf = NULL;
	if (get_display_config_value(ROTATE_STR, &buf) && buf != NULL)
	{
		if (strcmp(buf, "1") == 0)
			TFT_set_display_rotate(LV_DISP_ROT_180);
		else
			TFT_set_display_rotate(LV_DISP_ROT_NONE);

		free(buf);
	}

	if (get_display_config_value(BRIGHTNESS_STR, &buf) && buf != NULL)
	{
		int value = 255;
		sscanf(buf, "%d", &value);
		free(buf);
		value = inRange(value, 2, 255);
		TFT_set_display_brightness(value);
		display_service_data.brightnes = value;
	}

	if (get_display_config_value(NIGHT_MODE_STR, &buf) && buf != NULL)
	{
		if (strcmp(buf, "1") == 0)
			glob_set_bits_status_reg(STATUS_DISPLAY_NIGHT_MODE_ON);
		else
			glob_clear_bits_status_reg(STATUS_DISPLAY_NIGHT_MODE_ON);
		free(buf);
	}

	if (get_display_config_value(BRIGHTNESS_DAY_STR, &buf) && buf != NULL)
	{
		int value = 255;
		sscanf(buf, "%d", &value);
		display_service_data.brightness_day = inRange(value, 2, 255);\
		free(buf);
	}

	if (get_display_config_value(BRIGHTNESS_NIGHT_STR, &buf) && buf != NULL)
	{
		int value = 255;
		sscanf(buf, "%d", &value);
		display_service_data.brightness_night = inRange(value, 2, 255);\
		free(buf);
	}

	if (get_display_config_value(DAY_BEGIN_STR, &buf) && buf != NULL)
	{
		if (strlen(buf) == 8)
		{
			int h, m, s;
			sscanf(buf, "%d:%d:%d", &h, &m, &s);
			display_service_data.day_begin_h = inRange(h, 0, 23);
			display_service_data.day_begin_m = inRange(m, 0, 59);
			display_service_data.day_begin_s = inRange(s, 0, 59);
		}
		free(buf);
	}

	if (get_display_config_value(NIGHT_BEGIN_STR, &buf) && buf != NULL)
	{
		if (strlen(buf) == 8)
		{
			int h, m, s;
			sscanf(buf, "%d:%d:%d", &h, &m, &s);
			display_service_data.night_begin_h = inRange(h, 0, 23);
			display_service_data.night_begin_m = inRange(m, 0, 59);
			display_service_data.night_begin_s = inRange(s, 0, 59);
		}
		free(buf);
	}
}

static void check_display_conf_file(void)
{
	//!!! Нет проверок == NULL
	cJSON *root = cJSON_CreateObject();
	cJSON *display = cJSON_CreateObject();
	cJSON_AddItemToObject(root, DISPLAY_STR, display);

	cJSON *rotate_obj = cJSON_CreateString("0");
	cJSON_AddItemToObject(display, ROTATE_STR, rotate_obj);

	cJSON *brightness_obj = cJSON_CreateString("255");
	cJSON_AddItemToObject(display, BRIGHTNESS_STR, brightness_obj);

	cJSON *nigth_mode_obj = cJSON_CreateString("0");
	cJSON_AddItemToObject(display, NIGHT_MODE_STR, nigth_mode_obj);

	cJSON *brightness_day_obj = cJSON_CreateString("255");
	cJSON_AddItemToObject(display, BRIGHTNESS_DAY_STR, brightness_day_obj);

	cJSON *brightness_night_obj = cJSON_CreateString("150");
	cJSON_AddItemToObject(display, BRIGHTNESS_NIGHT_STR, brightness_night_obj);

	cJSON *day_begin_obj = cJSON_CreateString("08:00:00");
	cJSON_AddItemToObject(display, DAY_BEGIN_STR, day_begin_obj);

	cJSON *night_begin_obj = cJSON_CreateString("20:00:00");
	cJSON_AddItemToObject(display, NIGHT_BEGIN_STR, night_begin_obj);

	get_display_config_value(ROTATE_STR, &rotate_obj->valuestring);
	get_display_config_value(BRIGHTNESS_STR, &brightness_obj->valuestring);
	get_display_config_value(NIGHT_MODE_STR, &nigth_mode_obj->valuestring);
	get_display_config_value(BRIGHTNESS_DAY_STR, &brightness_day_obj->valuestring);
	get_display_config_value(BRIGHTNESS_NIGHT_STR, &brightness_night_obj->valuestring);
	get_display_config_value(DAY_BEGIN_STR, &day_begin_obj->valuestring);
	get_display_config_value(NIGHT_BEGIN_STR, &night_begin_obj->valuestring);

	FILE *file = fopen(DISPLAY_PATH, "w");
	if (file == NULL)
		printf(CANT_WRITE_FILE_TMPLT, TAG, DISPLAY_PATH);
	else
	{
		fprintf(file, "%s", cJSON_Print(root));
		fclose(file);
	}

	cJSON_Delete(root);
}
