/*
 * factory_reset.c
 *
 *  Created on: 23 янв. 2024 г.
 *      Author: user
 */

#include "update.h"

#define URL_MAX_LENGH	256

static const char *TAG = "UPDATE";

static const char *default_url_update_info = "https://ota.verbkinm.ru/other/esp32/meteo_home_II/update_info.json";
static const char *default_version = "0.0";
static const char *error_version = "Ошибка сервера обновлений!";

static char *url_update = NULL;
static char *url_update_info = NULL;

static char available_version[64];

static void check_update_conf_file(void);
static esp_err_t chek_update(void);
static void update(void);
static esp_err_t parse_http_response(const char* content);

static void check_update_conf_file(void)
{
	cJSON *root = cJSON_CreateObject();
	if (root == NULL)
		return;

	cJSON *update = cJSON_CreateObject();
	if (update == NULL)
		goto end;

	cJSON_AddItemToObjectCS(root, UPDATE_STR, update);

	cJSON *update_notification = cJSON_CreateString("1");
	if (update_notification == NULL)
		goto end;

	cJSON_AddItemToObject(update, ON_STR, update_notification);

	cJSON *url_obj = cJSON_CreateString(default_url_update_info);
	if (url_obj == NULL)
		goto end;

	cJSON_AddItemToObjectCS(update, URL_STR, url_obj);

	get_update_config_value(URL_STR, &url_obj->valuestring);
	get_update_config_value(ON_STR, &update_notification->valuestring);

	FILE *file = fopen(UPDATE_PATH, "w");
	if (file == NULL)
		printf(CANT_WRITE_FILE_TMPLT, TAG, UPDATE_PATH);
	else
	{
		fprintf(file, "%s", cJSON_Print(root));
		fclose(file);
	}

	end:
	cJSON_Delete(root);
}

void service_update_read_conf(void)
{
	char *buf = NULL;
	if (get_update_config_value(ON_STR, &buf) && buf != NULL)
	{
		if (strcmp(buf, "1") == 0)
			glob_set_bits_update_reg(UPDATE_ON);
		else
			glob_clear_bits_update_reg(UPDATE_ON);

		free(buf);
	}

	if (get_update_config_value(URL_STR, &buf) && buf != NULL)
	{
		if (url_update_info != NULL)
			free(url_update_info);

		url_update_info = calloc(1, strlen(buf) + 1);
		strcpy(url_update_info, buf);

		free(buf);
	}
}

static esp_err_t parse_http_response(const char* content)
{
	cJSON *root = cJSON_Parse(content);
	if (root == NULL)
		return ESP_FAIL;

	cJSON *firmware = cJSON_GetObjectItemCaseSensitive(root, FIRMWARE_STR);
	if (firmware == NULL)
		goto bad_end;

	cJSON *version = cJSON_GetObjectItemCaseSensitive(firmware, VERSION_STR);
	if (version == NULL)
		goto bad_end;

	char *new_version_from_server = cJSON_GetStringValue(version);
	if (new_version_from_server == NULL || strlen(new_version_from_server) == 0)
		goto bad_end;

	cJSON *url = cJSON_GetObjectItemCaseSensitive(firmware, URL_STR);
	char *new_url = cJSON_GetStringValue(url);
	if (new_url == NULL || strlen(new_url) == 0)
		goto bad_end;

	if (strlen(new_url) > URL_MAX_LENGH)
		goto bad_end;

	if (url_update != NULL)
		free(url_update);

	url_update = calloc(1, strlen(new_url) + 1);
	if (url_update == NULL)
		goto bad_end;

	strcpy(url_update, new_url);

	const esp_app_desc_t *esp_desc = esp_app_get_description();
	if (strcmp(new_version_from_server, esp_desc->version) > 0)
		glob_set_bits_update_reg(UPDATE_AVAILABLE);
	else
		glob_clear_bits_update_reg(UPDATE_AVAILABLE);

	strcpy(available_version, new_version_from_server);

	cJSON_Delete(root);
	return ESP_OK;

	bad_end:
	strcpy(available_version, error_version);
	glob_clear_bits_update_reg(UPDATE_AVAILABLE);

	cJSON_Delete(root);
	return ESP_FAIL;
}

static esp_err_t chek_update(void)
{
	if (url_update_info == NULL)
		return ESP_FAIL;

	ESP_LOGI(TAG, "check update from %s", url_update_info);

	glob_set_bits_update_reg(UPDATE_CHECK);
	esp_http_client_config_t config = {
			.url = url_update_info,
			.crt_bundle_attach = esp_crt_bundle_attach,
			//			.event_handler = http_event_handler,
	};

	esp_http_client_handle_t client = esp_http_client_init(&config);
	if (esp_http_client_open(client, 0 != ESP_OK))
	{
		ESP_LOGI(TAG, "esp_http_client_open error, url = %s", url_update_info);
		glob_clear_bits_update_reg(UPDATE_CHECK);
		esp_http_client_cleanup(client);
		return ESP_FAIL;
	}

	esp_http_client_fetch_headers(client);

	char *response_buffer = calloc(1, BUFSIZE);
	esp_http_client_read(client, response_buffer, BUFSIZE);

	if (parse_http_response(response_buffer) == ESP_FAIL)
	{
		ESP_LOGE(TAG, "Error parse url_update_info file %s", url_update_info);
		glob_set_bits_update_reg(UPDATE_CHECK_ERROR);
	}

	esp_http_client_cleanup(client);
	glob_clear_bits_update_reg(UPDATE_CHECK);

	return ESP_OK;
}

static void update(void)
{
	esp_err_t ret = ota_firmware(url_update);

	if (ret == ESP_OK)
		glob_set_bits_update_reg(UPDATE_DONE);
	else
		glob_set_bits_status_err(STATUS_ERROR_UPDATE);

	glob_clear_bits_update_reg(UPDATE_NOW);
}

const char *service_update_get_available_version(void)
{
	return available_version;
}

void service_update_task(void *pvParameters)
{
	vTaskDelay(DELAYED_LAUNCH / portTICK_PERIOD_MS);

	check_update_conf_file();
	service_update_read_conf();

	strcpy(available_version, default_version);

	uint16_t counter = COUNTER_CHECK_UPDATE;

	for( ;; )
	{
		//		printf("Update from core %d!\n", xPortGetCoreID() );

		if (glob_get_status_err())
			break;

		if ( !(glob_get_status_reg() & STATUS_IP_GOT) )
			goto for_end;

		if (glob_get_update_reg() & UPDATE_NOW)
		{
			update();
			break;
		}

		// Проверить версию доступного обновления. Запускается из Настройки - Обновления
		if (glob_get_update_reg() & UPDATE_CHECK)
		{
			if (chek_update() != ESP_OK)
				counter = COUNTER_CHECK_UPDATE;
		}

		// Фоновая проверка нового обновления. В lvgl потоке будет показано сообщение, если есть новая версия обновления
		if (glob_get_update_reg() & UPDATE_ON)
		{
			if (++counter > COUNTER_CHECK_UPDATE)
			{
				counter = 0;
				if (chek_update() != ESP_OK)
					counter = COUNTER_CHECK_UPDATE;
				if (glob_get_update_reg() & UPDATE_AVAILABLE)
					glob_set_bits_update_reg(UPDATE_MESSAGE);
			}
		}

		for_end:
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}
