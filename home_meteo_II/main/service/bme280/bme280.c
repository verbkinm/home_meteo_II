/*
 * bme280.c
 *
 *  Created on: 26 янв. 2024 г.
 *      Author: user
 */

#include "bme280.h"

static const char *TAG = "BME280_service";
static struct THP thp, thp_without_calibration;

static void check_conf_file(void);
static void read_conf(void);

static void check_conf_file(void)
{
	cJSON *root = cJSON_CreateObject();
	cJSON *thp = cJSON_CreateObject();
	cJSON_AddItemToObjectCS(root, THP_STR, thp);

	cJSON *t_calib_obj = cJSON_CreateString("0");
	cJSON_AddItemToObject(thp, THP_T_CALIB_STR, t_calib_obj);

	cJSON *h_calib_obj = cJSON_CreateString("0");
	cJSON_AddItemToObject(thp, THP_H_CALIB_STR, h_calib_obj);

	cJSON *p_calib_obj = cJSON_CreateString("0");
	cJSON_AddItemToObject(thp, THP_P_CALIB_STR, p_calib_obj);

	get_bme280_config_value(THP_T_CALIB_STR, &t_calib_obj->valuestring);
	get_bme280_config_value(THP_H_CALIB_STR, &h_calib_obj->valuestring);
	get_bme280_config_value(THP_P_CALIB_STR, &p_calib_obj->valuestring);

	FILE *file = fopen(SENSORS_PATH, "w");
	if (file == NULL)
		printf("cant write \"%s\" file!\n", SENSORS_PATH);
	else
	{
		fprintf(file, "%s", cJSON_Print(root));
		fclose(file);
	}

	cJSON_Delete(root);
}

static void read_conf(void)
{
	char *buf = NULL;
	int value = 0;

	if (get_bme280_config_value(THP_T_CALIB_STR, &buf))
	{
		sscanf(buf, "%d", &value);
		free(buf);
		BME280_set_calib_temperature(inRange(value, -50, 50));
	}

	if (get_bme280_config_value(THP_H_CALIB_STR, &buf))
	{
		sscanf(buf, "%d", &value);
		free(buf);
		BME280_set_calib_humidity(inRange(value, -50, 50));
	}

	if (get_bme280_config_value(THP_P_CALIB_STR, &buf))
	{
		sscanf(buf, "%d", &value);
		free(buf);
		value = inRange(value, -50, 50);
		BME280_set_calib_pressure(inRange(value, -50, 50));
	}
}

void service_BME280_save_calibrations(void)
{
	char buf[BUFSIZ] = {0};

	snprintf(buf, sizeof(buf), "%d", BME280_get_calib_temperature());
	set_bme280_config_value(THP_T_CALIB_STR, buf);

	snprintf(buf, sizeof(buf), "%d", BME280_get_calib_humidity());
	set_bme280_config_value(THP_H_CALIB_STR, buf);

	snprintf(buf, sizeof(buf), "%d", BME280_get_calib_pressure());
	set_bme280_config_value(THP_P_CALIB_STR, buf);
}

const struct THP *service_BME280_get_value(void)
{
	return &thp;
}

const struct THP *service_BME280_get_value_without_calibration(void)
{
	return &thp_without_calibration;
}

void service_BME280_task(void *pvParameters)
{
	vTaskDelay(DELAYED_LAUNCH / portTICK_PERIOD_MS);
	printf("%s task start\n", TAG);

	check_conf_file();
	read_conf();

	BME280_init();

	for( ;; )
	{
		if (glob_get_status_err()
				|| (glob_get_update_reg() & UPDATE_NOW)
				|| (glob_get_update_reg() & UPDATE_SD_NOW))
			break;

		BME280_readValues(&thp, &thp_without_calibration);

		vTaskDelay(SERVICE_PERIOD_BME280 / portTICK_PERIOD_MS);
	}
	printf("%s task stop\n", TAG);
	vTaskDelete(NULL);
}

