/*
 * sntp.c
 *
 *  Created on: 14 дек. 2023 г.
 *      Author: user
 */

#include "sntp.h"

const char *TAG = "SNTP";

char *sntp_utc;
char *sntp_server_url;

const char *sntp_utc_default = "UTC-3";
const char *sntp_server_url_default = "pool.ntp.org";

static void check_sntp_conf_file(void);

void time_sync_notification_cb(struct timeval *tv);

void time_sync_notification_cb(struct timeval *tv)
{
	glob_set_bits_status_reg(STATUS_TIME_SYNC);

	time_t now;
	struct tm timeinfo;
	char strftime_buf[64];

	time(&now);
	setenv("TZ", sntp_utc, 1);
	tzset();
	localtime_r(&now, &timeinfo);
	strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
	ESP_LOGI(TAG, "The current date/time in %s is: %s\n", sntp_utc, strftime_buf);

	DS3231_SetDataTime_tm(&timeinfo);
}

static void check_sntp_conf_file(void)
{
	cJSON *root = cJSON_CreateObject();

	cJSON *sntp = cJSON_CreateObject();
	cJSON_AddItemToObjectCS(root, SNTP_STR, sntp);

	cJSON *on_obj = cJSON_CreateString("0");
	cJSON_AddItemToObject(sntp, ON_STR, on_obj);

	cJSON *utc_obj = cJSON_CreateString("UTC-3");
	cJSON_AddItemToObjectCS(sntp, UTC_STR, utc_obj);

	cJSON *url_obj = cJSON_CreateString(sntp_server_url_default);
	cJSON_AddItemToObjectCS(sntp, URL_STR, url_obj);

	get_sntp_config_value(ON_STR, &on_obj->valuestring);
	get_sntp_config_value(UTC_STR, &utc_obj->valuestring);
	get_sntp_config_value(URL_STR, &url_obj->valuestring);

	FILE *file = fopen(SNTP_CONF_PATH, "w");
	if (file == NULL)
		printf(CANT_WRITE_FILE_TMPLT, TAG, WIFI_CONF_PATH);
	else
	{
		fprintf(file, "%s", cJSON_Print(root));
		fclose(file);
	}

	cJSON_Delete(root);
}

void read_sntp_conf(void)
{
	char *on = NULL;

	if (get_sntp_config_value(ON_STR, &on) && on != NULL)
	{
		if (strcmp(on, "1") == 0)
			glob_set_bits_status_reg(STATUS_SNTP_ON);
		else
			glob_clear_bits_status_reg(STATUS_SNTP_ON);
		free(on);
	}

	if (!get_sntp_config_value(UTC_STR, &sntp_utc))
		sntp_utc = (char *)sntp_utc_default;

	if (!get_sntp_config_value(URL_STR, &sntp_server_url))
		sntp_server_url = (char *)sntp_server_url_default;
}

void sntp_obtain_time(void)
{
	esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(sntp_server_url);
	config.sync_cb = time_sync_notification_cb;
	esp_netif_sntp_init(&config);

	int retry = 0;
	const int retry_count = 15;
	while (esp_netif_sntp_sync_wait(2000 / portTICK_PERIOD_MS) == ESP_ERR_TIMEOUT && ++retry < retry_count);

	esp_netif_sntp_deinit();
}

void sntp_service_task(void *pvParameters)
{
	vTaskDelay(DELAYED_LAUNCH / portTICK_PERIOD_MS);

	check_sntp_conf_file();
	read_sntp_conf();

	time_t now;
	struct tm timeinfo;

	while(true)
	{
		if (glob_get_status_err())
			break;

		if (glob_get_update_reg() & UPDATE_NOW)
			break;

		if ( !(glob_get_status_reg() & STATUS_SNTP_ON) || !(glob_get_status_reg() & STATUS_IP_GOT) )
			goto for_end;

		read_sntp_conf();

		time(&now);
		localtime_r(&now, &timeinfo);
		sntp_obtain_time();
		time(&now);

		// Set timezone
		setenv("TZ", sntp_utc, 1);
		tzset();
		localtime_r(&now, &timeinfo);

		if (sntp_get_sync_mode() == SNTP_SYNC_MODE_SMOOTH)
		{
			struct timeval outdelta;
			while (sntp_get_sync_status() == SNTP_SYNC_STATUS_IN_PROGRESS)
			{
				adjtime(NULL, &outdelta);
				vTaskDelay(2000 / portTICK_PERIOD_MS);
			}
		}
		vTaskDelay(1000 * (1 * 60) / portTICK_PERIOD_MS); // раз в 1 минуту

		for_end:
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

	vTaskDelete(NULL);
}

