/*
 * ota.c
 *
 *  Created on: 28 сент. 2023 г.
 *      Author: user
 */

#include "ota.h"

#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_crt_bundle.h"
#include "Global_def.h"
#include "Local_Lib/local_lvgl_lib.h"

static const char *TAG = "OTA";

static esp_err_t _http_event_handler(esp_http_client_event_t *evt);
static esp_err_t write_firmware(const esp_partition_t *part);

static esp_err_t write_firmware(const esp_partition_t *part)
{
	if (part == NULL)
		return ESP_ERR_INVALID_ARG;

	FILE *file = fopen(FIRMWARE_PATH, "rb");
	if (file == NULL)
	{
		ESP_LOGI(TAG, "Error open firmware file - %s", FIRMWARE_PATH);
		return ESP_ERR_NOT_FOUND;
	}

	glob_set_bits_update_reg(UPDATE_SD_NOW);
	vTaskDelay(5000 / portTICK_PERIOD_MS);

	ESP_LOGI(TAG, "Write to partition %s, subtype %d", part->label, part->subtype);

	ESP_LOGI(TAG, "Erase partition");
	esp_partition_erase_range(part, 0, part->size);

	ESP_LOGI(TAG, "Writing...");
	char buf[BUFSIZE];
	while(!feof(file))
	{
		size_t readBytes = fread(buf, 1, sizeof(buf), file);
		if (esp_partition_write(part, 0, buf, readBytes) != ESP_OK)
		{
			ESP_LOGE(TAG, "Error esp_partition_write.");
			fclose(file);

			return ESP_FAIL;
		}
	}
	fclose(file);
	ESP_LOGI(TAG, "Write to partition - SUCCESS");

	if (esp_ota_set_boot_partition(part) != ESP_OK)
	{
		ESP_LOGE(TAG, "Error esp_ota_set_boot_partition.");
		return ESP_FAIL;
	}

	ESP_LOGI(TAG, "Write firmware - SUCCESS");

	return ESP_OK;
}

esp_err_t backtofactory(void)
{
	esp_partition_iterator_t pi = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, "factory") ;
	if (pi == NULL)
	{
		ESP_LOGE ( TAG, "Failed to find factory partition");
		return ESP_FAIL;
	}

	const esp_partition_t* factory = esp_partition_get(pi);
	esp_partition_iterator_release(pi);

	if (esp_ota_set_boot_partition(factory) != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to set boot partition");
		return ESP_FAIL;
	}

	return ESP_OK;
}

static esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
	switch (evt->event_id) {
	case HTTP_EVENT_ERROR:
		ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
		break;
	case HTTP_EVENT_ON_CONNECTED:
		ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
		break;
	case HTTP_EVENT_HEADER_SENT:
		ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
		break;
	case HTTP_EVENT_ON_HEADER:
		ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
		break;
	case HTTP_EVENT_ON_DATA:
		ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
		break;
	case HTTP_EVENT_ON_FINISH:
		ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
		break;
	case HTTP_EVENT_DISCONNECTED:
		ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
		break;
	case HTTP_EVENT_REDIRECT:
		ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
		break;
	}
	return ESP_OK;
}

esp_err_t ota_firmware(const char *url)
{
	if (url == NULL)
	{
		ESP_LOGE(TAG, "url fail");
		return ESP_FAIL;
	}

	ESP_LOGI(TAG, "url - %s", url);

	esp_http_client_config_t config = {
			.url = url,
			.crt_bundle_attach = esp_crt_bundle_attach,
			.event_handler = _http_event_handler,
			.keep_alive_enable = true,
	};

	esp_https_ota_config_t ota_config = {
			.http_config = &config,
	};

	ESP_LOGI(TAG, "Attempting to download update from %s", config.url);

	esp_err_t ret = esp_https_ota(&ota_config);
	if (ret == ESP_OK)
	{
		ESP_LOGI(TAG, "OTA Succeed, please reboot device.");
		return ESP_OK;
	}

	ESP_LOGE(TAG, "Firmware upgrade failed");
	return ESP_FAIL;
}

esp_err_t ota_firmware_from_sd(void)
{
	const esp_partition_t *ota_0_part = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
	const esp_partition_t *ota_1_part = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, NULL);

	const esp_partition_t *curr_part = esp_ota_get_boot_partition();

	esp_err_t ret = ESP_ERR_NOT_SUPPORTED;
	if (curr_part->subtype == ESP_PARTITION_SUBTYPE_APP_FACTORY)
		ret = write_firmware(ota_0_part);
	else if (curr_part->subtype == ESP_PARTITION_SUBTYPE_APP_OTA_0)
		ret = write_firmware(ota_1_part);
	else if (curr_part->subtype == ESP_PARTITION_SUBTYPE_APP_OTA_1)
		ret = write_firmware(ota_0_part);

	glob_clear_bits_update_reg(UPDATE_SD_NOW);
	if (ret == ESP_OK)
	{
		glob_set_bits_update_reg(UPDATE_DONE);
		rename(FIRMWARE_PATH, FIRMWARE_OK_PATH);
	}
	else if (ret == ESP_FAIL)
	{
		glob_set_bits_status_err(STATUS_ERROR_UPDATE);
		rename(FIRMWARE_PATH, FIRMWARE_ERROR_PATH);
	}

	return ret;
}
