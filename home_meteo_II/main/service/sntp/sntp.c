/*
 * sntp.c
 *
 *  Created on: 14 дек. 2023 г.
 *      Author: user
 */

#include "sntp.h"

const char *TAG = "SNTP";
const static char *task_name = "sntp_task";

static const char *sntp_utc_default = "UTC-3";
static const char *sntp_server_url_default = "pool.ntp.org";

static char *sntp_utc = NULL;
static char *sntp_server_url = NULL;

static uint16_t counter = COUNTER_SNTP;

static void check_sntp_conf_file(void);
static void time_sync_notification_cb(struct timeval *tv);

static void time_sync_notification_cb(struct timeval *tv)
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
	//	ESP_LOGI(TAG, "The current date/time in %s is: %s\n", sntp_utc, strftime_buf);

	DS3231_SetDataTime_tm(&timeinfo);
}

static void check_sntp_conf_file(void)
{
	cJSON *root = cJSON_CreateObject();
	if (root == NULL)
		return;

	cJSON *sntp = cJSON_CreateObject();
	if (sntp == NULL)
		goto end;

	cJSON_AddItemToObjectCS(root, SNTP_STR, sntp);

	cJSON *on_obj = cJSON_CreateString("0");
	if (on_obj == NULL)
		goto end;

	cJSON_AddItemToObject(sntp, ON_STR, on_obj);

	cJSON *utc_obj = cJSON_CreateString("UTC-3");
	if (utc_obj == NULL)
		goto end;

	cJSON_AddItemToObjectCS(sntp, UTC_STR, utc_obj);

	cJSON *url_obj = cJSON_CreateString(sntp_server_url_default);
	if (url_obj == NULL)
		goto end;

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

	end:
	cJSON_Delete(root);
}

void service_sntp_read_conf(void)
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

	if (sntp_utc != NULL)
		free(sntp_utc);

	if (!get_sntp_config_value(UTC_STR, &sntp_utc))
	{
		size_t size = sizeof(sntp_utc_default);
		sntp_utc = calloc(1, size + 1);
		memcpy(sntp_utc, sntp_utc_default, size);
	}


	if (sntp_server_url != NULL)
		free(sntp_server_url);

	if (!get_sntp_config_value(URL_STR, &sntp_server_url))
	{
		size_t size = sizeof(sntp_server_url_default);
		sntp_utc = calloc(1, size + 1);
		memcpy(sntp_server_url, sntp_server_url_default, size);
	}
}

void service_sntp_obtain_time(void)
{
	esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(sntp_server_url);
	config.sync_cb = time_sync_notification_cb;
	esp_netif_sntp_init(&config);

	int retry = 0;
	const int retry_count = 15;
	while (esp_netif_sntp_sync_wait(2000 / portTICK_PERIOD_MS) == ESP_ERR_TIMEOUT && ++retry < retry_count);

	esp_netif_sntp_deinit();
}

const char *service_sntp_utc(void)
{
	return sntp_utc;
}

const char *service_sntp_url(void)
{
	return sntp_server_url;
}

void service_sntp_update(void)
{
	counter = COUNTER_SNTP;
}

void service_sntp_task(void *pvParameters)
{
	vTaskDelay(DELAYED_LAUNCH / portTICK_PERIOD_MS);
	printf("%s %s start\n", TAG, task_name);

	check_sntp_conf_file();
	service_sntp_read_conf();

	time_t now;
	struct tm timeinfo;

	while(true)
	{
		//		printf("SNTP from core %d!\n", xPortGetCoreID() );

		if (glob_get_status_err()
				|| (glob_get_update_reg() & UPDATE_NOW))
			break;

		if ( !(glob_get_status_reg() & STATUS_SNTP_ON)
				|| !(glob_get_status_reg() & STATUS_IP_GOT) )
			goto for_end;

		service_sntp_read_conf();

		if (++counter > COUNTER_SNTP)
		{
			counter = 0;

			time(&now);
			localtime_r(&now, &timeinfo);
			service_sntp_obtain_time();
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
		}

		for_end:
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

	vTaskDelete(NULL);
}

